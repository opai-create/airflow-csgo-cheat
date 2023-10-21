#pragma once
#include "../../../includes.h"
#include "../address.h"
#include "../utils_macro.h"

class c_memory
{
public:
	template < typename T = void* >
	__forceinline T getvfunc(const void* ptr, const unsigned int idx)
	{
		return (T)((*(void***)ptr)[idx]);
	}

	c_address get_interface(HMODULE module, const std::string& str);
	c_address find_pattern(IN HMODULE h_mod, IN LPCSTR sign_str, int size = 1488228);
};

declare_feature_ptr(memory);