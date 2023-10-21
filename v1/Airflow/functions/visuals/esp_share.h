#pragma once

#include <vector>

static constexpr uint16_t base_id = 0xDEAD;
static constexpr uint8_t old_airflow_id = 0xAF;
static constexpr uint8_t airflow_id = 0xFA;
static constexpr uint8_t weave_id = 0x99;
static constexpr uint8_t boss_id = 0xBA;
static constexpr uint8_t weave_boss_id = 0xBB;

#ifdef _DEBUG
static constexpr uint32_t unique_id = (base_id << 16) | (boss_id << 8);
#else
static constexpr uint32_t unique_id = (base_id << 16) | (airflow_id << 8);
#endif

enum e_msg_type : uint8_t
{
	// used to share player info between hack users
	// sends every tick and every player we're seeing on ESP
	player_info = 0xF1,
};

static __forceinline uint32_t get_shared_id(uint32_t n)
{
	return ((n & 0xFF00) >> 8);
}

static constexpr __forceinline uint32_t get_id(uint32_t sent_id = unique_id)
{
	return sent_id | ((uint8_t)e_msg_type::player_info);
}

#define id_player_info xor_int(get_id())

struct communication_string_t
{
	char data[16]{};
	uint32_t current_len = 0;
	uint32_t max_len = 15;
};

struct broken_shared_esp_data_t
{
	uint32_t id{};
	uint8_t user_id{};
	int16_t origin_z{};
	int16_t origin_y{};
	int16_t origin_x{};
	int8_t health{};
	int8_t is_fake{};
	int8_t is_boss{};
};

#pragma pack(push, 1)
struct shared_esp_data_t {
	uint32_t id{};
	uint8_t user_id{};
	int16_t origin_x{};
	int16_t origin_y{};
	int16_t origin_z{};
	int8_t health{};
	uint32_t tick{};

	struct user_info_t {
		uint16_t id{};
		uint8_t crash : 1 = 0;
		uint8_t boss : 1 = 0;

		user_info_t() { static_assert(sizeof(*this) <= 5); }
	} user_info;

	shared_esp_data_t() { static_assert(sizeof(*this) <= 19); }
};
#pragma pack(pop)

namespace esp_share
{
	extern void send_data_msg(c_voice_communication_data* data);
	extern void send_shared_esp_data(const std::vector<uint8_t>& voice);
	extern void send_shared_esp_data(broken_shared_esp_data_t data);
	extern void share_player_position(c_csplayer* player);
}