.PHONY: intro.mpeg2
intro.mpeg2:
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
