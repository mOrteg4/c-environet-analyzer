#include "util/time.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

namespace environet {
namespace util {

uint64_t Time::get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t Time::get_current_time_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t Time::get_current_time_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}

double Time::ms_to_seconds(uint64_t ms) { return static_cast<double>(ms) / 1000.0; }

uint64_t Time::seconds_to_ms(double seconds) {
    if (seconds <= 0.0) return 0;
    double val = seconds * 1000.0;
    if (val < 0.0) return 0;
    // clamp to uint64_t range
    if (val > static_cast<double>(std::numeric_limits<uint64_t>::max())) {
        return std::numeric_limits<uint64_t>::max();
    }
    return static_cast<uint64_t>(val + 0.5); // round to nearest
}

uint64_t Time::ms_to_us(uint64_t ms) { return ms * 1000ULL; }

uint64_t Time::us_to_ms(uint64_t us) { return us / 1000ULL; }

std::string Time::format_iso8601(uint64_t timestamp_ms) {
    std::time_t t = static_cast<time_t>(timestamp_ms / 1000ULL);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[64];
    // Include milliseconds to match expected length 24: YYYY-MM-DDTHH:MM:SS.mmmZ
    unsigned ms_part = static_cast<unsigned>(timestamp_ms % 1000ULL);
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03uZ",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec, ms_part);
    return std::string(buf);
}

std::string Time::format_human_readable(uint64_t timestamp_ms) {
    std::time_t t = static_cast<time_t>(timestamp_ms / 1000ULL);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    return std::string(buf);
}

uint64_t Time::parse_iso8601(const std::string& iso_string) {
    if (iso_string.empty()) return 0;
    // Accept formats like YYYY-MM-DDTHH:MM:SSZ (no fractional seconds)
    std::tm tm{};
    if (iso_string.size() < 20) return 0;
    // Basic parsing without strptime to keep portability
    try {
        tm.tm_year = std::stoi(iso_string.substr(0, 4)) - 1900;
        tm.tm_mon  = std::stoi(iso_string.substr(5, 2)) - 1;
        tm.tm_mday = std::stoi(iso_string.substr(8, 2));
        tm.tm_hour = std::stoi(iso_string.substr(11, 2));
        tm.tm_min  = std::stoi(iso_string.substr(14, 2));
        tm.tm_sec  = std::stoi(iso_string.substr(17, 2));
    } catch (...) {
        return 0;
    }
#if defined(_WIN32)
    // _mkgmtime converts tm (UTC) to time_t
    std::time_t tt = _mkgmtime(&tm);
#else
    // timegm converts tm (UTC) to time_t; if unavailable, fallback roughly via timezone
    std::time_t tt = timegm(&tm);
#endif
    if (tt < 0) return 0;
    return static_cast<uint64_t>(tt) * 1000ULL;
}

int64_t Time::get_time_diff_ms(uint64_t start_time, uint64_t end_time) {
    return static_cast<int64_t>(end_time) - static_cast<int64_t>(start_time);
}

bool Time::is_recent(uint64_t timestamp, uint64_t window_ms) {
    uint64_t now = get_current_time_ms();
    if (timestamp > now) return false; // future timestamp not considered recent
    uint64_t diff = now - timestamp;
    return diff <= window_ms;
}

void Time::sleep_ms(uint64_t duration_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}

void Time::sleep_us(uint64_t duration_us) {
    std::this_thread::sleep_for(std::chrono::microseconds(duration_us));
}

uint64_t Time::get_monotonic_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace util
} // namespace environet
