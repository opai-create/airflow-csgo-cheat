#include "globals.hpp"

#define IS_IN_RANGE(value, max, min) (value >= max && value <= min)
#define GET_BITS(value) (IS_IN_RANGE(value, '0', '9') ? (value - '0') : ((value & (~0x20)) - 'A' + 0xA))
#define GET_BYTE(value) (GET_BITS(value[0]) << 4 | GET_BITS(value[1]))

using patterns_t = std::vector<memory::address_t>;
using pattern_byte_t = std::pair<std::uint8_t, bool>;

namespace memory
{
	INLINE address_t get_pattern(HMODULE module, const char* pat)
	{
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(pat);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return &scanBytes[i];
            }
        }

        return {};
	}

	inline address_t get_interface(HMODULE module, const char* inter)
	{
        address_t fn = WINCALL(GetProcAddress)(module, CXOR("CreateInterface"));
        if (fn.pointer == NULL)
            return 0u;

        return fn.cast<void* (*)(const char*, int)>()(inter, 0);
	}

	inline address_t get_virtual(address_t ptr, const int& idx)
	{
		return ((*ptr.cast<void***>())[idx]);
	}
}