
freq_La = 440
freq_Do = 261
freq_DoSharp = 277
freq_Re = 293
freq_ReSharp = 311
freq_Mi = 329
freq_Fa = 349
freq_FaSharp = 370
freq_Sol = 392
freq_SolSharp = 415
freq_Si = 494

# Define the tone structure
class Tone:
    def __init__(self, freq_hz, sustain_ms, comment):
        self.freq_hz = freq_hz
        self.sustain_ms = sustain_ms
        self.comment = comment

# Function to convert numeric music sheet to C++ code
def convert_to_cpp(numeric_music_sheet):
    tones:list = []
    note_duration = 200  # Default duration
    note = '/'
    i = 0
    last_freq = freq_Re
    while (note != '\0' and i < len(numeric_music_sheet)):
        note = numeric_music_sheet[i]
        if note.isdigit():
            freq = None
            if note == '1':
                freq = freq_Do
            elif note == '2':
                freq = freq_Re
            elif note == '3':
                freq = freq_Mi
            elif note == '4':
                freq = freq_Fa
            elif note == '5':
                freq = freq_Sol
            elif note == '6':
                freq = freq_La
            else:
                raise ValueError("Invalid note:", note)

            # Append the tone with adjusted duration
            tones.append(Tone(freq, note_duration, note))
            last_freq = freq

        elif note == '[':  # Adjust duration if '[' is encountered
            note_duration *= int(numeric_music_sheet[i + 1])
            print(int(numeric_music_sheet[i + 1]))
            i += 1
        elif note == ']':  # Reset duration if ']' is encountered
            note_duration = 200  # Reset to default duration
        elif note == ' ':
            tones.append(Tone(last_freq,  note_duration, note))
        i+=1
    return tones

# Input numeric music sheet
numeric_music_sheet = "245 3 122 245 56[1]656 263245 3 122245 56[1]656[2]245 3 122245 56[1]656 263245 3 122245 56[1]656[2] 434"

# Convert numeric music sheet to C++ code
tones = convert_to_cpp(numeric_music_sheet)

# Output C++ code
print("Tone tones[] = {")
for tone in tones:
    print("\t{.freq_hz = " + str(tone.freq_hz) + ", .sustain_ms = " + str(tone.sustain_ms) + "},//" + str(tone.comment))
print("};")
