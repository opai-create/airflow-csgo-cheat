#pragma once
#define GET_XOR_KEYUI32 ( ( HASH( __FILE__ __TIMESTAMP__ ) ) % UINT32_MAX )

#ifndef _DEBUG
#define CREATE_DUMMY_PTR(member) inline member temp_##member{};

#define DECLARE_XORED_PTR(member, current_key) \
class xored_##member {\
private:\
  member* xored_pointer{};\
public: \
  INLINE xored_##member(member* ptr) { \
    xored_pointer = (member*)((((std::uintptr_t)ptr) ^ (current_key)) + seeds::const_xs32_from_seed(current_key)); \
  } \
  INLINE member* get() { \
    return (member*)((((std::uintptr_t)xored_pointer) - seeds::const_xs32_from_seed(current_key)) ^ (current_key)); \
  } \
};  \
inline xored_##member xored_ptr_##member(&temp_##member); \

#define XORED_PTR(member) xored_ptr_##member.get()
#endif