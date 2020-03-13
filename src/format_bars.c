// Copyright Nezametdinov E. Ildus 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//

// Variables from the outer scope:
// * cfg - contains string labels for speaker and mic;
// * speaker_volume - an int that represents the output volume;
// * mic_volume - an int that represents the input volume.
// * is_speaker_muted - a boolean that represents the state of the speaker;
// * is_mic_muted - a boolean that represents the state of the mic.

static char const* const bars[] = {u8" ",
                                   u8"\xE2\x96\x81",
                                   u8"\xE2\x96\x82",
                                   u8"\xE2\x96\x83",
                                   u8"\xE2\x96\x84",
                                   u8"\xE2\x96\x85",
                                   u8"\xE2\x96\x86",
                                   u8"\xE2\x96\x87",
                                   u8"\xE2\x96\x88",
                                   u8"X"};

#define select_label_(x)              \
    if(is_##x##_muted) {              \
        x##_idx = 9;                  \
    } else {                          \
        if(x##_volume >= 100) {       \
            x##_idx = 8;              \
        } else if(x##_volume >= 88) { \
            x##_idx = 7;              \
        } else if(x##_volume >= 75) { \
            x##_idx = 6;              \
        } else if(x##_volume >= 63) { \
            x##_idx = 5;              \
        } else if(x##_volume >= 50) { \
            x##_idx = 4;              \
        } else if(x##_volume >= 38) { \
            x##_idx = 3;              \
        } else if(x##_volume >= 20) { \
            x##_idx = 2;              \
        } else if(x##_volume > 0) {   \
            x##_idx = 1;              \
        }                             \
    }

int speaker_idx = 0;
select_label_(speaker);

int mic_idx = 0;
select_label_(mic);

fprintf(stdout, "%s%s %s%s\n", cfg.speaker, bars[speaker_idx], cfg.mic,
        bars[mic_idx]);
