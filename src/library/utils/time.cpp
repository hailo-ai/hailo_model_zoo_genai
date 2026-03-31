/**
 * Copyright (c) 2019-2026 Hailo Technologies Ltd. All rights reserved.
 * Distributed under the MIT license (https://opensource.org/licenses/MIT)
 **/
/**
 * @file time.cpp
 * @brief time utilities implementation
 **/

#include "utils/time.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace hailo_ollama
{

std::string to_iso_8601(std::chrono::time_point<std::chrono::system_clock> t, const std::string &suffix)
{
    // convert to time_t which will represent the number of
    // seconds since the UNIX epoch, UTC 00:00:00 Thursday, 1st. January 1970
    time_t epoch_seconds = std::chrono::system_clock::to_time_t(t);

    // Format this as date time to seconds resolution
    // e.g. 2016-08-30T08:18:51
    std::ostringstream stream;
    struct tm buf = get_utc_time_struct(epoch_seconds);
    stream << std::put_time(&buf, "%FT%T");

    // If we now convert back to a time_point we will get the time truncated
    // to whole seconds
    auto truncated = std::chrono::system_clock::from_time_t(epoch_seconds);

    // Now we subtract this seconds count from the original time to
    // get the number of extra microseconds..
    auto delta_us = std::chrono::duration_cast<std::chrono::nanoseconds>(t - truncated).count();

    // e.g. 2016-08-30T08:18:51.867479
    // And append this to the output stream as fractional seconds
    stream << "." << std::fixed << std::setw(9) << std::setfill('0') << delta_us;
    //
    // Add final suffix
    stream << suffix;

    return stream.str();
}


struct tm get_utc_time_struct(time_t epoch_seconds)
{
    struct tm buf;
#ifdef _WIN32
        gmtime_s(&buf, &epoch_seconds);
#else
        buf = *gmtime_r(&epoch_seconds, &buf);
#endif
    return buf;
}


std::string get_current_time_formatted()
{
    auto now = std::chrono::system_clock::now();
    return to_iso_8601(now, "Z");
}

std::string to_iso_8601(std::chrono::time_point<std::chrono::steady_clock> t, const std::string &suffix)
{
    return to_iso_8601(
        // we have to manually convert filetime_clock to system_clock
        // results are not accurate because of the two calls to now
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(t - decltype(t)::clock::now() +
            std::chrono::system_clock::now()), suffix);
}

std::string to_iso_8601(std::filesystem::file_time_type t, const std::string &suffix)
{
    // C++20 (and later) code has cast between clocks
    // return to_iso_8601(
    //     std::chrono::clock_cast<std::chrono::system_clock>(t),
    //     suffix);
    return to_iso_8601(
        // we have to manually convert filetime_clock to system_clock
        // results are not accurate because of the two calls to now
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(t - decltype(t)::clock::now() +
            std::chrono::system_clock::now()), suffix);
}

} // namespace hailo_ollama
