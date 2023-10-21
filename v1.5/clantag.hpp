#pragma once

class c_clantag
{
private:
	bool reset_tag{};

	int last_change_time{};
	float next_update_time{};

	std::string tag_desc = XOR(" airflow ");

	INLINE void set_clan_tag(const char* tag, const char* name)
	{
		static auto fn = offsets::clantag.cast<void(__fastcall*)(const char*, const char*)>();
		fn(tag, name);
	}

public:
	INLINE void reset()
	{
		reset_tag = false;

		last_change_time = 0;
		next_update_time = 0.f;
	}

	void run();
};

#ifdef _DEBUG
inline auto CLANTAG = std::make_unique<c_clantag>();
#else
CREATE_DUMMY_PTR(c_clantag);
DECLARE_XORED_PTR(c_clantag, GET_XOR_KEYUI32);

#define CLANTAG XORED_PTR(c_clantag)
#endif