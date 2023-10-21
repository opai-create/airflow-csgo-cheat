#pragma once

class c_glow
{
public:
	void run();
};

#ifdef _DEBUG
inline auto GLOW = std::make_unique<c_glow>();
#else
CREATE_DUMMY_PTR(c_glow);
DECLARE_XORED_PTR(c_glow, GET_XOR_KEYUI32);

#define GLOW XORED_PTR(c_glow)
#endif