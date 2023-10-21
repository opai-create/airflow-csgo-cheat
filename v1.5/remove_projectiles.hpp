#pragma once

class c_remove_projectiles
{
private:
	bool override_smoke{};

	i_material* mat1{};
	i_material* mat2{};
	i_material* mat3{};
	i_material* mat4{};

public:
	INLINE void reset()
	{
		override_smoke = false;

		mat1 = nullptr;
		mat2 = nullptr;
		mat3 = nullptr;
		mat4 = nullptr;
	}

	void remove_smoke();
};

#ifdef _DEBUG
inline auto REMOVE_PROJECTILES = std::make_unique<c_remove_projectiles>();
#else
CREATE_DUMMY_PTR(c_remove_projectiles);
DECLARE_XORED_PTR(c_remove_projectiles, GET_XOR_KEYUI32);

#define REMOVE_PROJECTILES XORED_PTR(c_remove_projectiles)
#endif