#pragma once

class c_thirdperson
{
private:
	bool enabled{};

public:
	INLINE void reset()
	{
		enabled = false;
	}

	void run_alive();
	void run_dead();
	void fix_camera_on_fakeduck(c_view_setup* setup);
};

#ifdef _DEBUG
inline auto THIRDPSERON = std::make_unique<c_thirdperson>();
#else
CREATE_DUMMY_PTR(c_thirdperson);
DECLARE_XORED_PTR(c_thirdperson, GET_XOR_KEYUI32);

#define THIRDPSERON XORED_PTR(c_thirdperson)
#endif