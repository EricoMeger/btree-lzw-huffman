#pragma once

#include "../hash_function.hpp"
#include <cstddef>
#include <cstdint>

class MurmurHash : public HashFunction {
public:
	explicit MurmurHash(uint32_t seed = 0);

	uint64_t hash(const std::string& input) override;

	std::string getName() const override {
		return "MurmurHash";
	}

private:
	uint32_t seed;
};

uint32_t murmur3_32(const void* key, size_t len, uint32_t seed);
