tmcbase = base.tmc
tmcbase = /usr/local/share/linkeng/flttime.tmc
colbase = RFDdiag_col.tmc
cmdbase = RFD.cmd
# genuibase = udp.genui

Module RFDdiag

TGTDIR = /home/RFDdiag
IGNORE = "*.o" "*.exe" "*.stackdump" Makefile
# SCRIPT = cyg_nc.sh
# IDISTRIB = interact_local.sh interact_remote.sh

RFDcol :
# RFDsrvr :
# RFDclt :
RFDdisp : display.tbl
%%
CXXFLAGS=-g
