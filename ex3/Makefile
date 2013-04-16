# vim: set ts=8:

SH = /bin/bash

HOSTCC  = clang
CROSSCC = avr32-linux-gcc


OPT_LEVEL = 3

FFMPEG = /home/edvardkk/dev/ffmpeg/


CFLAGS = -Iscorched -Ishared \
	 -O$(OPT_LEVEL) \
	 -fomit-frame-pointer -funroll-loops -fno-common \
	 -std=gnu99 -fgnu89-inline \
	 -Wall -Wextra -Winline -Werror \
         -DSC_RESOURCES_PATH='"scorched-resources/"'\
	 -g

LDFLAGS = -lm -lpthread -lrt -fuse-linker-plugin


HOSTCFLAGS = $(CFLAGS) -DHOST_BUILD\
		       -DDSP_PATH=$(MOCK_DSP_PATH) \
		       -DFRAME_BUFFER_PATH=$(MOCK_FRAME_BUFFER_PATH)\
		       -DINPUT_DRIVER_PATH=$(MOCK_INPUT_DRIVER_PATH)\
		       -g\
		       -fcolor-diagnostics\
		       -Weverything -Wno-padded -Wno-gnu -Wno-disabled-macro-expansion
HOSTLDFLAGS = $(LDFLAGS) -g -lSDL 

CROSSCFLAGS = $(CFLAGS) -I$(FFMPEG)\
                        -DDSP_PATH='"/dev/dsp"'\
	                -DFRAME_BUFFER_PATH='"/dev/fb0"'\
			-DINPUT_DRIVER_PATH='"/dev/stk1000switches"'
CROSSLDFLAGS = $(LDFLAGS)


MOCK_DSP_PATH = '"/tmp/__scland__mockdsp"'
MOCK_FRAME_BUFFER_PATH = '"/tmp/__scland__mockfb"'
MOCK_INPUT_DRIVER_PATH = '"/tmp/__scland__mockinput"'

SCORCHED = scland
SCORCHED_HOST = scland.host

SCORCHED_SRCS = $(shell find scorched -name '*.c')
SCORCHED_HDRS = $(shell find scorched -name '*.h')
SCORCHED_OBJS = $(SCORCHED_SRCS:%.c=%.o)
SCORCHED_OBJS_HOST = $(SCORCHED_SRCS:%.c=%.host.o)

SHARED_SRCS = $(shell find shared -name '*.c')
SHARED_HDRS = $(shell find shared -name '*.h')
SHARED_OBJS = $(SHARED_SRCS:%.c=%.o)
SHARED_OBJS_HOST = $(SHARED_SRCS:%.c=%.host.o)

MOCKENV = mockenv
MOCKENV_SRCS = $(shell find host-mock-env -name '*.c')
MOCKENV_HDRS = $(shell find host-mock-env -name '*.h')
MOCKENV_OBJS = $(MOCKENV_SRCS:%.c=%.host.o)


all: $(SCORCHED) $(SCORCHED_HOST) $(MOCKENV) driver
	@du -sh $(SCORCHED)
	@du -sh $(SCORCHED_HOST)


$(SCORCHED): $(SCORCHED_HDRS) $(SCORCHED_OBJS) $(SHARED_HDRS) $(SHARED_OBJS)
	$(CROSSCC) $(SCORCHED_OBJS) $(SHARED_OBJS) $(CROSSLDFLAGS) -o $@

$(SCORCHED_HOST): $(SCORCHED_HDRS) $(SCORCHED_OBJS_HOST) \
	          $(SHARED_HDRS) $(SHARED_OBJS_HOST)
	$(HOSTCC) $(SCORCHED_OBJS_HOST) $(SHARED_OBJS_HOST) \
	          $(HOSTLDFLAGS) -o $@

$(MOCKENV): $(MOCKENV_HDRS) $(MOCKENV_OBJS) \
	    $(SHARED_HDRS) $(SHARED_OBJS_HOST)
	$(HOSTCC) $(MOCKENV_OBJS) $(SHARED_OBJS_HOST) $(HOSTLDFLAGS) -o $@

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
	                       --show-reachable=yes \
		               --track-fds=yes ./$(SCORCHED_HOST)"

.PHONY: host-sc-testnov
host-sc-testnov: all 
	./$(MOCKENV) "./$(SCORCHED_HOST)"

.PHONY: clean
clean: 
	find . -name "*.o" | xargs rm -f
	rm -f $(SCORCHED) $(SCORCHED_HOST) $(MOCKENV)
	cd driver; make clean


.PHONY: scorched-resources/movies/intro.mpeg2
scorched-resources/movies/intro.mpeg2:
	/home/edvardkk/dev/meh/ffmpeg/ffmpeg \
		-v debug -i /home/edvardkk/Downloads/intro_input.mov \
		-f rawvideo \
	    	-vcodec mpeg2video \
	    	-qmin 5 -qmax 5 \
		-vf "crop=1280,560,0,0" \
		-s 320x140 \
		-vf vflip \
		-ss 9.2 \
		-r 15 -frames 101 \
		-y \
		-an $@
	#perl -e 'open F,shift; do { read(F,$$a,4); print scalar reverse($$a);} while(!eof(F));' /tmp/oter > $@
