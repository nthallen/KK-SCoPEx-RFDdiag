tmcbase = base.tmc
tmcbase = /usr/local/share/monarch/modules/flttime.tmc
colbase = RFDdiag_col.tmc
cmdbase = RFD.cmd
# genuibase = rf.genui

Module RFDdiag

TGTDIR = /home/RFDdiag
IGNORE = "*.o" "*.exe" "*.stackdump" Makefile
DISTRIB = interact

RFDcol :
RFDdisp : display.tbl
doit : RFDdiag.doit
%%
CXXFLAGS=-g
