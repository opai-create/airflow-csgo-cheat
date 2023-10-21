#pragma once

#define IS_IN_RANGE(value, max, min) (value >= max && value <= min)
#define GET_BITS(value) (IS_IN_RANGE(value, '0', '9') ? (value - '0') : ((value & (~0x20)) - 'A' + 0xA ))
#define GET_BYTE(value) (GET_BITS(value[0]) << 4 | GET_BITS(value[1]))

#define CONCAT_IMPL( x, y ) x##y
#define MARCO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define PAD( size ) std::byte MARCO_CONCAT( _pad, __COUNTER__ ) [ size ]

#define VFUNC(name, type, index, ...) \
	inline auto name { \
		using fn_t = type; \
		return memory::get_virtual(memory::address_t(this), XORN(index)).cast<fn_t>()(this, __VA_ARGS__); \
	}

namespace memory
{
	struct address_t
	{
		std::uintptr_t pointer;

		INLINE address_t() : pointer(0) {}
		INLINE address_t(const std::uintptr_t& ptr) : pointer(ptr) {}
		INLINE address_t(const void* ptr) : pointer((std::uintptr_t)ptr) {}

		template <typename T = address_t>
		INLINE T cast()
		{
			if (pointer == 0)
				return {};

			return (T)pointer;
		}

		template <typename T = address_t>
		INLINE T add(const std::size_t& amt)
		{
			if (pointer == 0)
				return {};

			return (T)(pointer + amt);
		}

		template <typename T = address_t>
		INLINE T sub(const std::size_t& amt)
		{
			if (pointer == 0)
				return {};

			return (T)(pointer - amt);
		}

		template <typename T = address_t>
		INLINE T deref()
		{
			if (pointer == 0)
				return {};

			return *(T*)pointer;
		}

		template <typename T = address_t>
		INLINE T get(std::size_t n = 1)
		{
			std::uintptr_t out;

			if (!pointer)
				return T{ };

			out = pointer;

			for (std::size_t i = n; i > 0; --i)
			{
				if (!valid(out))
					return T{ };

				out = *(std::uintptr_t*)out;
			}

			return (T)out;
		}

		static INLINE bool valid(std::uintptr_t addr)
		{
			MEMORY_BASIC_INFORMATION mbi;

			if (!addr)
				return false;

			if (!WINCALL(VirtualQuery)((const void*)addr, &mbi, sizeof(mbi)))
				return false;

			if ((mbi.Protect & PAGE_NOACCESS) || (mbi.Protect & PAGE_GUARD))
				return false;

			return true;
		}

		template <typename T = address_t>
		INLINE T relative(const std::size_t& amt)
		{
			if (!pointer)
				return {};

			auto out = pointer + amt;
			auto r = *(std::uint32_t*)out;

			if (!r)
				return {};

			out = (out + 4) + r;

			return (T)out;
		}
	};

	struct bits_t
	{
		std::uint32_t bits;

		bits_t() : bits(0) {}
		bits_t(const std::uint32_t& bit) : bits(bit) {}

		INLINE bool has(const std::uint32_t& bit)
		{
			return bits & bit;
		}

		INLINE void force(const std::uint32_t& bit)
		{
			bits |= bit;
		}

		INLINE void remove(const std::uint32_t& bit)
		{
			bits &= ~bit;
		}
	};

	template <typename T>
	class c_restore
	{
	private:
		T* stored;
		T backup;
	public:
		INLINE c_restore(T* orig)
		{
			stored = orig;
			backup = *orig;
		}

		INLINE ~c_restore()
		{
			*stored = std::move(backup);
		}
	};

	template <typename T>
	class c_multi_restore
	{
	private:
		struct values_t
		{
			T* ptr;
			T backup;

			values_t(T* ptr, T backup) : ptr(ptr), backup(backup) {}
		};

		std::vector<values_t> values;
	public:
		INLINE c_multi_restore(std::vector<T*> list)
		{
			for (const auto& i : list)
				values.emplace_back(i, *i);
		}

		INLINE ~c_multi_restore()
		{
			for (auto& i : values)
				*i.ptr = std::move(i.backup);

			values.clear();
		}
	};

	INLINE address_t get_offset(HMODULE module, const address_t& offset)
	{
		return (std::uintptr_t)module + offset.pointer;
	}

	extern address_t get_pattern(HMODULE module, const char* pat);
	extern address_t get_interface(HMODULE module, const char* inter);
	extern address_t get_virtual(address_t ptr, const int& idx);
}

#define RESTORE(original) auto MARCO_CONCAT( backup, __COUNTER__ ) = memory::c_restore{ &original }
#define MULTI_RESTORE(type, ...) auto MARCO_CONCAT(multi_backup, __COUNTER__) = memory::c_multi_restore<type>({__VA_ARGS__})