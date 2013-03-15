"""
Dirt simple Python script for converting MIDI files to
our custom "struct melody" format.

Does the job, but could definitely be cleaned up to read better :-)
"""

import sys
import python_midi as midi

BASE_NOTE = 0
BEGIN, END = 'BEGIN', 'END'
NOTE_PAUSE = '/'
NOTE_REPEAT = '.'

def note_for_midi_note(v):
    return chr(ord('0') + v - 3*12)


# Read command line options...
name = sys.argv[2]
scaling = int(sys.argv[3])
shift = int(sys.argv[4])
voices_to_read = map(int, sys.argv[5].split(','))
cut_end_time = int(sys.argv[6]) if len(sys.argv) == 7 else 1e20


translated_events = []

# Read MIDI events...
f = midi.read_midifile(sys.argv[1])
for i, voice in enumerate(f):
    if i not in voices_to_read:
        continue
    t = 0
    for ev in voice:
        t += ev.tick
        if isinstance(ev, midi.NoteOnEvent):
            # Some MIDI files have zero-volume NoteOn events representing
            # note releases. (Wtf..? :)
            if ev.data[1] == 0:
                translated_events.append([t, END, ev.data[0]])
            else:
                translated_events.append([t, BEGIN, ev.data[0]])
        elif isinstance(ev, midi.NoteOffEvent):
            translated_events.append([t, END, ev.data[0]])


# Sort events on time...
translated_events.sort(lambda a, b: cmp(a[0], b[0]))


# Shuffle the data into individual voices...
voices = []
while translated_events:
    assert(len([1 for ev in translated_events if ev[1] == BEGIN]) ==
           len([1 for ev in translated_events if ev[1] == END]))

    voice = [0, -1]
    has_note = False

    to_remove = set()
    for i, (t, type_, note) in enumerate(translated_events[:]):
        t = scaling*t
        note += shift
        while note < 0:
            note += 12
        assert note >= 0
        last_note = None if not voice else voice[-1]
        if not has_note and type_ == BEGIN:
            voice.append(t)
            voice.append(note)
            has_note = True
            to_remove.add(i)
        elif has_note and type_ == END and note == last_note:
            voice.append(t)
            voice.append(-1)
            to_remove.add(i)
            has_note = False

    assert len(to_remove)%2 == 0
    translated_events = [t for i, t in enumerate(translated_events)
                         if not i in to_remove]
    voices.append(voice)


# Trim possible silence from the start of the voices...
cut_start_time = 1e6
for v in voices:
    assert v[0] == 0
    if v[1] != -1:
        cut_start_time = 0
        break
    cut_start_time = min(cut_start_time, v[2])
for v in voices:
    for i in range(len(v)//2):
        v[2*i] -= cut_start_time


# Cut at the specified song end point...
for i in range(len(voices)):
    v = []
    for j in range(len(voices[i])//2):
        if voices[i][2*j] > cut_end_time:
            break
        v.append(voices[i][2*j])
        v.append(voices[i][2*j+1])
    voices[i] = v


# Add a sentinel pause note at the end of each voice...
song_len = max(map(max, voices))
for v in voices:
    v += [song_len+1, -1]


# Output the final structure...
print """\
{
 .name = "%s",
 .num_voices = %d,
 .len = %d,
 .voices = {\
""" % (name, len(voices), song_len,)
for i, v in enumerate(voices):
    print ' {', ','.join(map(str, v)), '}',
    print ',' if i < len(voices)-1 else '',
    print
print """\
 /* cut: %d. */
 }
}
""" % (cut_start_time,)
