#pragma once

namespace numbers {
	class c_number_obfuscated {
	private:
		std::uint32_t xor_key{};
		std::uint32_t xor_obfuscated{};

	public:
		constexpr c_number_obfuscated(std::uint32_t num, std::uint32_t key) {
			xor_key = seeds::const_xs32_from_seed(key);
			xor_obfuscated = num ^ xor_key;
		}

		INLINE std::uint32_t get() const {
			return xor_obfuscated ^ xor_key;
		}
	};

	template <std::uint32_t num, std::uint32_t seed>
	std::uint32_t obfuscate() {
		constexpr auto x = c_number_obfuscated(num, seed
			+ __TIME__[0] - '0'
			+ __TIME__[1] - '0'
			+ __TIME__[3] - '0'
			+ __TIME__[4] - '0'
			+ __TIME__[6] - '0'
			+ __TIME__[7] - '0');

		return x.get();
	}
}

#ifdef _DEBUG
#define XORN(n) n
#else
#define XORN(n) numbers::obfuscate<n, __COUNTER__>()
#endif