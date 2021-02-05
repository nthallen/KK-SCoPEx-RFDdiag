/** @file RFDdiag.cc */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "dasio/loop.h"
#include "dasio/appid.h"
#include "RFDdiag.h"
#include "RFD_int.h"
#include "nl.h"
#include "nl_assert.h"
#include "oui.h"
#include "crc16modbus.h"
#include "dasio/tm_data_sndr.h"

DAS_IO::AppID_t DAS_IO::AppID("RFDdiag", "RFD Performance Diagnostic Tool", "V1.0");

RFD_interface::RFD_interface(const char *name,
      const char *rfd_port, RFD_tmr *tmr)
      : DAS_IO::Serial(name, max_packet_size, rfd_port, O_RDWR),
        write_blocked(false),
        log_tx_pkts(false),
        write_pkts_dropped(0),
        L2R_Int_packets_tx(0),
        L2R_Int_bytes_tx(0),
        Int_packets_tx(0),
        Int_bytes_tx(0),
        L2R_Transmit_SN(0),
        L2R_Packet_size(sizeof(RFDdiag_packet)),
        L2R_Packet_rate(0),
        L2R_command_len(0),
        tmr(tmr),
        // From receiver:
        allow_remote_commands(allow_rmt_commands),
        R2L_Total_packets_rx(0),
        R2L_Total_valid_packets_rx(0),
        R2L_Total_invalid_packets_rx(0),
        R2L_Int_packets_rx(0),
        R2L_Int_min_latency(0),
        R2L_Int_max_latency(0),
        R2L_Int_bytes_rx(0),
        R2L_latencies(0) {
  setup(230400, 8, 'n', 1, sizeof(RFDdiag_packet), 1);
  hwflow_enable(true);
  opkt = (RFDdiag_packet*)new_memory(max_packet_size);
  nl_assert(tmr);
  tmr->set_transmitter(this);
  pkt = (RFDdiag_packet *)buf;
  flags = Fl_Read | gflag(0);
}

uint16_t RFD_interface::crc_calc(uint8_t *buf, int len) {
  return crc16modbus_word(0, (void const *)buf, len);
}

int32_t RFD_interface::get_timestamp() {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    msg(MSG_FATAL, "%s: clock_gettime() returned %d: %s",
      iname, errno, strerror(errno));
  uint32_t secs_today = (ts.tv_sec % (3600*24));
  uint32_t msecs = ts.tv_nsec/1000000;
  return (secs_today*1000)+msecs;
}

RFDdiag_t RFDdiag;

// Commands:
//   S:\d+  Set packet size
//   R:\d+  Set packet rate
//   T:\d+  Set RTS
//   B:\d+  Set Baud
//   H:\d+  Set Read Flag (hearing?)
//   L:\d+  Log Tx Packets
//   Q      Quit
//   XS:\d+ Remote Set packet size
//   XR:\d+ Remote Set packet rate
//   XT:\d+ Remote Set RTS
//   XB:\d+ Remote Set Baud
//   XH:\d+ Remote Set Read Flag (hearing?)
//   XQ     Remote Quit
/**
 * @param cmd if non-zero, command originates from another interface
 * @param cmdlen must be <= 7, greater than 0
 * @return true to terminate
 * If cmd is non-zero
 */
