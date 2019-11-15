// Copyright Nezametdinov E. Ildus 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
#include <pulse/pulseaudio.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char const* speaker;
    char const* speaker_muted;

    char const* mic;
    char const* mic_muted;
} pvm_config;

typedef struct {
    uint32_t idx;
    bool is_idx_valid;
} pvm_device_desc;

typedef struct {
    pa_volume_t volume;
    bool is_muted;
} pvm_volume_info;

typedef struct {
    pvm_config cfg;

    pa_mainloop* mainloop;
    pa_context* context;

    pvm_device_desc sink_desc, source_desc;
    pvm_volume_info sink_info, source_info;

    bool has_data_changed;
} pvm_state;

static char const* pvm_program_name = "pavolmon";

// State initialization/destruction functions.
//

static void
pvm_initialize(pvm_state* x, pvm_config cfg);

static void
pvm_destroy(pvm_state* x);

// PulseAudio utility functions.
//

static void
pvm_unref(pa_operation* x);

static int
pvm_normalize_volume(pa_volume_t x);

static bool
pvm_update_volume_info(pvm_volume_info* info, pa_volume_t volume,
                       bool is_muted);

// PulseAudio event callback functions.
//

static void
pvm_context_state_callback(pa_context* context, void* userdata);

static void
pvm_context_subscription_event_callback(pa_context* context,
                                        pa_subscription_event_type_t type,
                                        uint32_t idx, void* userdata);

static void
pvm_server_info_callback(pa_context* context, pa_server_info const* info,
                         void* userdata);

static void
pvm_sink_info_callback(pa_context* context, pa_sink_info const* info, int eol,
                       void* userdata);

static void
pvm_source_info_callback(pa_context* context, pa_source_info const* info,
                         int eol, void* userdata);

// Output functions.
//

static void
pvm_print_usage(FILE* stream);

static void
pvm_print_program_description(FILE* stream);

static void
pvm_print_error_message_and_exit(char const* message);

static void
pvm_print_volume_data(pvm_state* x);

//
// Implementation of the state initialization/destruction functions.
//

void
pvm_initialize(pvm_state* x, pvm_config cfg) {
    *x = (pvm_state){.cfg = cfg, .mainloop = pa_mainloop_new()};

    if(x->mainloop == NULL) {
        exit(EXIT_FAILURE);
    }

    x->context = pa_context_new(
        pa_mainloop_get_api(x->mainloop), "PulseAudio volume monitor");
    if(x->context == NULL) {
        exit(EXIT_FAILURE);
    }

    pa_context_set_state_callback(x->context, pvm_context_state_callback, x);

    if(pa_context_connect(
           x->context, NULL,
           (pa_context_flags_t)(PA_CONTEXT_NOAUTOSPAWN | PA_CONTEXT_NOFAIL),
           NULL) < 0) {
        exit(EXIT_FAILURE);
    }
}

void
pvm_destroy(pvm_state* x) {
    if(x->context != NULL) {
        pa_context_unref(x->context);
    }

    if(x->mainloop != NULL) {
        pa_mainloop_free(x->mainloop);
    }
}

//
// Implementation of the PulseAudio utility functions.
//

void
pvm_unref(pa_operation* x) {
    if(x != NULL) {
        pa_operation_unref(x);
    }
}

int
pvm_normalize_volume(pa_volume_t x) {
    static_assert(PA_VOLUME_NORM > PA_VOLUME_MUTED, "");
    static pa_volume_t const volume_range = PA_VOLUME_NORM - PA_VOLUME_MUTED;

    x -= PA_VOLUME_MUTED;

    uint32_t p = (uint32_t)(((double)(x) / volume_range) * 100.0 + 0.5);
    p = ((p > 999U) ? 999U : p);

    return (int)(p);
}

bool
pvm_update_volume_info(pvm_volume_info* info, pa_volume_t volume,
                       bool is_muted) {
    if((info->volume == volume) && (info->is_muted == is_muted)) {
        return false;
    }

    info->volume = volume;
    info->is_muted = is_muted;

    return true;
}

//
// Implementation of the PulseAudio event callback functions.
//

void
pvm_context_state_callback(pa_context* context, void* userdata) {
    switch(pa_context_get_state(context)) {
        case PA_CONTEXT_CONNECTING:
            /*fallthrough*/;
        case PA_CONTEXT_AUTHORIZING:
            /*fallthrough*/;
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            pa_context_set_subscribe_callback(
                context, pvm_context_subscription_event_callback, userdata);
            pvm_unref(pa_context_subscribe(
                context,
                (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK |
                                         PA_SUBSCRIPTION_MASK_SOURCE |
                                         PA_SUBSCRIPTION_MASK_SERVER),
                NULL, NULL));
            pvm_unref(pa_context_get_server_info(
                context, pvm_server_info_callback, userdata));
            break;

        case PA_CONTEXT_TERMINATED:
            pa_mainloop_quit(((pvm_state*)(userdata))->mainloop, EXIT_SUCCESS);
            break;

        case PA_CONTEXT_FAILED:
            /*fallthrough*/;
        default:
            exit(EXIT_FAILURE);
    }
}

