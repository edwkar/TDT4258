AVCONV     = avconv
AVCONVOPTS = -f u8 -acodec pcm_u8 -ar 22000 -ac 2

INTRO    = intro.raw
FIRE     = fire.raw
EXPLODE  = explode.raw

all: $(INTRO) $(FIRE) $(EXPLODE)

$(INTRO): 40547__erh__a-minor-12-sec-intro-b.wav
	$(AVCONV) -i $^ $(AVCONVOPTS) $@

$(FIRE): 162397__qubodup__m16-single-shot-2.flac
	$(AVCONV) -i $^ $(AVCONVOPTS) $@

$(EXPLODE): 35643__sandyrb__usat-bomb.wav
	$(AVCONV) -i $^ $(AVCONVOPTS) $@

.PHONY: clean
clean:
	rm -f *.raw
