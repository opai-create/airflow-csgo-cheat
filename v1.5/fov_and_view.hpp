#pragma once

class c_fov_and_view
{
private:
	bool override_sensitivity{};
	int last_fov{};

public:
	INLINE void reset()
	{
		override_sensitivity = false;

		last_fov = 0;
	}

	void change_zoom_sensitivity();
	void change_fov();
	void change_fov_dead_and_remove_recoil(c_view_setup* setup);
};

#ifdef _DEBUG
inline auto FOV_AND_VIEW = std::make_unique<c_fov_and_view>();
#else
CREATE_DUMMY_PTR(c_fov_and_view);
DECLARE_XORED_PTR(c_fov_and_view, GET_XOR_KEYUI32);

#define FOV_AND_VIEW XORED_PTR(c_fov_and_view)
#endif