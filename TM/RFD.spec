tmcbase = base.tmc
tmcbase = /usr/local/share/monarch/modules/flttime.tmc
colbase = RFDdiag_col.tmc
cmdbase = RFD.cmd
cmdbase = /usr/local/share/monarch/modules/getcon.cmd
genuibase = RFD.genui
swsbase = RFD.sws

Module RFDdiag

TGTDIR = ../..
IGNORE = "*.o" "*.exe" "*.stackdump" Makefile
DISTRIB = interact

RFDcol :
RFDdisp : display.tbl
RFDalgo : RFDdiag.tma RFD.sws
RFDjsonext : RFD.genui
doit : RFDdiag.doit
%%
CXXFLAGS=-g
