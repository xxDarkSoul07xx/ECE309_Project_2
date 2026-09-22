#include "core/sentinel_scanner.h"

// init wtih sentinel we looking for
SentinelScanner::SentinelScanner(std::string s) : sentinel(std::move(s)), pending("") {}

SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    // combine what we were holding with the new chunck
    std::string combined = pending + std::string(chunk);

    // check is the sentinel in there
    std::size_t found = combined.find(sentinel);
    if (found != std::string::npos) {
        return { combined.substr(0, found), true };
    }

    // hold size-1 chars incase they are the start of sentinel
    std::size_t holdBack = sentinel.size() - 1;
    if (combined.size() <= holdBack) {
        pending = combined;
        return { "", false };
    }
// everything before holdBack safe
    std::size_t safeEnd = combined.size() - holdBack;
    std::string safeText = combined.substr(0, safeEnd);
    pending = combined.substr(safeEnd);
    return { safeText, false };
}

//dump rest out at the end
SentinelScanner::Out SentinelScanner::flush() {
    std::string leftover = pending;
    pending = "";
    return { leftover, false };
}