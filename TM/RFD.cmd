%INTERFACE <RFDdiag>
&command
  : &remote Set Packet Size %d * {
      if_RFDdiag.Turf("%sS:%d\n", $1, $5);
    }
  : &remote Set Packet Rate %d * {
      if_RFDdiag.Turf("%sR:%d\n", $1, $5);
    }
  : &remote Set Baud Rate %d * {
      if_RFDdiag.Turf("%sB:%d\n", $1, $5);
    }
  : &remote RTS &enable * {
      if_RFDdiag.Turf("%sT:%d\n", $1, $3);
    }
  : &remote Read &enable * {
      if_RFDdiag.Turf("%sH:%d\n", $1, $3);
    }
  : &remote Quit * {
      if_RFDdiag.Turf("%sQ\n", $1);
    }
  ;

&remote <const char *>
  : Local { $0 = ""; }
  : Remote { $0 = "X"; }
  ;

&enable <bool>
  : Enable { $0 = true; }
  : Disable { $0 = false; }
  ;
