#pragma once

class c_spoofers
{
private:
	bool enabled{};
	std::vector<c_con_command_base*> hidden_cvars{};

public:
	INLINE void reset()
	{
		enabled = false;

		hidden_cvars.clear();
	}

	void unlock_hidden_cvars();
};

#ifdef _DEBUG
inline auto SPOOFERS = std::make_unique<c_spoofers>();
#else
CREATE_DUMMY_PTR(c_spoofers);
DECLARE_XORED_PTR(c_spoofers, GET_XOR_KEYUI32);

#define SPOOFERS XORED_PTR(c_spoofers)
#endif