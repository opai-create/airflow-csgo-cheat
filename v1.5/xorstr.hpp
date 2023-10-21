#pragma once
#include <memory>
#include <string>

template <std::size_t string_size>
class c_xor_string {
protected:
	bool crypt_once{};
	mutable bool decrypted{};
	mutable char string[string_size]{};
	std::uint64_t xor_hash{};

	static constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol) {
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}

public:
	constexpr c_xor_string(const char(&str)[string_size], std::uint64_t hash_sol, bool crypt_once)
		: decrypted(false), string{ 0 }, xor_hash(hash_sol), crypt_once(crypt_once)
	{
		for (std::size_t i = 0; i < string_size; ++i)
			this->string[i] = static_cast<char>(str[i] ^ c_xor_string<string_size>::hash(i, this->xor_hash));
	}

	operator std::string() const
	{
		if (crypt_once) {
			if (!this->decrypted) {
				this->decrypted = true;
				for (std::size_t i = 0; i < string_size; ++i)
					this->string[i] ^= c_xor_string<string_size>::hash(i, this->xor_hash);
			}
		}
		else {
			for (std::size_t i = 0; i < string_size; ++i)
				this->string[i] ^= c_xor_string<string_size>::hash(i, this->xor_hash);
		}

		return { this->string, this->string + string_size - 1 };
	}
};

template <std::size_t string_size>
class c_xor_wstring {
protected:
	bool crypt_once{};
	mutable bool decrypted{};
	mutable wchar_t string[string_size]{};
	std::uint64_t xor_hash{};

	static constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol) {
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}

public:
	constexpr c_xor_wstring(const wchar_t(&str)[string_size], std::uint64_t hash_sol, bool crypt_once)
		: decrypted(false), string{ 0 }, xor_hash(hash_sol), crypt_once(crypt_once)
	{
		for (std::size_t i = 0; i < string_size; ++i)
			this->string[i] = static_cast<wchar_t>(str[i] ^ c_xor_wstring<string_size>::hash(i, this->xor_hash));
	}

	operator std::wstring() const {
		if (crypt_once) {
			if (!this->decrypted) {
				this->decrypted = true;
				for (std::size_t i = 0; i < string_size; ++i)
					this->string[i] ^= c_xor_wstring<string_size>::hash(i, this->xor_hash);
			}
		}
		else {
			for (std::size_t i = 0; i < string_size; ++i)
				this->string[i] ^= c_xor_wstring<string_size>::hash(i, this->xor_hash);
		}
		return { this->string, this->string + string_size - 1 };
	}
};

#ifdef _DEBUG
#define XOR(s) std::string(s)
#define CXOR(s) XOR(s).c_str()
#define WXOR(s) std::wstring(s)
#else

#define XOR(s)                                                                                                                                                                                                                 \
    (                                                                                                                                                                                                                                  \
    []() -> std::string                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                \
      static constexpr c_xor_string str{ s, __COUNTER__, true };                                                                                                                                                                     \
      return str;                                                                                                                                                                                                                    \
    })()

#define CXOR(s) (XOR(s)).c_str()

#define WXOR(s)                                                                                                                                                                                                                \
    (                                                                                                                                                                                                                                  \
    []() -> std::wstring                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                \
      static constexpr c_xor_wstring str{ s, __COUNTER__, true };                                                                                                                                                                    \
      return str;                                                                                                                                                                                                                    \
    })()

#define CWXOR(s) (xor_wstr(s)).c_str()
#endif