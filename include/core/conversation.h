#pragma once
#include "core/message.h"
#include <cstddef>

// store the messages in a array that grows bigger
class Conversation {
public:
    // constructor
    Conversation();
    // destructor
    ~Conversation();
    // copy constructor
    Conversation(const Conversation& other);
    // copy assignment
    Conversation& operator=(const Conversation& other);
    // move constructor
    Conversation(Conversation&& other) noexcept;
    // move assignment
    Conversation& operator=(Conversation&& other) noexcept;

    // add a message to the end and double space if ran out
    void append(Message msg);
    // how many messages we have
    std::size_t size() const noexcept;

    // make sure access is in range
    const Message& at(std::size_t i) const;

    // go thru old to new stuff
    const Message* begin() const noexcept;
    const Message* end() const noexcept;

private:
    Message* data; // array of messages
    std::size_t sz; // size of array
    std::size_t cap; // space we allocated
};