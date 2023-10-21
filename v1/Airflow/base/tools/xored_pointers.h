#pragma once
#include "../../includes.h"
#include "protect.h"

#define get_xor_keyui32 ( ( HASH( __FILE__ __TIMESTAMP__ ) ) % UINT32_MAX )

namespace seeds
{
    static __forceinline constexpr std::uint32_t const_xs32_from_seed(std::uint32_t seed, int add = 0)
    {
        seed ^= seed << (13 + add);
        seed ^= seed >> (17 + add);
        seed ^= seed << (15 + add);
        return seed;
    }

    static __forceinline std::uint32_t xs32_from_seed(std::uint32_t seed, int add = 0)
    {
        seed ^= seed << (13 + add);
        seed ^= seed >> (17 + add);
        seed ^= seed << (15 + add);
        return seed;
    }
}

template<typename T>
class c_xored_pointer
{
private:
    T* ptr;
    T* xored_addr;
    std::uintptr_t key, temp_key;
    int add;
public:
    __forceinline constexpr c_xored_pointer(T* new_ptr, std::uintptr_t new_key, int add) : ptr(new_ptr), add(add)
    {
        key = new_key;
        temp_key = seeds::const_xs32_from_seed(key, add);

        xored_addr = (T*)(((std::uintptr_t)ptr ^ new_key) + seeds::const_xs32_from_seed(key, add));
    }

    __forceinline constexpr T* get()
    {
        return (T*)((((std::uintptr_t)xored_addr) - temp_key) ^ key);
    }
};

#define create_xored_ptr(member) \
    constexpr auto new_key_##member = get_xor_keyui32; \
    inline member temp_##member; \
    inline c_xored_pointer<member> xored_##member(&temp_##member, new_key_##member, __COUNTER__);

#define xored_ptr(member) xored_##member.get()