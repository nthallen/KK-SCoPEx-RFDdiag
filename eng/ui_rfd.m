function ui_rfd
f = ne_dialg('RFD 900 Diagnostic',1);
f = ne_dialg(f, 'add', 0, 1, 'grfdlr', 'L2R' );
f = ne_dialg(f, 'add', 1, 0, 'prfdlrp', 'Pkts per sec' );
f = ne_dialg(f, 'add', 1, 0, 'prfdlrb', 'Bytes per sec' );
f = ne_dialg(f, 'add', 1, 0, 'prfdlrl', 'Latency' );
f = ne_dialg(f, 'add', 0, 1, 'grfdt', 'Totals' );
f = ne_dialg(f, 'add', 1, 0, 'prfdtp', 'Packets' );
f = ne_dialg(f, 'add', 0, 1, 'grfdrl', 'R2L' );
f = ne_dialg(f, 'add', 1, 0, 'prfdrlp', 'Pkts per sec' );
f = ne_dialg(f, 'add', 1, 0, 'prfdrlb', 'Bytes per sec' );
f = ne_dialg(f, 'add', 1, 0, 'prfdrll', 'Latency' );
f = ne_dialg(f, 'add', 0, 1, 'grfds', 'Status' );
f = ne_dialg(f, 'add', 1, 0, 'prfdss', 'Size' );
f = ne_dialg(f, 'add', 1, 0, 'prfdssn', 'SN' );
f = ne_dialg(f, 'add', 1, 0, 'prfdsstale', 'Stale' );
f = ne_listdirs(f, 'RFD_DATA_DIR', 15);
f = ne_dialg(f, 'newcol');
ne_dialg(f, 'resize');