bool RFD_interface::parse_command(unsigned char *cmd, unsigned cmdlen) {
  bool rv = false;
  if (cmdlen > 0 && cmdlen <= 7) {
    unsigned char *save_buf = buf;
    unsigned int save_nc = nc;
    unsigned int save_cp = cp;
    buf = cmd ? cmd : (unsigned char *)&(pkt->Remainder);
    const char *src = cmd ? "local" : "remote";
    cp = 0;
    nc = cmdlen;
    uint16_t newval;
    switch (buf[0]) {
      case 'S':
        if (not_str("S:") || not_uint16(newval)) {
          report_err("%s: Invalid S command syntax", iname);
          consume(nc);
        } else if (newval != L2R_Packet_size) {
          report_ok(nc);
          if (newval >= sizeof(RFDdiag_packet) && newval <= max_packet_size) {
            L2R_Packet_size = newval;
            msg(MSG_DEBUG, "%s S:%u", src, L2R_Packet_size);
          }
        }
        break;
      case 'R':
        if (not_str("R:") || not_uint16(newval)) {
          report_err("%s: Invalid R command syntax", iname);
          consume(nc);
        } else {
          report_ok(nc);
          if (newval != L2R_Packet_rate) {
            L2R_Packet_rate = newval;
            msg(MSG_DEBUG, "%s R:%u", src, L2R_Packet_rate);
            int per_nsecs = L2R_Packet_rate == 0 ? 0 :
              (1000000000/(int)L2R_Packet_rate);
            tmr->settime(per_nsecs);
          }
        }
        break;
      case 'T':
        if (not_str("T:") || not_uint16(newval)) {
          report_err("%s: Invalid T command syntax", iname);
          consume(nc);
        } else {
          report_ok(nc);
          int RTS_flag;
          RTS_flag = TIOCM_RTS;
          ioctl(fd,newval ? TIOCMBIS : TIOCMBIC, &RTS_flag);
        }
        break;
      case 'B':
        if (not_str("B:") || not_uint16(newval)) {
          report_err("%s: Invalid B command syntax", iname);
          consume(nc);
        } else {
          report_ok(nc);
          msg(0, "%s: Setting baud rate to %d", iname, newval);
          setup(newval, 8, 'n', 1, sizeof(RFDdiag_packet), 1);
          hwflow_enable(true);
        }
        break;
      case 'H':
        if (not_str("H:") || not_uint16(newval)) {
          report_err("%s: Invalid H command syntax", iname);
          consume(nc);
        } else {
          report_ok(nc);
          if (newval) {
            flags |= Fl_Read;
          } else {
            flags &= ~Fl_Read;
          }
        }
        break;
      case 'L':
        if (not_str("L:") || not_uint16(newval)) {
          report_err("%s: Invalid L command syntax", iname);
          consume(nc);
        } else {
          log_tx_pkts = (newval & 1) != 0;
        }
        break;
      case 'Q':
        report_ok(nc);
        msg(MSG_DEBUG, "%s Q", src);
        rv = true;
        break;
      case 'X':
        report_ok(nc);
        L2R_command_len = cmdlen-1;
        for (int i = 1; i < cmdlen; ++i) {
          L2R_command[i-1] = cmd[i];
        }
        break;
    }
    buf = save_buf;
    nc = save_nc;
    cp = save_cp;
  }
  return rv;
}

