//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#ifdef DLL_NO_TIMERS

namespace dll {

inline void dump_timers(){
    //No timers
}

struct auto_timer {
    auto_timer(const char* /*name*/) { }
};

} //end of namespace dll

#else

#include <chrono>

namespace chrono = std::chrono;

namespace dll {

constexpr const std::size_t max_timers = 32;

struct timer_t {
    const char* name;
    std::atomic<std::size_t> count;
    std::atomic<std::size_t> duration;

    timer_t() : name(nullptr), count(0), duration(0) {}
};

struct timers_t {
    std::array<timer_t, max_timers> timers;
    std::mutex lock;
};

inline timers_t& get_timers(){
    static timers_t timers;
    return timers;
}

inline std::string to_string_precision(double duration, int precision = 6){
    std::ostringstream out;
    out << std::setprecision(precision) << duration;
    return out.str();
}

inline std::string duration_str(double duration, int precision = 6){
    if(duration > 1000.0 * 1000.0 * 1000.0){
        return to_string_precision(duration / (1000.0 * 1000.0 * 1000.0), precision) + "s";
    } else if(duration > 1000.0 * 1000.0){
        return to_string_precision(duration / (1000.0 * 1000.0), precision) + "ms";
    } else if(duration > 1000.0){
        return to_string_precision(duration / 1000.0, precision) + "us";
    } else {
        return to_string_precision(duration, precision) + "ns";
    }
}

inline void dump_timers(){
    decltype(auto) timers = get_timers();
    for (std::size_t i = 0; i < max_timers; ++i) {
        decltype(auto) timer = timers.timers[i];

        if (timer.name) {
            std::cout << timer.name << "(" << timer.count << ") : " << duration_str(timer.duration) << std::endl;
        }
    }
}

struct auto_timer {
    const char* name;
    chrono::time_point<chrono::steady_clock> start;
    chrono::time_point<chrono::steady_clock> end;

    auto_timer(const char* name) : name(name) {
        start = chrono::steady_clock::now();
    }

    ~auto_timer(){
        end           = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

        decltype(auto) timers = get_timers();

        for (std::size_t i = 0; i < max_timers; ++i) {
            decltype(auto) timer = timers.timers[i];

            if (timer.name == name) {
                timer.duration += duration;
                ++timer.count;

                return;
            }
        }

        std::lock_guard<std::mutex> lock(timers.lock);

        for (std::size_t i = 0; i < max_timers; ++i) {
            decltype(auto) timer = timers.timers[i];

            if (timer.name == name) {
                timer.duration += duration;
                ++timer.count;

                return;
            }
        }

        for (std::size_t i = 0; i < max_timers; ++i) {
            decltype(auto) timer = timers.timers[i];

            if (!timer.name) {
                timer.name = name;
                timer.duration = duration;
                timer.count = 1;

                return;
            }
        }

        std::cerr << "Unable to register timer " << name << std::endl;
    }
};

} //end of namespace dll

#endif