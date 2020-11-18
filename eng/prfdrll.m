function prfdrll(varargin);
% prfdrll( [...] );
% R2L Latency
h = timeplot({'R2L_Int_min_latency','R2L_Int_mean_latency','R2L_Int_max_latency'}, ...
      'R2L Latency', ...
      'Latency', ...
      {'min','mean','max'}, ...
      varargin{:} );