bool RFD_interface::transmit(uint16_t n_pkts) {
  bool rv;
  if (n_pkts > 1)
    msg(MSG_DEBUG, "%s: transmit(n_pkts = %u)", iname, n_pkts);
  for (int i = 0; i < n_pkts; ++i) {
    if (!obuf_empty()) {
      if (!write_pkts_dropped)
        msg(MSG_WARN, "%s: Starting to drop tx packets", iname);
      write_pkts_dropped += n_pkts;
      return false;
    } else if (write_pkts_dropped) {
      msg(MSG_WARN, "%s: Tx recovered. %d packets dropped before tx",
        iname, write_pkts_dropped);
      write_pkts_dropped = 0;
    }
    // build the packet
    // msg(MSG_DBG(0), "Transmit Latencies: N:%d min:%d max:%d",
      // RFDdiag.R2L.Int_packets_rx, RFDdiag.R2L.Int_min_latency, RFDdiag.R2L.Int_max_latency);
    opkt->Synch[0] = RFD_SYNCH_0;
    opkt->Synch[1] = RFD_SYNCH_1;
    opkt->Command_bytes = L2R_command_len;
    opkt->Packet_size = sizeof(RFDdiag_packet) + L2R_command_len;
    if (opkt->Packet_size < L2R_Packet_size)
      opkt->Packet_size = L2R_Packet_size;
    opkt->Packet_rate = L2R_Packet_rate;
    opkt->Int_packets_tx = L2R_Int_packets_tx;
    opkt->Transmit_SN = L2R_Transmit_SN;
    opkt->Receive_SN = RFDdiag.R2L.Receive_SN;
    opkt->Int_packets_rx = RFDdiag.R2L.Int_packets_rx;
    opkt->Int_min_latency = RFDdiag.R2L.Int_min_latency;
    opkt->Int_mean_latency = RFDdiag.R2L.Int_mean_latency;
    opkt->Int_max_latency = RFDdiag.R2L.Int_max_latency;
    opkt->Int_bytes_rx = RFDdiag.R2L.Int_bytes_rx;
    opkt->Int_bytes_tx = RFDdiag.L2R.Int_bytes_tx;
    opkt->Total_valid_packets_rx = RFDdiag.R2L.Total_valid_packets_rx;
    opkt->Total_invalid_packets_rx = RFDdiag.R2L.Total_invalid_packets_rx;
    
    int j;
    for (j = 0; j < L2R_command_len; ++j) {
      opkt->Remainder[j] = L2R_command[j];
    }
    opkt->Transmit_timestamp = get_timestamp();
    
    for (; j < opkt->Packet_size - 2; ++j) {
      opkt->Remainder[j] = (uint8_t)rand();
    }
    crc_set();
    if (log_tx_pkts) log_packet(opkt);
    rv = iwrite((char *)opkt, opkt->Packet_size);
    ++L2R_Transmit_SN;
    ++Int_packets_tx;
    Int_bytes_tx += opkt->Packet_size;
    if (rv) {
      msg(MSG_DEBUG, "%s: Received error from write on pkt %d of %d",
        iname, i+1, n_pkts);
      return false;
    }
  }
  return rv;
}

void RFD_interface::crc_set() {
  uint8_t *data = (uint8_t*)opkt;
  uint16_t crc = crc_calc(data, opkt->Packet_size - 2);
  data[opkt->Packet_size-2] = crc & 0xFF;
  data[opkt->Packet_size-1] = (crc >> 8) & 0xFF;
}

const char *RFD_interface::ascii_escape() {
  return "<redacted>";
}

void RFD_interface::log_packet(RFDdiag_packet *pkt) {
  static std::string s;
  const uint8_t *cbuf = (const uint8_t *)pkt;
  char snbuf[8];
  int j = 0;
  while (j < pkt->Packet_size) {
    s.clear();
    int nr = pkt->Packet_size - j;
    if (nr > 20) nr = 20;
    for (int jj = 0; jj < nr; ++jj) {
      snprintf(snbuf, 8, " %02X", cbuf[j+jj]);
      s.append(snbuf);
    }
    msg(0, "pkt[%2d] = %s", j, s.c_str());
    j += nr;
  }
}

