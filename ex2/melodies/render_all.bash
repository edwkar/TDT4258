cat << END
/*
 *  This source file was automatically compiled from 
 *  MIDI data at `date`. 
 *  
 *  DO *NOT* EDIT THIS FILE.                                                
 */
END


python render_midi.py midi_files/got.mid got_loop\
                                    40 -43 1,2 460800 
echo ,

python render_midi.py midi_files/2ndprld.mid prelude2 50 -40 \
                                    1,2 388800
echo ,

python render_midi.py midi_files/Dungeon01.mid dungeon 170 -30 \
                                    1,2,3,4,5,6 441800
echo ,

python render_midi.py midi_files/b-pre101.mid blipp 60 -20 \
                                    2,3,4,5,6 30000