void
pvm_context_subscription_event_callback(pa_context* context,
                                        pa_subscription_event_type_t type,
                                        uint32_t idx, void* userdata) {
    pvm_state* s = (pvm_state*)(userdata);
    switch(type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            if(s->sink_desc.is_idx_valid && (s->sink_desc.idx == idx)) {
                pvm_unref(pa_context_get_sink_info_by_index(
                    context, idx, pvm_sink_info_callback, userdata));
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            if(s->source_desc.is_idx_valid && (s->source_desc.idx == idx)) {
                pvm_unref(pa_context_get_source_info_by_index(
                    context, idx, pvm_source_info_callback, userdata));
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER:
            pvm_unref(pa_context_get_server_info(
                context, pvm_server_info_callback, userdata));
            break;

        default:
            break;
    }
}

void
pvm_server_info_callback(pa_context* context, pa_server_info const* info,
                         void* userdata) {
    if(info == NULL) {
        return;
    }

    pvm_unref(pa_context_get_sink_info_by_name(
        context, info->default_sink_name, pvm_sink_info_callback, userdata));
    pvm_unref(
        pa_context_get_source_info_by_name(context, info->default_source_name,
                                           pvm_source_info_callback, userdata));
}

#define info_callback_impl_(type)                                           \
    /* Prevent compiler warnings. */ {                                      \
        (void)(context);                                                    \
        (void)(eol);                                                        \
    }                                                                       \
                                                                            \
    if(info == NULL) {                                                      \
        return;                                                             \
    }                                                                       \
                                                                            \
    pvm_state* s = (pvm_state*)(userdata);                                  \
    pvm_device_desc* desc = &(s->type##_desc);                              \
                                                                            \
    desc->idx = info->index;                                                \
    desc->is_idx_valid = true;                                              \
                                                                            \
    if(pvm_update_volume_info(                                              \
           &(s->type##_info), info->volume.values[0], (info->mute != 0))) { \
        s->has_data_changed = true;                                         \
    }

void
pvm_sink_info_callback(pa_context* context, pa_sink_info const* info, int eol,
                       void* userdata) {
    info_callback_impl_(sink);
}

void
pvm_source_info_callback(pa_context* context, pa_source_info const* info,
                         int eol, void* userdata) {
    info_callback_impl_(source);
}

#undef info_callback_impl_

//
// Implementation of the output functions.
//

void
pvm_print_usage(FILE* stream) {
    fprintf(stream,
            "usage: %s [-h] [-f SPEAKER MUTED_SPEAKER MICROPHONE "
            "MUTED_MICROPHONE]\n",
            pvm_program_name);
}

void
pvm_print_program_description(FILE* stream) {
    fprintf(stream,
            "Asynchronously monitors the state of the PulseAudio server and "
            "outputs the\n"
            "volume of the default sink (audio output) and source (audio "
            "input).\n\n");
    pvm_print_usage(stream);

    fprintf(stream,
            "optional arguments:\n"
            "  -h        show this help message and exit\n"
            "  -f SPEAKER MUTED_SPEAKER MICROPHONE MUTED_MICROPHONE\n"
            "            use the given labels for speaker, muted speaker, "
            "microphone and\n"
            "            muted microphone\n");
}

void
pvm_print_error_message_and_exit(char const* message) {
    fprintf(stderr, "%s: error: %s\n", pvm_program_name, message);
    pvm_print_usage(stderr);

    exit(EXIT_FAILURE);
}

void
pvm_print_volume_data(pvm_state* x) {
    if(x->has_data_changed) {
        x->has_data_changed = false;

        fprintf(stdout, "%s%3d%% %s%3d%%\n",
                (x->sink_info.is_muted ? x->cfg.speaker_muted : x->cfg.speaker),
                pvm_normalize_volume(x->sink_info.volume),
                (x->source_info.is_muted ? x->cfg.mic_muted : x->cfg.mic),
                pvm_normalize_volume(x->source_info.volume));
    }
}

//
// Program entry point.
//

int
main(int argc, char* argv[]) {
    pvm_config cfg = {.speaker = "VOL+",
                      .speaker_muted = "VOL-",
                      .mic = "MIC+",
                      .mic_muted = "MIC-"};

    if(argc == 2) {
        if(strcmp(argv[1], "-h") == 0) {
            pvm_print_program_description(stdout);
            return EXIT_SUCCESS;
        } else {
            pvm_print_error_message_and_exit("wrong number of arguments");
        }
    } else if(argc == 6) {
        if(strcmp(argv[1], "-f") == 0) {
            cfg.speaker = argv[2];
            cfg.speaker_muted = argv[3];
            cfg.mic = argv[4];
            cfg.mic_muted = argv[5];
        } else {
            pvm_print_error_message_and_exit("unknown argument");
        }
    } else if(argc > 1) {
        pvm_print_error_message_and_exit("wrong number of arguments");
    }

    pvm_state app = {};
    pvm_initialize(&app, cfg);

    while(pa_mainloop_iterate(app.mainloop, true, NULL) >= 0) {
        pvm_print_volume_data(&app);
    }

    pvm_destroy(&app);
    return EXIT_SUCCESS;
}
