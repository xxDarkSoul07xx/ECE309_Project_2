// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"
 
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

// make sure empty conversation doesn't crash and throws when you access out of bounds
void test_empty_conversation_bounds() {
    Conversation conv;
    assert(conv.size() == 0);
    assert(conv.begin() == conv.end());
    bool threw = false;
    try { conv.at(0); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

// system message should always be first
void test_system_message_ordering() {
    Conversation conv;
    conv.append(Message(Role::System, "system prompt"));
    conv.append(Message(Role::User, "hello"));
    conv.append(Message(Role::Assistant, "hi"));
    assert(conv.at(0).role() == Role::System);
    assert(conv.at(0).content() == "system prompt");
    assert(conv.at(1).role() == Role::User);
    assert(conv.at(2).role() == Role::Assistant);
}

// copy should make a totally separate buffer not just copy the pointer
void test_copy_constructor_deep_copy() {
    Conversation conv1;
    conv1.append(Message(Role::User, "hello"));
    Conversation conv2(conv1);
    assert(conv2.size() == 1);
    assert(conv2.begin() != conv1.begin());
    assert(conv2.at(0).content() == "hello");
}

// move should steal the pointer, not copy
void test_move_constructor_steals_pointer() {
    Conversation conv1;
    conv1.append(Message(Role::User, "hello"));
    const Message* ptr = conv1.begin();
    assert(ptr != nullptr);
    Conversation conv2(std::move(conv1));
    assert(conv2.begin() == ptr);
    assert(conv1.size() == 0);
    assert(conv1.begin() == nullptr);
}

// append 100 messages and make sure they all stayed correct after reallocations
void test_growth_behavior() {
    Conversation conv;
    for (int i = 0; i < 100; i++) {
        conv.append(Message(Role::User, "msg " + std::to_string(i)));
    }
    assert(conv.size() == 100);
    for (int i = 0; i < 100; i++) {
        assert(conv.at(i).content() == "msg " + std::to_string(i));
    }
}

// text with no sentinel should just pass through fine
void test_scanner_clean_text() {
    SentinelScanner scanner("<|end_conversation|>");
    auto result = scanner.feed("Hello there!");
    auto flushed = scanner.flush();
    assert(!result.sentinel_found);
    assert(!flushed.sentinel_found);
    assert(result.safe_text + flushed.safe_text == "Hello there!");
}

// try every possible split point to make sure sentinel always gets caught
void test_scanner_split_sentinel_every_boundary() {
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "Goodbye." + sentinel;
    for (std::size_t split = 0; split <= text.size(); split++) {
        SentinelScanner scanner(sentinel);
        auto out1 = scanner.feed(text.substr(0, split));
        auto out2 = scanner.feed(text.substr(split));
        assert(out1.sentinel_found || out2.sentinel_found);
        assert(out1.safe_text + out2.safe_text == "Goodbye.");
    }
}

// something that looks similar but isn't the sentinel shouldn't trigger it
void test_scanner_no_false_alarm() {
    SentinelScanner scanner("<|end_conversation|>");
    auto result = scanner.feed("<|end_world|>");
    auto flushed = scanner.flush();
    assert(!result.sentinel_found);
    assert(!flushed.sentinel_found);
    assert(result.safe_text + flushed.safe_text == "<|end_world|>");
}

// feed 4mb one byte at a time and make sure pending never gets too big
void test_scanner_bounded_memory() {
    const std::string sentinel = "<|end_conversation|>";
    SentinelScanner scanner(sentinel);
    std::string big(4 * 1024 * 1024, 'a');
    for (std::size_t i = 0; i < big.size(); i++) {
        scanner.feed(std::string_view(big.data() + i, 1));
        assert(scanner.pending_size() <= sentinel.size() - 1);
    }
    auto flushed = scanner.flush();
    assert(!flushed.sentinel_found);
}

// same as copy constructor test but for assignment operator
void test_copy_assignment_deep_copy() {
    Conversation conv1;
    conv1.append(Message(Role::User, "hello"));
    Conversation conv2;
    conv2 = conv1;
    assert(conv2.size() == 1);
    assert(conv2.begin() != conv1.begin());
    assert(conv2.at(0).content() == "hello");
}

// same as move constructor test but for move assignment
void test_move_assignment_steals_pointer() {
    Conversation conv1;
    conv1.append(Message(Role::User, "hello"));
    const Message* ptr = conv1.begin();
    Conversation conv2;
    conv2 = std::move(conv1);
    assert(conv2.begin() == ptr);
    assert(conv1.size() == 0);
}

namespace {

// fake input source so we can feed it scripted lines without real terminal input
class FakeInput : public InputSource {
public:
    explicit FakeInput(std::vector<std::string> lines) : lines(std::move(lines)), idx(0), eof(false) {}
 
    std::string read_line() override {
        if (idx >= lines.size()) { eof = true; return ""; }
        return lines[idx++];
    }
    bool is_eof() const override { return eof; }
 
private:
    std::vector<std::string> lines;
    std::size_t idx;
    bool eof;
};

// fake output that just throws away everything so we don't print to terminal during tests
class FakeOutput : public OutputSink {
public:
    void write(std::string_view) override {}
};

}

// harness should stop with TurnLimit when we hit max turns
void test_harness_turn_limit() {
    std::string path = "/tmp/turn_limit.script";
    {
        std::ofstream f(path);
        f << "role: assistant\nreply one\n---\n";
        f << "role: assistant\nreply two\n---\n";
        f << "role: assistant\nreply three\n";
    }
    HarnessConfig cfg;
    cfg.max_turns = 2;
    auto model = std::make_unique<ScriptedModelClient>(path);
    Harness harness(std::move(model), cfg);
    FakeInput in({"hello", "world"});
    FakeOutput out;
    StopReason reason = harness.run(in, out);
    assert(reason.kind == StopReason::Kind::TurnLimit);
}

// harness should stop with Sentinel when the model sends the stop token
void test_harness_sentinel_halt() {
    std::string path = "/tmp/sentinel_halt.script";
    {
        std::ofstream f(path);
        f << "role: assistant\nGoodbye.<|end_conversation|>\n";
    }
    HarnessConfig cfg;
    auto model = std::make_unique<ScriptedModelClient>(path);
    Harness harness(std::move(model), cfg);
    FakeInput in({"bye"});
    FakeOutput out;
    StopReason reason = harness.run(in, out);
    assert(reason.kind == StopReason::Kind::Sentinel);
}

// save a transcript and replay it, make sure we get the same reply back
void test_transcript_round_trip() {
    std::string path = "/tmp/round_trip.txt";
    {
        std::ofstream f(path);
        f << "role: user\nhello\n---\n";
        f << "role: assistant\nHi there!<|end_conversation|>\n";
    }
    ReplayModelClient replay(path);
    Conversation conv;
    conv.append(Message(Role::User, "hello"));
    Message reply = replay.generate(conv);
    assert(reply.content() == "Hi there!<|end_conversation|>");
    assert(reply.role() == Role::Assistant);
}
 
int main() {
    test_empty_conversation_bounds();
    test_system_message_ordering();
    test_copy_constructor_deep_copy();
    test_move_constructor_steals_pointer();
    test_growth_behavior();
    test_scanner_clean_text();
    test_scanner_split_sentinel_every_boundary();
    test_scanner_no_false_alarm();
    test_scanner_bounded_memory();
    test_copy_assignment_deep_copy();
    test_move_assignment_steals_pointer();
    test_harness_turn_limit();
    test_harness_sentinel_halt();
    test_transcript_round_trip();
    return 0;
}