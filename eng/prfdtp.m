function prfdtp(varargin);
% prfdtp( [...] );
% Totals Packets
h = timeplot({'L2R_Total_packets_tx','L2R_Total_valid_packets_rx','L2R_Total_invalid_packets_rx','R2L_Total_packets_tx','R2L_Total_valid_packets_rx','R2L_Total_invalid_packets_rx'}, ...
      'Totals Packets', ...
      'Packets', ...
      {'R2L\_tx','R2L\_valid','R2L\_invalid','R2L\_tx','R2L\_valid','R2L\_invalid'}, ...
      varargin{:} );