bool RFD_interface::protocol_input() {
  bool rv = false;
  // This routine, originally written for UDP, assumed
  // we would always receive a complete UDP packet.
  // Now that we are using serial, we need to first
  // make sure we have a complete packet before doing
  // all the math. A complete packet begins with
  // RFD_SYNCH_0 and RFD_SYNCH_1, followed by a binary
  // packet length.
  cp = 0;
  while (nc-cp >= 4) {
    if (not_found(RFD_SYNCH_0)) {
      ++R2L_Total_invalid_packets_rx;
      return false;
    }
    consume(cp-1); // realign buf to pkt
    cp = 1; // allows continue to search to next synch
    if (nc < 4) return false; // long enough to include Packet_size
    if (buf[cp] != RFD_SYNCH_1)
      continue;
    if (pkt->Packet_size < sizeof(RFDdiag_packet) ||
        pkt->Packet_size > max_packet_size) {
      ++R2L_Total_packets_rx;
      ++R2L_Total_invalid_packets_rx;
      report_err("%s: Invalid Packet_size(%u)", iname, pkt->Packet_size);
      continue;
    }
    if (nc < pkt->Packet_size) {
      return false;
    }
    ++R2L_Total_packets_rx;
    
    int32_t now = get_timestamp();
    
    if (!crc_ok()) {
      ++R2L_Total_invalid_packets_rx;
      continue;
    }
    cp = pkt->Packet_size; // Now continue will skip entire packet
    if (sizeof(RFDdiag_packet) + pkt->Command_bytes > pkt->Packet_size) {
      ++R2L_Total_invalid_packets_rx;
      report_err("%s: Packet minsize(%u)+Cmd(%d) > Packet_size",
        iname, sizeof(RFDdiag_packet), pkt->Command_bytes);
      continue;
    }
    
    int32_t latency = now - pkt->Transmit_timestamp;
    if (R2L_Int_packets_rx == 0) {
      R2L_latencies = R2L_Int_min_latency = R2L_Int_max_latency = latency;
    } else {
      if (latency < R2L_Int_min_latency) R2L_Int_min_latency = latency;
      else if (latency > R2L_Int_max_latency) R2L_Int_max_latency = latency;
      R2L_latencies += latency;
    }
    ++R2L_Int_packets_rx;
    ++R2L_Total_valid_packets_rx;
    if (RFDdiag.R2L.Receive_SN > 0 && pkt->Transmit_SN <= RFDdiag.R2L.Receive_SN) {
      report_err("%s: Rx SN %u <= previous by %u", iname, pkt->Transmit_SN,
        RFDdiag.R2L.Receive_SN - pkt->Transmit_SN);
    }
        
    RFDdiag.R2L.Packet_size = pkt->Packet_size;
    RFDdiag.R2L.Packet_rate = pkt->Packet_rate;
    
    RFDdiag.R2L.Receive_SN = pkt->Transmit_SN;
    RFDdiag.R2L.Total_packets_tx = pkt->Transmit_SN;
    RFDdiag.L2R.Receive_SN = pkt->Receive_SN;
    R2L_Int_bytes_rx += pkt->Packet_size;
    RFDdiag.R2L.Int_packets_tx = pkt->Int_packets_tx;
    RFDdiag.L2R.Int_packets_rx = pkt->Int_packets_rx;
    RFDdiag.L2R.Int_min_latency = pkt->Int_min_latency;
    RFDdiag.L2R.Int_mean_latency = pkt->Int_mean_latency;
    RFDdiag.L2R.Int_max_latency = pkt->Int_max_latency;
    RFDdiag.L2R.Int_bytes_rx = pkt->Int_bytes_rx;
    RFDdiag.R2L.Int_bytes_tx = pkt->Int_bytes_tx;
    RFDdiag.L2R.Total_valid_packets_rx = pkt->Total_valid_packets_rx;
    RFDdiag.L2R.Total_invalid_packets_rx = pkt->Total_invalid_packets_rx;
    
    if (pkt->Command_bytes > 0 && allow_remote_commands) {
      rv = parse_command(0, pkt->Command_bytes);
    }
  }
  report_ok(cp);
  return rv;
}

bool RFD_interface::read_error(int my_errno) {
  msg(MSG_ERROR, "%s: read error %d: %s", iname, my_errno, strerror(my_errno));
  return false;
}

bool RFD_interface::tm_sync() {
  // Update RFDdiag struct with current readings,
  // then clear our interval counters
  // msg(MSG_DBG(0), "RcvSync Latencies: N:%d min:%d max:%d",
    // R2L_Int_packets_rx, R2L_Int_min_latency, R2L_Int_max_latency);
  RFDdiag.R2L.Int_packets_rx = R2L_Int_packets_rx;
  RFDdiag.R2L.Int_min_latency = R2L_Int_min_latency;
  RFDdiag.R2L.Int_max_latency = R2L_Int_max_latency;
  RFDdiag.R2L.Int_mean_latency = R2L_Int_packets_rx ?
    R2L_latencies/((int32_t)R2L_Int_packets_rx) : 0;
  RFDdiag.R2L.Int_bytes_rx = R2L_Int_bytes_rx;
  RFDdiag.R2L.Total_valid_packets_rx = R2L_Total_valid_packets_rx;
  RFDdiag.R2L.Total_invalid_packets_rx = R2L_Total_invalid_packets_rx;
  R2L_Int_packets_rx = 0;
  R2L_Int_min_latency = 0;
  R2L_Int_max_latency = 0;
  R2L_latencies = 0;
  R2L_Int_bytes_rx = 0;

  // From transmitter:
  RFDdiag.L2R.Packet_size = L2R_Packet_size;
  RFDdiag.L2R.Packet_rate = L2R_Packet_rate;
  L2R_Int_packets_tx = Int_packets_tx;
  L2R_Int_bytes_tx = Int_bytes_tx;
  Int_packets_tx = 0;
  Int_bytes_tx = 0;
  RFDdiag.L2R.Int_packets_tx = L2R_Int_packets_tx;
  RFDdiag.L2R.Int_bytes_tx = L2R_Int_bytes_tx;
  RFDdiag.L2R.Total_packets_tx = L2R_Transmit_SN;
  return false;
}

