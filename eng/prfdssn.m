function prfdssn(varargin);
% prfdssn( [...] );
% Status SN
h = timeplot({'L2R_Receive_SN','R2L_Receive_SN'}, ...
      'Status SN', ...
      'SN', ...
      {'L2R\_Receive\_SN','R2L\_Receive\_SN'}, ...
      varargin{:} );
