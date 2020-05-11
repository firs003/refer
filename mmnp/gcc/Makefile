VERBOSE = @

LIBNAME = mmnp
ARCHNAME = x86
SRCPATH = ../libsrc
APIHEAD = ../libsrc/vis_mmnp_api.h
HDMI_ENC_PROJECT_PATH = ~/apps/hdmi_enc_mmnp/

ARMPATH =/opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/bin/
AR = ar
COMPILE.c = gcc
COMPILE.cpp = g++
LINK.c = gcc
LINK.cpp = g++


FLAGS = -Wall -c -D_GNU_SOURCE
LDFLAG = -lpthread

LIBPATH = ./lib
LIBFLAG = -L$(LIBPATH)
LDFLAG += -l$(LIBNAME)_$(ARCHNAME)
LIBFILE = $(LIBPATH)/lib$(LIBNAME)_$(ARCHNAME).a
SOURCES_LIB.c = $(wildcard $(SRCPATH)/*.c)
SOURCES_LIB.cpp = $(wildcard $(SRCPATH)/*.cpp)
SOURCES_TEST = $(wildcard ../*.c)
OBJFILES_LIB.c = $(SOURCES_LIB.c:%.c=%.o)
OBJFILES_LIB.cpp = $(SOURCES_LIB.cpp:%.cpp=%.o)
OBJFILES_LIB = $(OBJFILES_LIB.c)
OBJFILES_LIB += $(OBJFILES_LIB.cpp)
OBJFILES_TEST = $(SOURCES_TEST:%.c=%.o)
OUTFILE = ./$(LIBNAME)_demo_$(ARCHNAME)
#lib files
TSLIB = $(SRCPATH)/libTSpacket_$(ARCHNAME).a
LIVELIB = $(SRCPATH)/liblive555_$(ARCHNAME).a
RTMPLIB = $(SRCPATH)/librtmpsender_$(ARCHNAME).a
#LIVELIB = $(SRCPATH)/libliveMedia_$(ARCHNAME).a $(SRCPATH)/libBasicUsageEnvironment_$(ARCHNAME).a $(SRCPATH)/libUsageEnvironment_$(ARCHNAME).a $(SRCPATH)/libgroupsock_$(ARCHNAME).a
#CPPINCLUDES = -I/usr/local/include/groupsock -I/usr/local/include/UsageEnvironment -I/usr/local/include/liveMedia -I/usr/local/include/BasicUsageEnvironment

#debug
FLAGS += -g
#all:  $(LIBFILE)	
all: $(OUTFILE)	
	@echo Done!

#release:  $(OUTFILE)
#	@echo Copy Libfile $@ to project dir $(HDMI_ENC_PROJECT_PATH)
#	$(VERBOSE) cp $(LIBFILE) $(HDMI_ENC_PROJECT_PATH)
#	$(VERBOSE) cp $(APIHEAD) $(HDMI_ENC_PROJECT_PATH)
#	@echo Done!

$(OUTFILE): $(OBJFILES_TEST) $(LIBFILE)
	@echo Linking $@ from $^ ..
	$(VERBOSE) $(LINK.cpp) $(LIBFLAG) -o $@ $^ $(LDFLAG)

$(OBJFILES_TEST): %.o:%.c
	@echo Compiling $(shell basename $@) from $(shell basename $<) ..
	$(VERBOSE) $(COMPILE.c) $(FLAGS) -o $@ $<

lib: $(LIBFILE)
$(LIBFILE): $(OBJFILES_LIB.c) $(OBJFILES_LIB.cpp)
#	@echo SOURCES_LIB.c=$(SOURCES_LIB.c)
#	@echo OBJFILES_LIB=$(OBJFILES_LIB)
	@echo Generating Libfile $@ from $(shell basename $<) ..
	$(VERBOSE) cp $(TSLIB) $(LIBPATH)
	$(VERBOSE) $(AR) x $(TSLIB)
	$(VERBOSE) cp $(RTMPLIB) $(LIBPATH)
	$(VERBOSE) $(AR) x $(RTMPLIB)
	$(VERBOSE) cp $(LIVELIB) $(LIBPATH)
	$(foreach livefile,$(LIVELIB),$(shell $(AR) x $(livefile)))
	$(VERBOSE) mv *.o $(LIBPATH)
	$(VERBOSE) $(AR) rc $@ $^ $(LIBPATH)/*.o
	$(VERBOSE) rm $(LIBPATH)/*.o
	
$(OBJFILES_LIB.c): %.o:%.c
	@echo 1.Compiling $(shell basename $@) from $(shell basename $<) ..
	$(VERBOSE) $(COMPILE.c) $(FLAGS) -o $@ $<

$(OBJFILES_LIB.cpp): %.o:%.cpp
	@echo 2.Compiling $(shell basename $@) from $(shell basename $<) ..
	$(VERBOSE) $(COMPILE.cpp) $(FLAGS) -o $@ $<

	
lib: $(LIBFILE)
clean:
	@echo Removing ganerated files...
	$(VERBOSE) rm -rf $(OBJFILES_LIB) $(OBJFILES_TEST) $(OUTFILE) $(LIBFILE) $(LIBPATH)/*.o *.o