bool RFD_interface::crc_ok() {
  uint16_t crc = crc_calc(buf, pkt->Packet_size-2);
  uint16_t bufcrc = buf[pkt->Packet_size-2] + (buf[pkt->Packet_size-1]<<8);
  //bool rv = buf[nc-2] == (crc & 0xFF) && buf[nc-1] == ((crc>>8)&0xFF);
  bool rv = crc == bufcrc;
  if (!rv) {
    msg(MSG_ERROR, "%s: CRC error: recd: %04X calc: %04X", iname, bufcrc, crc);
    log_packet(pkt);
  }
  return rv;
}

RFD_cmd::RFD_cmd(RFD_interface *tx)
    : DAS_IO::Cmd_reader("cmd", 40, "RFDdiag"),
      tx(tx) {}

bool RFD_cmd::protocol_input() {
  bool rv = false;
  while (cp < nc ) {
    int i;
    for (i = 0; cp + i < nc && buf[cp+i] != '\n'; ++i) ;
    if (cp + i < nc) { // then buf[cp+i] == '\n'
      rv = tx->parse_command(&buf[cp], i+1) || rv;
      cp += i+1;
    } else {
      break;
    }
  }
  report_ok(cp);
  return rv;
}

RFD_tmr::RFD_tmr()
      : tm_tmr(),
        tx(0)
{}

void RFD_tmr::set_transmitter(RFD_interface *tx) {
  this->tx = tx;
}

bool RFD_tmr::protocol_input() {
  report_ok(nc);
  uint16_t n_pkts =
    (n_expirations < 60000) ? (uint16_t)n_expirations : 60000;
  return  tx ? tx->transmit(n_pkts) : false;
}

bool allow_rmt_commands = false;
const char *rfd_port;

void RFDdiag_init_options(int argc, char **argv) {
  int optltr;

  optind = OPTIND_RESET;
  opterr = 0;
  while ((optltr = getopt(argc, argv, opt_string)) != -1) {
    switch (optltr) {
      case 'c': allow_rmt_commands = true; break;
      case 'p': rfd_port = optarg; break;
      case '?':
        msg(3, "Unrecognized Option -%c", optopt);
      default:
        break;
    }
  }
  if (rfd_port == 0)
    msg(MSG_FATAL, "Must specify RFD900+ serial port with -p option");
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  DAS_IO::Loop ELoop;
  RFD_tmr *tmr = new RFD_tmr();
  ELoop.add_child(tmr);
  RFD_interface *tx = new RFD_interface("radio", rfd_port, tmr);
  ELoop.add_child(tx);
  
  DAS_IO::TM_data_sndr *tm = new DAS_IO::TM_data_sndr("TM", 0, "RFDdiag", (char *)&RFDdiag, sizeof(RFDdiag));
  ELoop.add_child(tm);
  tm->connect();
  
  RFD_cmd *cmd = new RFD_cmd(tx);
  cmd->connect();
  ELoop.add_child(cmd);
  msg(MSG, "%s %s Starting",
    DAS_IO::AppID.fullname, DAS_IO::AppID.rev);
  ELoop.event_loop();
  msg(MSG, "Terminating");
}
