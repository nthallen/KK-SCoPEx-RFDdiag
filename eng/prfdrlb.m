function prfdrlb(varargin);
% prfdrlb( [...] );
% R2L Bytes per sec
h = timeplot({'R2L_Int_bytes_tx','R2L_Int_bytes_rx'}, ...
      'R2L Bytes per sec', ...
      'Bytes per sec', ...
      {'R2L\_Int\_bytes\_tx','R2L\_Int\_bytes\_rx'}, ...
      varargin{:} );
