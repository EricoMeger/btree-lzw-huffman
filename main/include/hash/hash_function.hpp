#pragma once
#include <iostream>
#include <cstdint>
#include <string>

class HashFunction {
public:
    virtual uint64_t hash(const std::string& input) = 0;
    virtual std::string getName() const = 0;
    virtual ~HashFunction() = default;
};
