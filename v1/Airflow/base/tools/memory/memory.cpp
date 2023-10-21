#include "memory.h"

#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <Psapi.h>

create_feature_ptr(memory);

c_address c_memory::get_interface(HMODULE module, const std::string& str)
{
	typedef void* (*create_interface_fn)(const char* name, int ret);
	create_interface_fn create_interface = (create_interface_fn)GetProcAddress(module, xor_c("CreateInterface"));

	if (!create_interface)
		return nullptr;

	return create_interface(str.c_str(), 0);
}

c_address c_memory::find_pattern(IN HMODULE h_mod, IN LPCSTR sign_str, int size)
{
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)h_mod;
	PIMAGE_NT_HEADERS nt_hdr = (PIMAGE_NT_HEADERS)((PBYTE)h_mod + dos_hdr->e_lfanew);

	DWORD size_of_image = size != 1488228 ? size : nt_hdr->OptionalHeader.SizeOfImage;
	PBYTE scan_bytes = (PBYTE)h_mod;

	SHORT sign_bytes[64];
	INT_PTR sign_bytes_last_index = sizeof(sign_bytes) / sizeof(sign_bytes[0]) - 1, sign_bytes_count = -1;
	for (;; ++sign_str)
	{
		CHAR sign_chr = *sign_str;
		if (sign_chr == '\0')
			break;
		else if (sign_chr == '?')
		{
			++sign_str;
			sign_chr = *sign_str;
			if (sign_chr != '?')
				--sign_str;
			if (sign_bytes_count == sign_bytes_last_index)
				return (PBYTE)-1;
			sign_bytes[++sign_bytes_count] = -1;
		}
		else if (sign_chr == ' ')
			continue;
		else
		{
			SHORT sign_byte = 0;
			if ((sign_chr >= '0') && (sign_chr <= '9'))
				sign_byte = (sign_chr - '0');
			else if ((sign_chr >= 'A') && (sign_chr <= 'F'))
				sign_byte = (10 + sign_chr - 'A');
			else if ((sign_chr >= 'a') && (sign_chr <= 'f'))
				sign_byte = (10 + sign_chr - 'a');
			else
				goto set_sign_byte;

			++sign_str;
			sign_chr = *sign_str;
			if ((sign_chr >= '0') && (sign_chr <= '9'))
				sign_byte = ((sign_byte << 4) | (sign_chr - '0'));
			else if ((sign_chr >= 'A') && (sign_chr <= 'F'))
				sign_byte = ((sign_byte << 4) | (10 + sign_chr - 'A'));
			else if ((sign_chr >= 'a') && (sign_chr <= 'f'))
				sign_byte = ((sign_byte << 4) | (10 + sign_chr - 'a'));
			else
				--sign_str;

		set_sign_byte:
			if (sign_bytes_count == sign_bytes_last_index)
				return (PBYTE)-1;
			sign_bytes[++sign_bytes_count] = sign_byte;
		}
	}
	++sign_bytes_count;

	for (INT_PTR i = 0, end = (size_of_image - sign_bytes_count); i < end; ++i)
	{
		for (INT_PTR j = 0; j < sign_bytes_count; ++j)
		{
			SHORT sign_byte = sign_bytes[j];
			if ((scan_bytes[i + j] != sign_byte) && (sign_byte != -1))
				goto _continue;
		}

		return &scan_bytes[i];

	_continue:;
	}

	return nullptr;
}