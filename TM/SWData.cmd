%{
  #include "SWData.h"
  #ifdef SERVER
    SWData_t SWData;
  #endif
%}

%INTERFACE <SWData:DG/data>

&command
  : &SWTM * { if_SWData.Turf(); }
  ;
&SWTM
  : SW Status &SWStat { SWData.SWStat = $3; }
  ;
&SWStat <uint8_t>
  : Idle { $0 = SWS_IDLE; }
  : Set %d { $0 = $2; }
  : Sequence { $0 = SWS_SEQUENCE; }
  : Shutdown Full { $0 = SWS_SHUTDOWN; }
  ;
