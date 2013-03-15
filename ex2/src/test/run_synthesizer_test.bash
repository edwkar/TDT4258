cd .. 
make melodies__rendered.c
cd test
gcc -std=gnu99 -I.. -O2 synthesizer_test.c ../synthesizer.c ../melodies.c\
    ../utils.c -lm -o synthesizer_test && \
./synthesizer_test got_loop | aplay -f S16_LE -c 1 -r 48000
rm -f synthesizer_test
