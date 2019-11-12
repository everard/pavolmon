// Copyright(c) 2019 Nezametdinov E. Ildus.
// See the LICENCE file for licensing conditions.
//
#include <cstddef>
#include <cstdint>

#include <string>
#include <iomanip>
#include <iostream>

#include <pulse/pulseaudio.h>

namespace svq {

using ::std::uint32_t;

struct config {
    ::std::string speaker, speaker_muted;
    ::std::string mic, mic_muted;
};

struct state {
    state(config const& c) : cfg{c}, context{}, mainloop{pa_mainloop_new()} {
        if(mainloop == nullptr) {
            ::std::exit(EXIT_FAILURE);
        }

        context = pa_context_new(
            pa_mainloop_get_api(mainloop), "PulseAudio volume monitor");
        if(context == nullptr) {
            ::std::exit(EXIT_FAILURE);
        }

        pa_context_set_state_callback(context, context_state_callback, this);

        if(pa_context_connect(context, nullptr,
                              static_cast<pa_context_flags_t>(
                                  PA_CONTEXT_NOAUTOSPAWN | PA_CONTEXT_NOFAIL),
                              nullptr) < 0) {
            ::std::exit(EXIT_FAILURE);
        }
    }

    ~state() {
        if(context) {
            pa_context_unref(context);
        }

        if(mainloop) {
            pa_mainloop_free(mainloop);
        }
    }

    void
    print_collected_volume_data() {
        static_assert(PA_VOLUME_NORM > PA_VOLUME_MUTED);
        static constexpr pa_volume_t volume_range =
            PA_VOLUME_NORM - PA_VOLUME_MUTED;

        auto normalize = [](pa_volume_t x) {
            x -= PA_VOLUME_MUTED;

            double p = (static_cast<double>(x) / volume_range) * 100.0 + 0.5;
            return static_cast<uint32_t>(p);
        };

        if(has_data_changed) {
            has_data_changed = false;

            ::std::cout << (sink.is_muted ? cfg.speaker_muted : cfg.speaker)
                        << ::std::setw(3) << normalize(sink.volume) << "% "
                        << (source.is_muted ? cfg.mic_muted : cfg.mic)
                        << ::std::setw(3) << normalize(source.volume) << '%'
                        << ::std::endl;
        }
    }

    static void
    unref(pa_operation* x) {
        if(x != nullptr) {
            pa_operation_unref(x);
        }
    }

    static void
    context_state_callback(pa_context* context, void* userdata) {
        auto self = static_cast<state*>(userdata);

        switch(pa_context_get_state(context)) {
            case PA_CONTEXT_CONNECTING:
                [[fallthrough]];
            case PA_CONTEXT_AUTHORIZING:
                [[fallthrough]];
            case PA_CONTEXT_SETTING_NAME:
                break;

            case PA_CONTEXT_READY:
                pa_context_set_subscribe_callback(
                    context, context_subscription_event_callback, userdata);
                unref(pa_context_subscribe(
                    context,
                    static_cast<pa_subscription_mask_t>(
                        PA_SUBSCRIPTION_MASK_SINK |
                        PA_SUBSCRIPTION_MASK_SINK_INPUT |
                        PA_SUBSCRIPTION_MASK_SOURCE |
                        PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT |
                        PA_SUBSCRIPTION_MASK_SERVER),
                    nullptr, nullptr));
                unref(pa_context_get_server_info(
                    context, server_info_callback, userdata));
                break;

            case PA_CONTEXT_TERMINATED:
                pa_mainloop_quit(self->mainloop, EXIT_SUCCESS);
                break;

            case PA_CONTEXT_FAILED:
                [[fallthrough]];
            default:
                ::std::exit(EXIT_FAILURE);
        }
    }

    static void
    context_subscription_event_callback(pa_context* context,
                                        pa_subscription_event_type_t type,
                                        uint32_t, void* userdata) {
        switch(type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
            case PA_SUBSCRIPTION_EVENT_SINK:
                [[fallthrough]];
            case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
                [[fallthrough]];
            case PA_SUBSCRIPTION_EVENT_SOURCE:
                [[fallthrough]];
            case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
                [[fallthrough]];
            case PA_SUBSCRIPTION_EVENT_SERVER:
                unref(pa_context_get_server_info(
                    context, server_info_callback, userdata));
                break;

            default:
                break;
        }
    }

    static void
    server_info_callback(pa_context* context, pa_server_info const* info,
                         void* userdata) {
        if(info == nullptr) {
            return;
        }

        unref(pa_context_get_sink_info_by_name(
            context, info->default_sink_name, sink_info_callback, userdata));
        unref(pa_context_get_source_info_by_name(
            context, info->default_source_name, source_info_callback,
            userdata));
    }

    static void
    sink_info_callback([[maybe_unused]] pa_context* context,
                       pa_sink_info const* info, [[maybe_unused]] int eol,
                       void* userdata) {
        if(info == nullptr) {
            return;
        }

        auto self = static_cast<state*>(userdata);
        self->has_data_changed = true;
        self->sink = volume_data{info->volume.values[0], info->mute != 0};
    }

    static void
    source_info_callback([[maybe_unused]] pa_context* context,
                         pa_source_info const* info, [[maybe_unused]] int eol,
                         void* userdata) {
        if(info == nullptr) {
            return;
        }

        auto self = static_cast<state*>(userdata);
        self->has_data_changed = true;
        self->source = volume_data{info->volume.values[0], info->mute != 0};
    }

    struct volume_data {
        pa_volume_t volume;
        bool is_muted;
    };

    config cfg;

    pa_context* context;
    pa_mainloop* mainloop;

    volume_data sink{}, source{};
    bool has_data_changed{false};
};

} // namespace svq

int
main() {
    ::svq::config cfg{u8"\xF0\x9F\x94\x8A+", u8"\xF0\x9F\x94\x87-",
                      u8"\xF0\x9F\x8E\xA4+", u8"\xF0\x9F\x8E\xA4-"};
    ::svq::state app{cfg};

    while(pa_mainloop_iterate(app.mainloop, true, nullptr) >= 0) {
        app.print_collected_volume_data();
    }

    return EXIT_SUCCESS;
}
