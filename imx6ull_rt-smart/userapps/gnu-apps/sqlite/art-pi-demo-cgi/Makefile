CROSS_COMPILE= arm-linux-musleabi-
CC= $(CROSS_COMPILE)gcc
CXX= $(CROSS_COMPILE)g++

PWD := $(shell pwd)
# userapp dir
ROOTDIR := $(PWD)/../../..
RT_DIR=$(ROOTDIR)/sdk/rt-thread

CFLAGS=-march=armv7-a -marm -msoft-float -D__RTTHREAD__ -DHAVE_CCONFIG_H -Werror -Wall -O0 -g -gdwarf-2 -n --static
CFLAGS+= -I. -I$(ROOTDIR) -I$(RT_DIR)/include -I$(RT_DIR)/components/dfs -I$(RT_DIR)/components/drivers -I$(RT_DIR)/components/finsh -I$(RT_DIR)/components/net

LDFLAGS=-march=armv7-a -marm -msoft-float -T $(ROOTDIR)/linker_scripts/arm/cortex-a/link.lds
LDFLAGS+= -L$(RT_DIR)/lib -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive -n --static -Wl,--start-group -lrtthread -Wl,--end-group

default:
	$(CC) $(CFLAGS) -o sysinfo.o -c sysinfo.c
	$(CXX) $(LDFLAGS) -o sysinfo.elf sysinfo.o

clean:
	@rm -f *.o *.elf
