rm *.raw
for i in $(ls *wav *flac); do 
    avconv -i $i -f u8 -acodec pcm_u8 -ar 22000 -ac 2 "$i.raw"; 
done
