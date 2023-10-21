#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <mutex>

class ImDrawList;
struct ImVec2;
class c_color;

namespace mutexes
{
	inline std::mutex players{};
	inline std::mutex weapons{};
	inline std::mutex local{};
	inline std::mutex bomb{};
	inline std::mutex warning{};
	inline std::mutex spectators{};
}

struct spectator_t
{
	bool spectated{};

	std::string name{};
	std::string chase_mode{};

	void* ptr{};

	inline void reset() {
		spectated = false;
		name = "";
		chase_mode = "";
		ptr = nullptr;
	}
};

struct spectator_animation_t
{
	bool was_spectating{};
	float start_time{};
	float anim_step{};

	std::string name{};

	inline void reset() {
		was_spectating = false;
		start_time = 0.f;
		anim_step = 0.f;
		name = "";
	}
};

struct key_binds_t
{
	int key = -1;
	int type = -1;

	bool toggled{};

	std::string name{};

	inline void reset() {
		key = -1;
		type = -1;

		toggled = false;
		name = "";
	}

	inline void reset2() {
		key = -1;

		toggled = false;
	}
};

extern std::vector< std::string > key_strings;

struct tab_animation_t
{
	float hovered_alpha{};
	float alpha{};

	bool hovered{};
	bool selected{};

	inline void reset() {
		hovered_alpha = 0.f;
		alpha = 0.f;

		hovered = false;
		selected = false;
	}
};

struct item_animation_t
{
	bool active{};

	float hovered_alpha{};
	float alpha{};

	__forceinline void reset()
	{
		active = false;

		hovered_alpha = 0.f;
		alpha = 0.f;
	}
};

struct menu_key_binds_t
{
	std::string name{};

	int type = -1;

	float time{};
	float alpha{};

	__forceinline void reset(float time)
	{
		//this->name = "";

		this->type = -1;

		this->time = time;
	}
};

struct menu_bomb_t
{
	bool filled{};
	bool defusing{};
	bool defused{};

	int time{};
	int health{};

	std::string bomb_site = XOR("A");

	__forceinline void reset()
	{
		if (!filled)
			return;

		time = 0;
		health = 0;

		bomb_site = XOR("A");

		filled = false;
		defusing = false;
		defused = false;
	}
};

class c_float_color
{
private:
	float color_base[4]{};

public:
	__forceinline c_float_color()
	{
		color_base[0] = 1.f;
		color_base[1] = 1.f;
		color_base[2] = 1.f;
		color_base[3] = 1.f;
	}

	__forceinline c_float_color(c_color& clr)
	{
		color_base[0] = clr.r() / 255.f;
		color_base[1] = clr.g() / 255.f;
		color_base[2] = clr.b() / 255.f;
		color_base[3] = clr.a() / 255.f;
	}

	__forceinline c_float_color(int r = 255, int g = 255, int b = 255, int a = 255)
	{
		color_base[0] = r / 255.f;
		color_base[1] = g / 255.f;
		color_base[2] = b / 255.f;
		color_base[3] = a / 255.f;
	}

	__forceinline float* float_base()
	{
		return color_base;
	}

	__forceinline float& operator[](int idx)
	{
		return color_base[idx];
	}

	__forceinline c_color& base()
	{
		c_color out = c_color(color_base[0] * 255.f, color_base[1] * 255.f, color_base[2] * 255.f, color_base[3] * 255.f);
		return out;
	}
};