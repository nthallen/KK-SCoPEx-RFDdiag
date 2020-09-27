tmcbase = base.tmc
tmcbase = /usr/local/share/monarch/modules/flttime.tmc
colbase = RFDdiag_col.tmc
cmdbase = RFD.cmd
genuibase = RFD.genui

Module RFDdiag

TGTDIR = /home/RFDdiag
IGNORE = "*.o" "*.exe" "*.stackdump" Makefile
DISTRIB = interact

RFDcol :
RFDdisp : display.tbl
# RFDengext :
doit : RFDdiag.doit
%%
CXXFLAGS=-g
