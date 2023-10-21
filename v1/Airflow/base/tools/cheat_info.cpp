#include "cheat_info.h"
#include "address.h"
#include "memory/memory.h"
#include "syscalls.h"
#include <thread>
#include <string>
#include <sstream>

#define HANDLE_TO_DWORD64(p) ((DWORD64)(LONG_PTR)(p))
#define PTR_TO_DWORD64(p) ((DWORD64)(ULONG_PTR)(p))

#define is_in_range(value, max, min) (value >= max && value <= min)
#define get_bits(value) (is_in_range(value, '0', '9') ? (value - '0') : ((value & (~0x20)) - 'a' + 0xa))
#define get_byte(value) (get_bits(value[0]) << 4 | get_bits(value[1]))

NTSTATUS
NTAPI
niggers(
	IN  DWORD64 BaseAddress,
	IN  DWORD64 NumBytesToProtect,
	IN  ULONG  ProtectionStatus,
	OUT PULONG  OldProtectionStatus
)
{
	DWORD64 Base = BaseAddress;
	DWORD64 NumBytes = NumBytesToProtect;

	return syscall(0x50, -1, &Base, &NumBytes, ProtectionStatus, OldProtectionStatus);
}

using patterns_t = std::vector< c_address >;
using pattern_byte_t = std::pair< uint8_t, bool >;

c_address __get_pattern(c_address start, const std::string& pat, size_t len)
{
	uint8_t* scan_start, * scan_end;
	std::vector< pattern_byte_t > pattern{};
	std::stringstream			  stream{ pat };
	std::string				      w;

	if (!start || !len || pat.empty())
		return{};

	// split spaces and convert to hex.
	while (stream >> w) {
		// wildcard.
		if (w[0] == '?')
			pattern.push_back({ 0, true });

		// valid hex digits.
		else if (std::isxdigit(w[0]) && std::isxdigit(w[1]))
			pattern.push_back({ (uint8_t)std::strtoul(w.data(), 0, 16), false });
	}

	scan_start = start.as< uint8_t* >();
	scan_end = scan_start + len;

	// find match.
	auto result = std::search(scan_start, scan_end, pattern.begin(), pattern.end(),
		[](const uint8_t b, const pattern_byte_t& p) {
			// byte matches or it's a wildcard.
			return b == p.first || p.second;
		});

	// nothing found.
	if (result == scan_end)
		return{};

	return (uintptr_t)result;
}

std::vector<c_address> find_all_patterns(c_address start, size_t len, const std::string& pat)
{
	std::vector<c_address> out{};
	c_address result{};

	for (;;)
	{
		result = __get_pattern(start, pat, len);
		if (!result)
			break;

		// if we arrived here we found something.
		out.push_back(result);

		// set new len.
		len = (start + len) - (result + 1);

		// new start point.
		start = result + 1;
	}

	return out;
}

void c_main_cheat_info::init()
{
	cheat::initialize();

	//auto base_module = this->cheat_module;

	//auto dosHeader = (PIMAGE_DOS_HEADER)base_module;
	//auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_module + dosHeader->e_lfanew);
	//auto scanBytes = reinterpret_cast<std::uint8_t*>(base_module);
	//auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;

	//std::vector<c_address> found_starts = find_all_patterns(base_module, sizeOfImage, xor_str("BE EF AB BA 10 20 FC"));
	//std::vector<c_address> found_ends = find_all_patterns(base_module, sizeOfImage, xor_str("BE EF BA AB 20 10 FC"));

	//// something went wrong..... maybe forgot smth?
	//if (found_starts.size() != found_ends.size())
	//{
	//	*(int*)0x1 = 1;
	//	return;
	//}

	//for (int i = 0; i < found_starts.size(); ++i)
	//{
	//	auto& start_ptr = found_starts[i];
	//	auto& end_ptr = found_ends[i];

	//	auto address_diff = end_ptr - start_ptr;
	//	for (int j = 0; j < address_diff; ++j)
	//	{
	//		auto addr = start_ptr + j;

	//		ULONG old_protect{};
	//		syscall(0x0050, GetCurrentProcess(), (PVOID)addr, sizeof(addr), (ULONG)PAGE_EXECUTE_READWRITE, &old_protect);

	//		*(uint32_t*)addr = 0x90;

	//		syscall(0x0050, GetCurrentProcess(), (PVOID)addr, sizeof(addr), old_protect, &old_protect);
	//	}
	//}
}