#pragma once
#include <string>

// for who sent a message
enum class Role { System, User, Assistant };

class Message {
public:
    // make Conversation be able to make empty slots before filling them in
    Message() : r(Role::System), c("") {}
    // constructor
    Message(Role role, std::string content) : r(role), c(std::move(content)) {}
    // getter for role
    Role role() const noexcept { return r; }
    // getter for text
    const std::string& content() const noexcept { return c; }

private:
    Role r;
    std::string c;
};