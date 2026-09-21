#include "core/conversation.h"
#include <stdexcept>

// constructor
Conversation::Conversation() : data(nullptr), sz(0), cap(0) {}

// destructor
Conversation::~Conversation() {
    delete[] data;
}

// copy constructor
Conversation::Conversation(const Conversation& other) : data(nullptr), sz(other.sz), cap(other.cap) {
    if (other.cap > 0) {
        data = new Message[other.cap];
        for (std::size_t i = 0; i < other.sz; i++) {
            data[i] = other.data[i];
        }
    }
}

// copy assignment
Conversation& Conversation::operator=(const Conversation& other) {
    if (this == &other) return *this;
    delete[] data;
    data = nullptr;
    sz = other.sz;
    cap = other.cap;
    if (other.cap > 0) {
        data = new Message[other.cap];
        for (std::size_t i = 0; i < other.sz; i++) {
            data[i] = other.data[i];
        }
    }
    return *this;
}

// move constructor
Conversation::Conversation(Conversation&& other) noexcept : data(other.data), sz(other.sz), cap(other.cap) {
    other.data = nullptr;
    other.sz = 0;
    other.cap = 0;
}

// move assignment
Conversation& Conversation::operator=(Conversation&& other) noexcept {
    if (this == &other) return *this;
    delete[] data;
    data = other.data;
    sz = other.sz;
    cap = other.cap;
    other.data = nullptr;
    other.sz = 0;
    other.cap = 0;
    return *this;
}

// add to the end and if we ran out of space double capacity and then move everything over
void Conversation::append(Message msg) {
    if (sz == cap) {
        std::size_t newCap = (cap == 0) ? 1 : cap * 2;
        Message* newData = new Message[newCap];
        for (std::size_t i = 0; i < sz; i++) {
            newData[i] = std::move(data[i]);
        }
        delete[] data;
        data = newData;
        cap = newCap;
    }
    data[sz] = std::move(msg);
    sz++;
}

// returns how many messages that we have
std::size_t Conversation::size() const noexcept {
    return sz;
}

// checks bounds and returns the message
const Message& Conversation::at(std::size_t i) const {
    if (i >= sz) {
        throw std::out_of_range("index out of range");
    }
    return data[i];
}

// points to first message
const Message* Conversation::begin() const noexcept {
    return data;
}

// pointes to last + 1 spot
const Message* Conversation::end() const noexcept {
    return data + sz;
}