#ifndef RFD_INT_H_INCLUDED
#define RFD_INT_H_INCLUDED
#include <stdint.h>
#include "dasio/interface.h"
#include "dasio/tm_tmr.h"
#include "dasio/cmd_reader.h"
#include "RFDdiag.h"

extern bool allow_remote_commands;
extern const char *remote_ip, *rx_port, *tx_port;
void RFDdiag_init_options(int argc, char **argv);

typedef struct __attribute__((packed)) {
  uint16_t Command_bytes;
  /** Requested size of packet, in bytes */
  uint16_t Packet_size;
  uint16_t Packet_rate;
  /** Number of packets transmitted during last second */
  uint16_t Int_packets_tx;
  /** The SN of this packet */
  uint32_t Transmit_SN;
  /** The SN of the last packet received */
  uint32_t Receive_SN;
  /** msecs since midnight utc */
  int32_t  Transmit_timestamp;
  /** Number of packets received during last second */
  uint32_t Int_packets_rx;
  /** Minimum receive latency during last second */
  uint32_t Int_min_latency;
  /** Mean receive latency during last second */
  uint32_t Int_mean_latency;
  /** Maximum receive latency during last second */
  uint32_t Int_max_latency;
  /** Total bytes received during last second */
  uint32_t Int_bytes_rx;
  /** Interval bytes transmitted during last second */
  uint32_t Int_bytes_tx;
  /** Total valid packets received */
  uint32_t Total_valid_packets_rx;
  /** Total invalid packets received */
  uint32_t Total_invalid_packets_rx;
  uint8_t  Remainder[2];
  // All the padding and commands go in before the CRC
} RFDdiag_packet;

class RFD_interface : public DAS_IO::Interface {
  public:
    inline RFD_interface(const char *name, int bufsz) :
      DAS_IO::Interface(name, bufsz) {}
  protected:
    uint16_t crc_calc(uint8_t *buf, int len);
    int32_t get_timestamp();
};

class RFD_tmr;
class RFD_receiver;

class RFD_transmitter : public RFD_interface {
  public:
    RFD_transmitter(const char *rmt_ip, const char *rmt_port,
      RFD_tmr *tmr);
    bool parse_command(char *cmd, unsigned cmdlen);
    bool transmit(uint16_t n_pkts);
    bool tm_sync_too();
  protected:
    void crc_set();
    RFDdiag_packet *pkt;
    uint32_t L2R_Int_packets_tx;
    uint32_t L2R_Int_bytes_tx;
    uint32_t Int_packets_tx;
    uint32_t Int_bytes_tx;
    uint32_t L2R_Transmit_SN;
    uint16_t L2R_Packet_size;
    uint16_t L2R_Packet_rate;
    uint8_t L2R_command_len;
    uint8_t L2R_command[8];
    static const int max_packet_size = 8000;
    RFD_tmr *tmr;
    RFD_receiver *rx;
};

class RFD_receiver : public RFD_interface {
  public:
    RFD_receiver(const char *port, bool allow_remote_commands, RFD_transmitter *tx);
  protected:
    bool protocol_input();
    bool tm_sync();
    bool crc_ok();
    const char *recv_port;
    bool allow_remote_commands;
    RFD_transmitter *tx;
    RFDdiag_packet *pkt;
    uint32_t R2L_Total_packets_rx;
    uint32_t R2L_Total_valid_packets_rx;
    uint32_t R2L_Total_invalid_packets_rx;
    uint32_t R2L_Int_packets_rx;
    int32_t  R2L_Int_min_latency;
    int32_t  R2L_Int_max_latency;
    uint32_t R2L_Int_bytes_rx;
    int32_t  R2L_latencies;
    // uint32_t L2R_Int_packets_tx;
};

class RFD_cmd : public DAS_IO::Cmd_reader {
  public:
    RFD_cmd(RFD_transmitter *tx);
  protected:
    bool protocol_input();
    RFD_transmitter *tx;
};

class RFD_tmr : public DAS_IO::tm_tmr {
  public:
    RFD_tmr();
    void set_transmitter(RFD_transmitter *tx);
  protected:
    bool protocol_input();
    RFD_transmitter *tx;
};

#endif
