SH = /bin/bash

HOSTCC = gcc
HOSTLD = ld
CROSSCC = avr32-linux-gcc
CROSSLD = avr32-linux-ld


CFLAGS = -Iscorched -Ishared -O2 -g -std=gnu99 \
		 -Wall -Wextra -Winline -Werror \
		 -DSC_RESOURCES_PATH='"scorched/resources/"'
LDFLAGS = -g -lm -lpthread


HOSTCFLAGS = $(CFLAGS) -DHOST_BUILD\
			           -DDSP_PATH=$(MOCK_DSP_PATH) \
			           -DFRAME_BUFFER_PATH=$(MOCK_FRAME_BUFFER_PATH)\
			           -DINPUT_DRIVER_PATH=$(MOCK_INPUT_DRIVER_PATH)
HOSTLDFLAGS = $(LDFLAGS) -lSDL

CROSSCFLAGS = $(CFLAGS) -DDSP_PATH='"/dev/dsp"'\
			            -DFRAME_BUFFER_PATH='"/dev/fb0"'\
						-DINPUT_DRIVER_PATH='"/dev/stk1000switches"'
CROSSLDFLAGS = $(LDFLAGS)


MOCK_DSP_PATH = '"/tmp/__scland__mockdsp"'
MOCK_FRAME_BUFFER_PATH = '"/tmp/__scland__mockfb"'
MOCK_INPUT_DRIVER_PATH = '"/tmp/__scland__mockinput"'

SCORCHED = scland
SCORCHED_HOST = scland.host

SCORCHED_SRCS = $(shell find scorched -name *.c)
SCORCHED_HDRS = $(shell find scorched -name *.h)
SCORCHED_OBJS = $(SCORCHED_SRCS:%.c=%.o)
SCORCHED_OBJS_HOST = $(SCORCHED_SRCS:%.c=%.host.o)

SHARED_SRCS = $(shell find shared -name *.c)
SHARED_HDRS = $(shell find shared -name *.h)
SHARED_OBJS = $(SHARED_SRCS:%.c=%.o)
SHARED_OBJS_HOST = $(SHARED_SRCS:%.c=%.host.o)

MOCKENV = mockenv
MOCKENV_SRCS = $(shell find host-mock-env -name *.c)
MOCKENV_HDRS = $(shell find host-mock-env -name *.h)
MOCKENV_OBJS = $(MOCKENV_SRCS:%.c=%.host.o)


all: $(SCORCHED) $(SCORCHED_HOST) $(MOCKENV) driver


$(SCORCHED): $(SCORCHED_HDRS) $(SCORCHED_OBJS) $(SHARED_HDRS) $(SHARED_OBJS)
	$(CROSSCC) $(SCORCHED_OBJS) $(SHARED_OBJS) $(CROSSLDFLAGS) -o $@

$(SCORCHED_HOST): $(SCORCHED_HDRS) $(SCORCHED_OBJS_HOST) \
	              $(SHARED_HDRS) $(SHARED_OBJS_HOST)
	$(HOSTCC) $(SCORCHED_OBJS_HOST) $(SHARED_OBJS_HOST) $(HOSTLDFLAGS) -o $@

$(MOCKENV): $(MOCKENV_HDRS) $(MOCKENV_OBJS)
	$(HOSTCC) $(MOCKENV_OBJS) $(HOSTLDFLAGS) -o $@

%.host.o : %.c
	$(HOSTCC) $(HOSTCFLAGS) $< -o $@ -c

%.o : %.c
	$(CROSSCC) $(CROSSCFLAGS) $< -o $@ -c

.PHONY: driver
driver:
	$(MAKE) -C ./driver


# requiring "all" is really too much, but it doesn't hurt 
# to ensure that the cross-build is singing along.
.PHONY: host-sc-test
host-sc-test: all 
	./$(MOCKENV) "valgrind --leak-check=full \
		                   --track-fds=yes ./$(SCORCHED_HOST)"


.PHONY: clean
clean: 
	find . -name "*.o" | xargs rm -f
	rm -f $(SCORCHED) $(SCORCHED_HOST) $(MOCKENV)
	cd driver; make clean
