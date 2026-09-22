#pragma once
#include <string>
#include <string_view>

// scan a stream of chuncks of text for a sentinel string and handle when the sentinel gets split across chucnks
class SentinelScanner {
public:
    explicit SentinelScanner(std::string sentinel);

    // we can print safe_text, sentinel_found is did we find it or not
    struct Out {
        std::string safe_text;
        bool sentinel_found;
    };

    // feed in the next chunck, return whatever is safe to print and if we find it
    Out feed(std::string_view chunk);
    Out flush();

    // test that pending doesn't get too big
    std::size_t pending_size() const { return pending.size(); }

private:
    std::string sentinel; // what we looking for
    std::string pending; // chars we hold incase the start of the sentinel are them
};