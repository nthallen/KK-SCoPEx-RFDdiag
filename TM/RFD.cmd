%INTERFACE <RFDdiag>
&command
  : Local set packet size %d * {
      if_RFDdiag.Turf("S:%d\n", $5);
    }
  : Local set packet rate %d * {
      if_RFDdiag.Turf("R:%d\n", $5);
    }
  : Local Quit * {
      if_RFDdiag.Turf("Q\n");
    }
  : Remote set packet size %d * {
      if_RFDdiag.Turf("XS:%d\n", $5);
    }
  : Remote set packet rate %d * {
      if_RFDdiag.Turf("XR:%d\n", $5);
    }
  : Remote Quit * {
      if_RFDdiag.Turf("XQ\n");
    }
  ;