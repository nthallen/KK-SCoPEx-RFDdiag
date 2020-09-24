.PHONY : all clean
LDFLAGS = -L/usr/local/lib
LIBS += -ldasio -lnl
#CXXFLAGS += -fdiagnostics-color=always
CXXFLAGS=-g

all : RFDdiag

RFDdiag : RFDdiag.o RFDdiagoui.o crc16modbus.o
	$(CXX) $(CXXFLAGS) -o RFDdiag RFDdiag.o RFDdiagoui.o crc16modbus.o $(LDFLAGS) $(LIBS)
RFDdiag.o : RFDdiag.cc RFD_int.h RFDdiag.h
RFDdiagoui.o : RFDdiagoui.cc RFD_int.h RFDdiag.h
RFDdiagoui.cc : RFDdiag.oui
	oui -o RFDdiagoui.cc RFDdiag.oui

clean :
	rm -f RFDdiag RFDdiagoui.cc *.o *.stackdump