// Copyright Nezametdinov E. Ildus 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

// Variables from the outer scope:
// * speaker - a string label for speaker;
// * speaker_volume - an int that represents the output volume;
// * mic - a string label for microphone;
// * mic_volume - an int that represents the input volume.

static char const* const bars[] = {u8" ",
                                   u8"\xE2\x96\x81",
                                   u8"\xE2\x96\x82",
                                   u8"\xE2\x96\x83",
                                   u8"\xE2\x96\x84",
                                   u8"\xE2\x96\x85",
                                   u8"\xE2\x96\x86",
                                   u8"\xE2\x96\x87",
                                   u8"\xE2\x96\x88"};

int speaker_idx = 0;
if(speaker_volume >= 100) {
    speaker_idx = 8;
} else if(speaker_volume >= 88) {
    speaker_idx = 7;
} else if(speaker_volume >= 75) {
    speaker_idx = 6;
} else if(speaker_volume >= 63) {
    speaker_idx = 5;
} else if(speaker_volume >= 50) {
    speaker_idx = 4;
} else if(speaker_volume >= 38) {
    speaker_idx = 3;
} else if(speaker_volume >= 25) {
    speaker_idx = 2;
} else if(speaker_volume >= 13) {
    speaker_idx = 1;
}

int mic_idx = 0;
if(mic_volume >= 100) {
    mic_idx = 8;
} else if(mic_volume >= 88) {
    mic_idx = 7;
} else if(mic_volume >= 75) {
    mic_idx = 6;
} else if(mic_volume >= 63) {
    mic_idx = 5;
} else if(mic_volume >= 50) {
    mic_idx = 4;
} else if(mic_volume >= 38) {
    mic_idx = 3;
} else if(mic_volume >= 25) {
    mic_idx = 2;
} else if(mic_volume >= 13) {
    mic_idx = 1;
}

fprintf(stdout, "%s%s %s%s\n", speaker, bars[speaker_idx], mic, bars[mic_idx]);
