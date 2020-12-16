%INTERFACE <RFDdiag>
&command
  : Local Set Packet Size %d * {
      if_RFDdiag.Turf("S:%d\n", $5);
    }
  : Local Set Packet Rate %d * {
      if_RFDdiag.Turf("R:%d\n", $5);
    }
  : Local RTS &enable * {
      if_RFDdiag.Turf("T:%d\n", $3);
    }
  : Local Read &enable * {
      if_RFDdiag.Turf("B:%d\n", $3);
    }
  : Local Quit * {
      if_RFDdiag.Turf("Q\n");
    }
  : Remote Set packet size %d * {
      if_RFDdiag.Turf("XS:%d\n", $5);
    }
  : Remote Set packet rate %d * {
      if_RFDdiag.Turf("XR:%d\n", $5);
    }
  : Remote RTS &enable * {
      if_RFDdiag.Turf("XT:%d\n", $3);
    }
  : Remote Read &enable * {
      if_RFDdiag.Turf("XB:%d\n", $3);
    }
  : Remote Quit * {
      if_RFDdiag.Turf("XQ\n");
    }
  ;
&enable <bool>
  : Enable { $0 = true; }
  : Disable { $0 = false; }
  ;