VERBOSE = @

#CC = gcc
#AR = ar
CC = arm_v5t_le-gcc
AR = arm_v5t_le-ar
FLAGS = -Wall -c
TARGET = release/libvis_util.a
SOURCES = $(wildcard src/*.c)
OBJFILES = $(SOURCES:%.c=%.o)
VIS_UTIL_INCLUDE = -I/opt/dvsdk_dm365/dvsdk_demos_2_10_00_17/apps/

all: $(TARGET)

$(TARGET): $(OBJFILES)
#	@echo SOURCES=$(SOURCES)
#	@echo OBJFILES=$(OBJFILES)
	$(VERBOSE) $(AR) rc $@ $^

$(OBJFILES): %.o:%.c
	@echo Compiling $@ from $< ..
	$(VERBOSE) $(CC) $(FLAGS) $(VIS_UTIL_INCLUDE) -o $@ $<

clean:
	@echo Removing ganerated files...
	$(VERBOSE) rm -rf $(OBJFILES) $(TARGET)
