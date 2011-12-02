#
# OMNeT++/OMNEST Makefile for exercise2
#
# This file was generated with the command:
#  opp_makemake -f --deep -O out
#

# Name of target to be created (-o option)
TARGET = exercise2$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -Iimages \
    -Iimages/abstract \
    -Iimages/background \
    -Iimages/block \
    -Iimages/device \
    -Iimages/maps \
    -Iimages/misc \
    -Iimages/msg \
    -Iimages/old \
    -Iimages/status

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cpp and .msg files
OBJS = $O/networknode.o $O/landmarknode.o $O/commonnode.o $O/protocol_m.o

# Message files
MSGFILES = \
    protocol.msg

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $(TARGET)

$(TARGET) : $O/$(TARGET)
	$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY:

.SUFFIXES: .cpp

$O/%.o: %.cpp
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cpp %_m.h: %.msg
	$(MSGC) -s _m.cpp $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	-rm -rf $O
	-rm -f exercise2 exercise2.exe libexercise2.so libexercise2.a libexercise2.dll libexercise2.dylib
	-rm -f ./*_m.cpp ./*_m.h
	-rm -f images/*_m.cpp images/*_m.h
	-rm -f images/abstract/*_m.cpp images/abstract/*_m.h
	-rm -f images/background/*_m.cpp images/background/*_m.h
	-rm -f images/block/*_m.cpp images/block/*_m.h
	-rm -f images/device/*_m.cpp images/device/*_m.h
	-rm -f images/maps/*_m.cpp images/maps/*_m.h
	-rm -f images/misc/*_m.cpp images/misc/*_m.h
	-rm -f images/msg/*_m.cpp images/msg/*_m.h
	-rm -f images/old/*_m.cpp images/old/*_m.h
	-rm -f images/status/*_m.cpp images/status/*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cpp images/*.cpp images/abstract/*.cpp images/background/*.cpp images/block/*.cpp images/device/*.cpp images/maps/*.cpp images/misc/*.cpp images/msg/*.cpp images/old/*.cpp images/status/*.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/networknode.o: networknode.cpp \
	protocol.h \
	networknode.h \
	commonnode.h
$O/commonnode.o: commonnode.cpp \
	protocol_m.h \
	protocol.h \
	commonnode.h
$O/landmarknode.o: landmarknode.cpp \
	landmarknode.h \
	protocol_m.h \
	protocol.h \
	commonnode.h
$O/protocol_m.o: protocol_m.cpp \
	protocol_m.h \
	protocol.h

