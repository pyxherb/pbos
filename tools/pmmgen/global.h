#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <unordered_map>
#include <memory>

#define SUBSET_CONST 0x01

struct Subset final {
	std::uint8_t flags;
	std::unordered_map<unsigned int, std::string> entries;

	inline Subset(std::uint8_t flags) {
		this->flags = flags;
	}
};

#endif
