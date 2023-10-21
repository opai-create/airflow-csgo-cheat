#pragma once

class c_renderable;
class c_base_entity;
class c_collideable;
class c_cs_player;
class i_net_channel_info;

#pragma region CLIENT
class c_recv_table;
class c_recv_prop;

class d_variant
{
public:
	union
	{
		float flt_;
		long int_;
		char* string;
		void* data;
		float vector[3];
		__int64 int64;
	};

	send_prop_type_t type;
};

struct c_recv_proxy_data
{
public:
	const c_recv_prop* recv_prop;
	d_variant value;
	int element_index;
	int object_id;
};

using data_table_recv_var_proxy_fn = void (*)(const c_recv_prop*, void**, void*, int);
using array_length_recv_proxy_fn = void (*)(void*, int, int);
using recv_var_proxy_fn = void (*)(const c_recv_proxy_data*, void*, void*);

class c_recv_prop
{
public:
	char* prop_name;
	send_prop_type_t prop_type;
	int prop_flags;
	int buffer_size;
	int is_inside_of_array;
	const void* extra_data_ptr;
	c_recv_prop* array_prop;
	array_length_recv_proxy_fn array_length_proxy;
	recv_var_proxy_fn proxy_fn;
	data_table_recv_var_proxy_fn data_table_proxy_fn;
	c_recv_table* data_table;
	int offset;
	int element_stride;
	int elements_count;
	const char* parent_array_prop_name;
};

class c_recv_table
{
public:
	c_recv_prop* props;
	int props_count;
	void* decoder_ptr;
	char* table_name;
	bool is_initialized;
	bool is_in_main_list;
};

using create_client_class_fn = void* (*)(int, int);
using create_event_fn = void* (*)();

class c_client_class
{
public:
	create_client_class_fn create_fn;
	create_event_fn create_event_fn;
	char* network_name;
	c_recv_table* recvtable_ptr;
	c_client_class* next_ptr;
	int class_id;
};

using inputfunc_fn = void(__cdecl*)(void* data);

class datamap_t;

class typedescription_t
{
public:
	fieldtype_t type;
	const char* name;
	int offset[TD_OFFSET_COUNT];
	unsigned short size;
	short flags;
	const char* ext_name;
	void* save_restore_ops;
	inputfunc_fn input_func;
	datamap_t* td;
	int bytes;
	typedescription_t* override_field;
	int override_count;
	float tolerance;

private:
	PAD(0x8);
};

class datamap_t
{
public:
	typedescription_t* desc;
	int size;
	char const* name;
	datamap_t* base;
	bool chains_validated;
	bool packed_offsets_computed;
	int packed_size;
};
#pragma endregion

#pragma region ENGINE
class c_voice_communication_data
{
public:
	std::uint32_t xuid_low;
	std::uint32_t xuid_high;
	std::int32_t sequence_bytes;
	std::uint32_t section_number;
	std::uint32_t uncompressed_sample_offset;

	INLINE std::uint8_t* raw_data()
	{
		return (std::uint8_t*)this;
	}
};

class c_svc_msg_voice_data
{
public:
	PAD(0x8);
	std::int32_t client;
	std::int32_t audible_mask;
	std::uint32_t xuid_low;
	std::uint32_t xuid_high;
	std::string* voice_data;
	bool proximity;
	bool caster;
	std::int32_t format;
	std::int32_t sequence_bytes;
	std::uint32_t section_number;
	std::uint32_t uncompressed_sample_offset;

	INLINE c_voice_communication_data GetData() const
	{
		c_voice_communication_data data{ };
		data.xuid_low = xuid_low;
		data.xuid_high = xuid_high;
		data.sequence_bytes = sequence_bytes;
		data.section_number = section_number;
		data.uncompressed_sample_offset = uncompressed_sample_offset;
		return data;
	}
};

class c_clc_msg_voice_data
{
public:
	std::uint32_t vtable;
	PAD(4);
	std::uint32_t clc_msg_voice_data_vtable;
	PAD(8);
	std::uintptr_t data;
	std::uint32_t xuid_low;
	std::uint32_t xuid_high;
	std::int32_t format;
	std::int32_t sequence_bytes;
	std::uint32_t section_number;
	std::uint32_t uncompressed_sample_offset;
	std::int32_t cached_size;
	std::uint32_t flags;
	PAD(0xFF);

	INLINE void set_data(c_voice_communication_data* data)
	{
		xuid_low = data->xuid_low;
		xuid_high = data->xuid_high;
		sequence_bytes = data->sequence_bytes;
		section_number = data->section_number;
		uncompressed_sample_offset = data->uncompressed_sample_offset;
	}
};

struct renderable_info_t
{
	c_renderable* renderable;
	void* alpha_roperty;
	std::int32_t enum_count;
	std::int32_t render_frame;
	std::uint16_t first_shadow;
	std::uint16_t leaf_list;
	std::int16_t area;
	std::uint16_t flags;
	std::uint16_t render_in_fast_reflection : 1;
	std::uint16_t disable_shadow_depth_rendering : 1;
	std::uint16_t disable_csm_rendering : 1;
	std::uint16_t disable_shadow_depth_caching : 1;
	std::uint16_t splitscreen_enabled : 2;
	std::uint16_t translucency_type : 2;
	std::uint16_t model_type : 8;
	vec3_t bloated_abs_mins;
	vec3_t bloated_abs_maxs;
	vec3_t abs_mins;
	vec3_t abs_maxs;
};

class c_net_message
{
public:
	virtual ~c_net_message() { };

	virtual void set_net_channel(void* netchan) = 0; // netchannel this message is from/for
	virtual void set_reliable(bool state) = 0;       // set to true if it's a reliable message

	virtual bool process(void) = 0; // calles the recently set handler to process this message

	virtual bool read_from_buffer(void* buffer) = 0; // returns true if parsing was OK
	virtual bool write_to_buffer(void* buffer) = 0; // returns true if writing was OK

	virtual bool is_reliable(void) const = 0; // true, if message needs reliable handling

	virtual int get_type(void) const = 0;         // returns module specific header tag eg svc_serverinfo
	virtual int get_group(void) const = 0;        // returns net message group of this message
	virtual const char* get_name(void) const = 0; // returns network message name, eg "svc_serverinfo"
	virtual i_net_channel_info* get_net_channel(void) const = 0;
	virtual const char* to_string(void) const = 0; // returns a human readable string about message content
};

class c_net_channel
{
public:
	PAD(0x18);

	int out_sequence_nr;
	int in_sequence_nr;
	int out_sequence_nr_ack;

	int out_reliable_state;
	int in_reliable_state;

	int choked_packets;

	VFUNC(send_datagram(), int(__thiscall*)(decltype(this), void*), SEND_DATAGRAM_VFUNC, 0);
	VFUNC(send_net_msg(const std::uintptr_t msg, bool reliable = false, bool voice = false),
		bool(__thiscall*)(decltype(this), const std::uintptr_t, bool, bool), 40,
		msg, reliable, voice);
};

class i_net_channel_info
{
public:
	enum
	{
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		TEMPENTS,		// temp entities
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		TOTAL			// must be last and is not a real group
	};

	virtual const char* get_name() const = 0;	// get channel name
	virtual const char* get_address() const = 0; // get channel IP address as string
	virtual float get_time() const = 0; // current net time
	virtual float get_time_connected() const = 0; // get connection time in seconds
	virtual int get_buffer_size() const = 0; // netchannel packet history size
	virtual int get_data_rate() const = 0; // send data rate in byte/sec

	virtual bool is_loopback() const = 0; // true if loopback channel
	virtual bool is_timing_out() const = 0; // true if timing out
	virtual bool is_playback() const = 0; // true if demo playback

	virtual float get_latency(int flow) const = 0; // current latency (RTT), more accurate but jittering
	virtual float get_avg_latency(int flow) const = 0; // average packet latency in seconds
	virtual float get_avg_loss(int flow) const = 0; // avg packet loss[0..1]
	virtual float get_avg_choke(int flow) const = 0; // avg packet choke[0..1]
	virtual float get_avg_data(int flow) const = 0; // data flow in bytes/sec
	virtual float get_avg_packets(int flow) const = 0; // avg packets/sec
	virtual int get_total_data(int flow) const = 0; // total flow in/out in bytes
	virtual int get_total_packets(int flow) const = 0;
	virtual int get_sequence_nr(int flow) const = 0; // last send seq number
	virtual bool is_valid_packet(int flow, int frame_number) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float get_packet_time(int flow, int frame_number) const = 0; // time when packet was send
	virtual int get_packet_bytes(int flow, int frame_number, int group) const = 0; // group size of this packet
	virtual bool get_stream_progress(int flow, int* recieved, int* total) const = 0; // TCP progress if transmitting
	virtual float get_time_since_last_received() const = 0; // get time since last recieved packet in seconds
	virtual float get_command_interpolation_amount(int flow, int frame_number) const = 0;
	virtual void get_packet_response_latency(int flow, int frame_number, int* latency_msecs, int* choke) const = 0;
	virtual void get_remote_framerate(float* frametime, float* frametime_std_deviation) const = 0;
	virtual float get_timeout_seconds() const = 0;
};

class c_event_info
{
public:
	short class_id;
	PAD(2);
	float delay;
	PAD(48);
	c_event_info* next;
	PAD(4);
};

struct player_info_t
{
	int64_t unknown;
	union
	{
		int64_t steaid64;
		struct
		{
			int32_t xuid_low;
			int32_t xuid_high;
		};
	};

	char name[128];
	int user_id;
	char sz_steaid[20];
	PAD(0x10);
	unsigned long steaid;
	char friends_name[128];
	bool fakeplayer;
	bool ishltv;
	unsigned int customFiles[4];
	unsigned char files_downloaded;
};
#pragma endregion

#pragma region STUDIOHDR
struct mstudioiklink_t {
	int bone;
	vec3_t knee_dir;
	vec3_t unused0;

	mstudioiklink_t() {}
private:
	mstudioiklink_t(const mstudioiklink_t& vOther);
};

struct mstudioikchain_t {
	int sznameindex;
	inline char* const name(void) const { return ((char*)this) + sznameindex; }
	int linktype;
	int numlinks;
	int linkindex;
	inline mstudioiklink_t* link(int i) const { return (mstudioiklink_t*)(((byte*)this) + linkindex) + i; };
};

struct model_t {
	void* handle;
	char name[MAX_PATH];

	int load_flags;
	int server_count;
	int type;
	int flags;

	vec3_t mins, maxs;
	float radius;
	void* key_values;

	union {
		void* brush;
		unsigned short studio;
		void* sprite;
	};
};

struct mstudiobbox_t {
	int	bone;
	int	group;
	vec3_t min;
	vec3_t max;
	int name_id;
	vec3_t rotation;
	float radius;
	PAD(0x10);
};

struct mstudiohitboxset_t {
	int name_index;
	int num_hitboxes;
	int hitbox_index;

	INLINE mstudiobbox_t* hitbox(int i) const {
		return (mstudiobbox_t*)(((BYTE*)this) + hitbox_index) + i;
	}
};

struct mstudiobone_t {
	int name_index{};

	inline char* const get_name(void) const
	{
		return ((char*)this) + name_index;
	}

	int parent{};
	int bone_controller[6]{};

	vec3_t pos{};
	quaternion_t quat{};

	vec3_t rot{};

	vec3_t pos_scale{};
	vec3_t rot_scale{};

	matrix3x4_t pose_to_bone{};

	quaternion_t alignment{};

	int flags{ };

	int proc_type{};
	int proc_index{};

	mutable int physics_bone{ };

	inline void* get_procedure() const
	{
		if (proc_index == 0)
			return NULL;
		else
			return (void*)(((unsigned char*)this) + proc_index);
	};

	int surface_prop_iindex{};

	inline char* const get_surface_prop_name(void) const
	{
		return ((char*)this) + surface_prop_iindex;
	}

	inline int get_surface_prop(void) const
	{
		return surface_prop_lookup;
	}

	int contents{};
	int surface_prop_lookup{};
	int unused[7]{};
};

struct mstudiobonecontroller_t {
	int bone;
	int type;
	float start;
	float end;
	int rest;
	int input_field;
	int unused[8];
};

struct mstudioattachment_t {
	int name_index;
	uint32_t flags;
	int local_bone;
	matrix3x4_t local;
	int unused[8];

	INLINE char* const get_name() const {
		return ((char*)this) + name_index;
	}
};

struct mstudioanimtag_t {
	int tag;
	float cycle;
	int sztagindex;

	INLINE char* const tag_name(void) const {
		return ((char*)this) + sztagindex;
	}
};

struct mstudioposeparamdesc_t {
	int name_index;

	INLINE char* const get_name(void) const {
		return ((char*)this) + name_index;
	}

	int flags;
	float start;
	float end;
	float loop;
};

struct mstudioseqdesc_t {
	int baseptr;
	int szlabelindex;
	int szactivitynameindex;
	int flags;
	int activity;
	int actweight;

	int numevents;
	int eventindex;

	vec3_t bbmin;
	vec3_t bbmax;

	int numblends;
	int animindexindex;

	int movementindex;
	int groupsize[2];
	int paramindex[2];
	float paramstart[2];
	float paramend[2];
	int paramparent;

	float fadeintime;
	float fadeouttime;

	int localentrynode;
	int localexitnode;
	int nodeflags;

	float entryphase;
	float exitphase;

	float lastframe;

	int nextseq;
	int pose;

	int numikrules;

	int numautolayers;
	int autolayerindex;

	int weightlistindex;

	int posekeyindex;

	int numiklocks;
	int iklockindex;

	int keyvalueindex;
	int keyvaluesize;

	int cycleposeindex;

	int activitymodifierindex;
	int numactivitymodifiers;

	int animtag_index;
	int num_animtags;

	INLINE mstudioanimtag_t* anim_tag(int i) const {
		return (mstudioanimtag_t*)(((byte*)this) + animtag_index) + i;
	};

	int unused[3];

	mstudioseqdesc_t() {}
};

struct studiohdr_t {
	int id{};
	int version{};

	long checksum{};

	char name[64]{};

	int length{};

	vec3_t eye_pos{};
	vec3_t ilum_pos{};

	vec3_t hull_min{};
	vec3_t hull_max{};

	vec3_t bb_min{};
	vec3_t bb_max{};

	int flags{};

	int bones{};
	int bone_index{};

	int bone_controllers{};
	int bone_controller_index{};

	int hitbox_sets{};
	int hitbox_set_index{};

	int local_anim{};
	int local_anim_index{};

	int local_seq{};
	int local_seq_index{};

	int activity_list_version{};
	int events_indexed{};

	int textures{};
	int texture_index{};

	int numcdtextures;
	int cdtextureindex;

	int numskinref{};
	int numskinfamilies{};
	int skinindex{};

	int numbodyparts{};
	int bodypartindex{};

	int numlocalattachments{};
	int localattachmentindex{};

	int numlocalnodes{};
	int localnodeindex{};
	int localnodenameindex{};

	int numflexdesc{};
	int flexdescindex{};

	int numflexcontrollers{};
	int flexcontrollerindex{};

	int numflexrules{};
	int flexruleindex{};

	int numikchains{};
	int ikchainindex{};

	int nummouths{};
	int mouthindex{};

	int numlocalposeparameters{};
	int localposeparamindex{};

	const char* get_name(void) const {
		return name;
	}

	mstudiohitboxset_t* hitbox_set(int i) {
		if (i > hitbox_sets)
			return nullptr;

		return (mstudiohitboxset_t*)((std::uint8_t*)this + hitbox_set_index) + i;
	}

	mstudioseqdesc_t* seq_desc(int i) {
		if (i > local_seq)
			return nullptr;

		return (mstudioseqdesc_t*)((std::uint8_t*)this + local_seq_index + i);
	}

	mstudiobone_t* bone(int i) {
		if (i > bones)
			return nullptr;

		return (mstudiobone_t*)((std::uint8_t*)this + bone_index) + i;
	}

	inline mstudioikchain_t* ik_chain(int i) const {
		return (mstudioikchain_t*)(((std::uint8_t*)this) + ikchainindex) + i;
	};
};

struct studiohwdata_t {
	int root_lod;
	int num_lods;
	void* lods;
	int num_stidio_meshes;

	INLINE float lod_metric(float unitSphereSize) const { return (unitSphereSize != 0.0f) ? (100.0f / unitSphereSize) : 0.0f; }
	INLINE int get_lod_for_metric(float lodMetric) const {
		return 0;
	}
};
#pragma endregion

#pragma region MATERIALS
class c_key_values
{
public:
	PAD(40);
};

class c_material_var
{
public:
	VFUNC(set_vec_value(float value), void(__thiscall*)(decltype(this), float), 4, value);
	VFUNC(set_vec_value(int value), void(__thiscall*)(decltype(this), int), 5, value);
	VFUNC(set_vec_value(const char* value), void(__thiscall*)(decltype(this), const char*), 6, value);
	VFUNC(set_vec_value(vec3_t vec), void(__thiscall*)(decltype(this), float, float, float), 11, vec.x, vec.y, vec.z);
};

class i_material
{
public:
	virtual const char* get_name() const = 0;
	virtual const char* get_texture_group_name() const = 0;

	virtual int get_preview_image_properties(int* width, int* height, void* image_format, bool* translucent) const = 0;
	virtual int get_preview_image(unsigned char* data, int width, int height, void* image_format) const = 0;

	virtual int get_mapping_width() = 0;
	virtual int get_mapping_height() = 0;

	virtual int get_num_animation_frames() = 0;

	virtual bool in_material_page() = 0;

	virtual void get_material_offset(float* offset) = 0;
	virtual void get_material_scale(float* scale) = 0;

	virtual i_material* get_material_page() = 0;
	virtual c_material_var* find_var(const char* name, bool* found, bool complain = true) = 0;

	virtual void increment_reference_count() = 0;
	virtual void decrement_reference_count() = 0;

	INLINE void add_ref() { increment_reference_count(); }
	INLINE void release() { decrement_reference_count(); }

	virtual int get_enum_id() const = 0;

	virtual void get_low_res_clr_sample(float s, float t, float* color) const = 0;

	virtual void recompute_state_snapshots() = 0;

	virtual bool is_translucent() = 0;
	virtual bool is_alpha_tested() = 0;
	virtual bool is_vertex_lit() = 0;

	virtual int get_vertex_format() const = 0;

	virtual bool has_proxy() const = 0;
	virtual bool use_cube_map() = 0;

	virtual bool needs_tangent_space() = 0;
	virtual bool needs_power_of_two_frame_buffer_texture(bool check_specific_to_this_frame = true) = 0;
	virtual bool needs_full_frame_buffer_texture(bool check_specific_to_this_frame = true) = 0;
	virtual bool needs_software_skinning() = 0;

	virtual void alpha_modulate(float alpha) = 0;
	virtual void color_modulate(float r, float g, float b) = 0;

	virtual void set_material_var_flag(material_flags_t flag, bool on) = 0;
	virtual bool get_material_var_flag(material_flags_t flag) const = 0;

	virtual void get_reflectivity(vec3_t& reflect) = 0;

	virtual bool get_property_flag(int type) = 0;

	virtual bool is_two_sided() = 0;

	virtual void set_shader(const char* shader_name) = 0;

	virtual int get_num_passes() = 0;
	virtual int get_texture_memory_bytes() = 0;

	virtual void refresh() = 0;

	virtual bool needs_lightmap_blend_alpha() = 0;
	virtual bool needs_software_lightning() = 0;

	virtual int shader_param_count() const = 0;
	virtual c_material_var** get_shader_params() = 0;

	virtual bool is_error_material() const = 0;
	virtual void unused() = 0;

	virtual float get_alpha_modulation() = 0;
	virtual void get_color_modulation(float* r, float* g, float* b) = 0;
	virtual bool is_translucent_under_modulation(float alpha_modulation = 1.0f) const = 0;

	virtual c_material_var* find_var_fast(char const* name, unsigned int* token) = 0;

	virtual void set_shader_and_params(void* key_values) = 0;
	virtual const char* get_shader_name() const = 0;

	virtual void delete_if_unreferenced() = 0;

	virtual bool is_spite_card() = 0;

	virtual void call_bind_proxy(void* data) = 0;

	virtual void refresh_preversing_material_vars() = 0;

	virtual bool was_reloaded_from_whitelist() = 0;

	virtual bool set_tem_excluded(bool set, int excluded_dimension_limit) = 0;

	virtual int get_reference_count() const = 0;
};
#pragma endregion

#pragma region MODELRENDER
struct draw_model_state_t
{
	studiohdr_t* studio_hdr;
	studiohwdata_t* studio_hdr_data;
	c_renderable* renderable;
	const matrix3x4_t* model_to_world;
	void* decals;
	int draw_flags;
	int lod;
};

struct model_render_info_t
{
	vec3_t origin;
	vec3_t angles;
#if !LEGACY
	PAD(4);
#endif
	c_renderable* renderable;
	const model_t* model;
	const matrix3x4_t* model_to_world;
	const matrix3x4_t* lightning_offset;
	const vec3_t* lightning_origin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	unsigned short instance;

	model_render_info_t()
	{
		model_to_world = NULL;
		lightning_offset = NULL;
		lightning_origin = NULL;
	}
};
#pragma endregion

#pragma region INPUT
class c_user_cmd
{
public:
	INLINE c_user_cmd()
	{
		std::memset(this, 0, sizeof(*this));
	};

	virtual ~c_user_cmd() {};

	INLINE void copy(c_user_cmd* cmd)
	{
		std::memcpy(this, cmd, sizeof(c_user_cmd));
	}

	int command_number;
	int tickcount;
	vec3_t viewangles;
	vec3_t aim_direction;
	float forwardmove;
	float sidemove;
	float upmove;
	memory::bits_t buttons;
	int impulse;
	int weapon_select;
	int weapon_sub_type;
	int random_seed;
	short mouse_dx;
	short mouse_dy;
	bool has_been_predicted;
	vec3_t headangles;
	vec3_t headoffset;

	INLINE CRC32_t get_checksum() const
	{
		CRC32_t crc;
		CRC32_Init(&crc);

		CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
		CRC32_ProcessBuffer(&crc, &tickcount, sizeof(tickcount));
		CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
		CRC32_ProcessBuffer(&crc, &aim_direction, sizeof(aim_direction));
		CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
		CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
		CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
		CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
		CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
		CRC32_ProcessBuffer(&crc, &weapon_select, sizeof(weapon_select));
		CRC32_ProcessBuffer(&crc, &weapon_sub_type, sizeof(weapon_sub_type));
		CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
		CRC32_ProcessBuffer(&crc, &mouse_dx, sizeof(mouse_dx));
		CRC32_ProcessBuffer(&crc, &mouse_dy, sizeof(mouse_dy));
		CRC32_Final(&crc);
		return crc;
	}
};

struct verified_cmd_t
{
	c_user_cmd cmd;
	CRC32_t crc;
};
#pragma endregion

#pragma region PREDICTION
class c_move_data
{
public:
	bool run_funcs;
	bool game_code_moved_player;
	PAD(2);
	vec3_t handle;
	bool unk;
	PAD(3);
	vec3_t view_angles;
	vec3_t old_view_angles;
	int32_t buttons;
	int32_t old_buttons;
	float forwardmove;
	float sidemove;
	float upmove;
	float max_speed;
	float client_max_speed;
	PAD(12);
	vec3_t angles;
	PAD(84);
	vec3_t abs_origin;
};
#pragma endregion

#pragma region CONVARS
using fn_change_callback_fn = void(__cdecl*)(void*, const char*, float);

class c_con_command_base
{
public:
	void* vtable{};
	c_con_command_base* next{};
	bool registered{};
	const char* name{};
	const char* help_string{};
	int flags{};
	c_con_command_base* s_cmd_base{};
	void* accessor{};
};

class c_convar
{
public:
	PAD(4);
	c_convar* next;
	bool registered;
	const char* name;
	const char* help_string;
	int flags;
	PAD(4);
	c_convar* parent;
	const char* default_value;
	char* string;
	int string_length;
	float float_value;
	int int_value;
	bool has_min;
	float min_value;
	bool has_max;
	float m_max_value;
	c_utl_vector<fn_change_callback_fn> fn_change_callbacks;
public:
	VFUNC(get_name(), const char* (__thiscall*)(decltype(this)), 5);

	VFUNC(set_value(const char* value), void(__thiscall*)(decltype(this), const char*), 14, value);
	VFUNC(set_value(float value), void(__thiscall*)(decltype(this), float), 15, value);
	VFUNC(set_value(int value), void(__thiscall*)(decltype(this), int), 16, value);
	VFUNC(set_value(c_color value), void(__thiscall*)(decltype(this), c_color), 16, value);

	INLINE float get_float() const
	{
		std::uint32_t xored = *(uintptr_t*)(&parent->float_value) ^ (uintptr_t)this;
		return *(float*)&xored;
	}

	INLINE int get_int() const
	{
		return parent->int_value ^ (uintptr_t)this;
	}

	INLINE bool get_bool() const
	{
		return !!get_int();
	}

	INLINE const char* get_string() const
	{
		char const* value = parent->string;
		return value ? value : "";
	}
};
#pragma endregion

#pragma region EVENTS
class c_game_event
{
public:
	virtual ~c_game_event();
	virtual const char* get_name() const = 0;

	virtual bool is_reliable() const = 0;
	virtual bool is_local() const = 0;
	virtual bool is_empty(const char* key_name = NULL) const = 0;

	virtual bool get_bool(const char* key_name = NULL, bool default_value = false) const = 0;
	virtual int get_int(const char* key_name = NULL, int default_value = 0) const = 0;
	virtual uint64_t get_uint64(const char* key_name = NULL, uint64_t default_value = 0) const = 0;
	virtual float get_float(const char* key_name = NULL, float default_value = 0.0f) const = 0;
	virtual const char* get_string(const char* key_name = NULL, const char* default_value = "") const = 0;
	virtual const wchar_t* get_wstring(const char* key_name = NULL, const wchar_t* default_value = L"") const = 0;
	virtual const void* get_ptr(const char* key_name = NULL) const = 0;

	virtual void set_bool(const char* key_name, bool value) = 0;
	virtual void set_int(const char* key_name, int value) = 0;
	virtual void set_uint64(const char* key_name, uint64_t value) = 0;
	virtual void set_float(const char* key_name, float value) = 0;
	virtual void set_string(const char* key_name, const char* value) = 0;
	virtual void set_wstring(const char* key_name, const wchar_t* value) = 0;
	virtual void set_ptr(const char* key_name, const void* value) = 0;
};
#pragma endregion

#pragma region ENGINETRACE
struct surface_t
{
	const char* name;
	short surface_props;
	std::uint16_t flags;
};

struct surfacephysicsparams_t
{
	float friction;
	float elasticity;
	float density;
	float thickness;
	float dampening;
};

struct surfaceaudioparams_t
{
	float reflectivity;
	float hardness_factor;
	float roughness_factor;
	float rough_threshold;
	float hard_threshold;
	float hard_velocity_threshold;
	float high_pitch_occlusion;
	float mid_pitch_occlusion;
	float low_pitch_occlusion;
};

struct surfacesoundnames_t
{
	std::uint16_t walk_step_left;
	std::uint16_t walk_step_right;
	std::uint16_t run_step_left;
	std::uint16_t run_step_right;
	std::uint16_t impact_soft;
	std::uint16_t impact_hard;
	std::uint16_t scrape_smooth;
	std::uint16_t scrape_rough;
	std::uint16_t bullet_impact;
	std::uint16_t rolling;
	std::uint16_t break_sound;
	std::uint16_t strain_sound;
};

struct surfacegameprops_t
{
	float max_speed_factor;
	float jump_factor;
	float penetration_modifier;
	float damage_modifier;
	std::uint16_t material;
	std::uint8_t climbable;

	PAD(0x4);
};

struct surfacesoundhandles_t
{
	std::uint16_t walk_step_left;
	std::uint16_t walk_step_right;
	std::uint16_t run_step_left;
	std::uint16_t run_step_right;
	std::uint16_t impact_soft;
	std::uint16_t impact_hard;
	std::uint16_t scrape_smooth;
	std::uint16_t scrape_rough;
	std::uint16_t bullet_impact;
	std::uint16_t rolling;
	std::uint16_t break_sound;
	std::uint16_t strain_sound;
};

struct surface_data_t
{
	surfacephysicsparams_t physics;
	surfaceaudioparams_t audio;
	surfacesoundnames_t sounds;
	surfacegameprops_t game;
	surfacesoundhandles_t soundhandles;
};

class i_entity_enumerator
{
public:
	virtual bool enum_entity(c_base_entity* handle) = 0;
};

struct brush_side_info_t
{
	vec4_t plane;
	std::uint16_t bevel;
	std::uint16_t thin;
};

class c_phys_collide;

struct cplane_t
{
	vec3_t normal;
	float dist;
	std::uint8_t type;
	std::uint8_t signbits;
	std::uint8_t pad[2];
};

struct vcollide_t
{
	std::uint16_t solid_count : 15;
	std::uint16_t is_packed : 1;
	std::uint16_t desc_size;
	c_phys_collide** solids;
	char* key_values;
	void* user_data;
};

struct cmodel_t
{
	vec3_t mins, maxs;
	vec3_t origin;
	std::int32_t headnode;
	vcollide_t vcollision_data;
};

struct csurface_t
{
	const char* name;
	std::int16_t surface_props;
	std::uint16_t flags;
};

struct ray_t
{
	vec3_aligned_t start;
	vec3_aligned_t delta;
	vec3_aligned_t start_offset;
	vec3_aligned_t extents;
	const matrix3x4_t* world_axis_transform;
	bool is_ray;
	bool is_swept;

	ray_t() : world_axis_transform(NULL) {}

	ray_t(vec3_t const& start, vec3_t const& end)
	{
		delta = end - start;

		is_swept = (delta.length_sqr() != 0);

		extents.reset();

		world_axis_transform = NULL;
		is_ray = true;

		extents.reset();
		this->start = start;
	}

	ray_t(vec3_t const& start, vec3_t const& end, vec3_t const& mins, vec3_t const& maxs)
	{
		delta = { end - start };
		world_axis_transform = nullptr;
		is_swept = delta.length_sqr() != 0.f;
		extents = { maxs - mins };
		extents *= 0.5f;
		is_ray = extents.length_sqr() < 1e-6;
		start_offset = { mins + maxs };
		start_offset *= 0.5f;
		this->start = { start + start_offset };
		start_offset *= -1.f;
	}

	void init(vec3_t const& start, vec3_t const& end)
	{
		delta = end - start;

		is_swept = (delta.length_sqr() != 0);

		extents.reset();

		world_axis_transform = NULL;
		is_ray = true;

		start_offset.reset();
		this->start = start;
	}

	void init(vec3_t const& start, vec3_t const& end, vec3_t const& mins, vec3_t const& maxs)
	{
		delta = end - start;

		world_axis_transform = NULL;
		is_swept = (delta.length_sqr() != 0);

		extents = maxs - mins;
		extents *= 0.5f;
		is_ray = (extents.length_sqr() < 1e-6);

		start_offset = maxs + mins;
		start_offset *= 0.5f;
		this->start = start + start_offset;
		start_offset *= -1.0f;
	}

	vec3_t inv_delta() const
	{
		vec3_t inv_delta{};
		for (int i = 0; i < 3; ++i) {
			if (delta[i] != 0.0f)
				inv_delta[i] = 1.0f / delta[i];
			else
				inv_delta[i] = FLT_MAX;
		}

		return inv_delta;
	}
};

class i_trace_filter
{
public:
	virtual bool should_hit_entity(c_base_entity* entity, int mask) = 0;
	virtual trace_type_t get_trace_type() const = 0;
};

class c_trace_filter : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask);

	virtual trace_type_t get_trace_type() const
	{
		return TRACE_EVERYTHING;
	}

	INLINE void set_ignore_class(char* Class)
	{
		ignore = Class;
	}

	void* skip;
	char* ignore = new char[1];
};

class c_trace_filter_one_entity : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return !(this->entity == entity);
	}

	trace_type_t get_trace_type() const
	{
		return TRACE_EVERYTHING;
	}

	void* entity;
};

class c_trace_filter_skip_entity : public i_trace_filter
{
public:
	c_trace_filter_skip_entity(c_base_entity* entity)
	{
		skip = entity;
	}

	bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return !(entity == skip);
	}

	virtual trace_type_t get_trace_type() const
	{
		return TRACE_EVERYTHING;
	}

	void* skip;
};

class c_trace_filter_entities_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return true;
	}

	virtual trace_type_t get_trace_type() const
	{
		return TRACE_ENTITIES_ONLY;
	}
};

class c_trace_filter_world_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return false;
	}

	virtual trace_type_t get_trace_type() const
	{
		return TRACE_WORLD_ONLY;
	}
};

class c_trace_filter_world_and_props_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return false;
	}

	virtual trace_type_t get_trace_type() const
	{
		return TRACE_EVERYTHING;
	}
};

class c_trace_filter_hit_all : public c_trace_filter
{
public:
	virtual bool should_hit_entity(c_base_entity* entity, int mask)
	{
		return true;
	}
};

class c_trace_filter_no_npcs_or_player : public i_trace_filter
{
public:
	bool should_hit_entity(c_base_entity* entity, int mask);

	virtual trace_type_t get_trace_type() const {
		return TRACE_EVERYTHING;
	}

	c_base_entity* skip;
};

using should_hit_fn = bool(__cdecl*)(c_base_entity* const, const int);
class c_trace_filter_simple
{
public:
	std::uintptr_t vtable;
	c_base_entity* ignore_entity;
	int collision_group;
	should_hit_fn should_hit;

	c_trace_filter_simple();
	c_trace_filter_simple(c_base_entity* ignore_entity,
		const int collision_group = 0, should_hit_fn should_hit = nullptr);
};

class c_trace_filter_skip_two_entities
{
public:
	std::uintptr_t vtable;
	c_base_entity* first_ignore_entity;
	int collision_group;
	should_hit_fn should_hit;
	c_base_entity* second_ignore_entity;

	c_trace_filter_skip_two_entities();
	c_trace_filter_skip_two_entities(c_base_entity* first_ignore_entity,
		c_base_entity* second_ignore_entity, const int collision_group = 0, should_hit_fn should_hit = nullptr);
};

class c_base_trace
{
public:
	INLINE bool is_disp_surface() { return ((disp_flags & DISPSURF_FLAG_SURFACE) != 0); }
	INLINE bool is_disp_surface_walkable() { return ((disp_flags & DISPSURF_FLAG_WALKABLE) != 0); }
	INLINE bool is_disp_surface_buildable() { return ((disp_flags & DISPSURF_FLAG_BUILDABLE) != 0); }
	INLINE bool is_disp_surface_prop1() { return ((disp_flags & DISPSURF_FLAG_SURFPROP1) != 0); }
	INLINE bool is_disp_surface_prop2() { return ((disp_flags & DISPSURF_FLAG_SURFPROP2) != 0); }
public:
	vec3_t start;
	vec3_t end;
	cplane_t plane;

	float fraction;

	int contents;
	uint16_t disp_flags;

	bool all_solid;
	bool start_solid;

	c_base_trace() {}
};

class c_game_trace : public c_base_trace
{
public:
	bool did_hit_world() const;
	bool did_hit_non_world_entity() const;
	bool did_hit() const;
	bool is_visible() const;
public:

	float fraction_left_solid;
	csurface_t surface;
	std::uint32_t hitgroup;
	std::int16_t physics_bone;
	std::uint16_t world_surface_index;
	c_base_entity* entity;
	std::int32_t hitbox;

	c_game_trace() {}

	INLINE void clear()
	{
		std::memset(this, 0, sizeof(c_game_trace));

		fraction = 1.f;
		surface.name = XOR("**empty**").c_str();
	}

private:
	// No copy constructors allowed
	c_game_trace(const c_game_trace& other) :
		fraction_left_solid(other.fraction_left_solid),
		surface(other.surface),
		hitgroup(other.hitgroup),
		physics_bone(other.physics_bone),
		world_surface_index(other.world_surface_index),
		entity(other.entity),
		hitbox(other.hitbox)
	{
		start = other.start;
		end = other.end;
		plane = other.plane;
		fraction = other.fraction;
		contents = other.contents;
		disp_flags = other.disp_flags;
		all_solid = other.all_solid;
		start_solid = other.start_solid;
	}
};

INLINE bool c_game_trace::did_hit() const
{
	return fraction < 1 || all_solid || start_solid;
}

INLINE bool c_game_trace::is_visible() const
{
	return fraction > 0.97f;
}
#pragma endregion

#pragma region RENDERVIEW
class c_view_setup
{
public:
	PAD(176);
	float fov;
	float viewmodel_fov;
	vec3_t origin;
	vec3_t angles;
};
#pragma endregion

#pragma region ENTITIES
struct cmd_context_t
{
	bool needs_processing;
	c_user_cmd user_cmd;
	int cmd_number;
};

class c_animation_layers
{
public:
	bool client_blend;
	float blend_in;
	void* studio_hdr;

	int dispatch_sequence;
	int second_dispatch_sequence;

	std::uint32_t order;
	std::uint32_t sequence;

	float prev_cycle;
	float weight;
	float weight_delta_rate;
	float playback_rate;
	float cycle;

	void* owner;

	PAD(4);

	INLINE void copy_to(c_animation_layers& other) {
		std::memcpy(this, &other, sizeof(*this));
	}
};

class c_bone_accessor
{
public:
	alignas(16) matrix3x4_t* bones;

	int readable_bones;
	int writable_bones;
};

struct animstate_pose_param_cache_t
{
	bool initialized;
	int index;
	const char* name;

	animstate_pose_param_cache_t()
	{
		initialized = false;
		index = -1;
		name = "";
	}
	void set_value(c_cs_player* player, float value);
	float get_value(c_cs_player* player);
};

struct aimmatrix_transition_t
{
	float	m_flDurationStateHasBeenValid;
	float	m_flDurationStateHasBeenInValid;
	float	m_flHowLongToWaitUntilTransitionCanBlendIn;
	float	m_flHowLongToWaitUntilTransitionCanBlendOut;
	float	m_flBlendValue;

	void UpdateTransitionState(bool bStateShouldBeValid, float flTimeInterval, float flSpeed)
	{
		if (bStateShouldBeValid)
		{
			m_flDurationStateHasBeenInValid = 0;
			m_flDurationStateHasBeenValid += flTimeInterval;
			if (m_flDurationStateHasBeenValid >= m_flHowLongToWaitUntilTransitionCanBlendIn)
			{
				m_flBlendValue = math::approach(1, m_flBlendValue, flSpeed);
			}
		}
		else
		{
			m_flDurationStateHasBeenValid = 0;
			m_flDurationStateHasBeenInValid += flTimeInterval;
			if (m_flDurationStateHasBeenInValid >= m_flHowLongToWaitUntilTransitionCanBlendOut)
			{
				m_flBlendValue = math::approach(0, m_flBlendValue, flSpeed);
			}
		}
	}

	void Init(void)
	{
		m_flDurationStateHasBeenValid = 0;
		m_flDurationStateHasBeenInValid = 0;
		m_flHowLongToWaitUntilTransitionCanBlendIn = 0.3f;
		m_flHowLongToWaitUntilTransitionCanBlendOut = 0.3f;
		m_flBlendValue = 0;
	}

	aimmatrix_transition_t()
	{
		Init();
	}
};

class c_animation_state
{
public:
	INLINE c_animation_state() {}
	INLINE c_animation_state(const c_animation_state& animstate) {
		std::memcpy(this, &animstate, sizeof(c_animation_state));
	}

	int* layer_order_preset{};

	bool first_run_since_init{};
	bool first_foot_plant_since_init{};

	int last_update_tick{};
	float eye_pos_smooth_lerp{};
	float strafe_weight_smoooth_fall_off{};

	aimmatrix_transition_t	m_tStandWalkAim;
	aimmatrix_transition_t	m_tStandRunAim;
	aimmatrix_transition_t	m_tCrouchWalkAim;

	int cached_model_index{};

	float step_height_left{};
	float step_height_right{};

	void* weapon_last_bone_setup{};

	void* player{};
	void* weapon{};
	void* weapon_last{};

	float last_update_time{};
	int last_update_frame{};
	float last_update_increment{};

	float eye_yaw{};
	float eye_pitch{};
	float abs_yaw{};
	float abs_yaw_last{};
	float move_yaw{};
	float move_yaw_ideal{};
	float move_yaw_current_to_ideal{};

	PAD(4);

	float primary_cycle{};
	float move_weight{};

	float move_weight_smoothed{};
	float anim_duck_amount{};
	float duck_additional{};
	float recrouch_weight{};

	vec3_t position_current{};
	vec3_t position_last{};

	vec3_t velocity{};
	vec3_t velocity_normalized{};
	vec3_t velocity_normalized_non_zero{};

	float velocity_length_xy{};
	float velocity_length_z{};

	float speed_as_portion_of_run_top_speed{};
	float speed_as_portion_of_walk_top_speed{};
	float speed_as_portion_of_crouch_top_speed{};

	float duration_moving{};
	float duration_still{};

	bool on_ground{};
	bool landing{};

	float jump_to_fall{};
	float duration_in_air{};
	float left_ground_height{};
	float land_anim_multiplier{};
	float walk_run_transition{};

	bool landed_on_ground_this_frame{};
	bool left_the_ground_this_frame{};

	float in_air_smooth_value{};

	bool on_ladder{};
	float ladder_weight{};
	float ladder_speed{};

	bool walk_to_run_transition_state{};

	bool defuse_started{};
	bool plant_anim_started{};
	bool twitch_anim_started{};
	bool adjust_started{};

	char activity_modifiers_server[20]{};

	float next_twitch_time{};
	float time_of_last_known_injury{};
	float last_velocity_test_time{};

	vec3_t velocity_last{};
	vec3_t target_acceleration{};
	vec3_t acceleration{};

	float acceleration_weight{};

	float aim_matrix_transition{};
	float aim_matrix_transition_delay{};

	bool flashed{};

	float strafe_change_weight{};
	float strafe_change_target_weight{};
	float strafe_change_cycle{};

	int strafe_sequence{};

	bool strafe_changing{};

	float duration_strafing{};
	float foot_lerp{};

	bool feet_crossed{};
	bool player_is_accelerating{};

	animstate_pose_param_cache_t pose_param_mappings[PLAYER_POSE_PARAM_COUNT]{};

	float duration_move_weight_is_too_high{};
	float static_approach_speed{};

	int previous_move_state{};
	float stutter_step{};

	float action_weight_bias_remainder{};

	PAD(112);

	float camera_smooth_height{};
	bool smooth_height_valid{};
	float last_time_velocity_over_ten{};

#ifndef LEGACY
	PAD(4);
#endif

	float aim_yaw_min{};
	float aim_yaw_max{};
	float aim_pitch_min{};
	float aim_pitch_max{};

	int animstate_model_version{};

	float last_amt = -1;
	float last_exponent = 0;

	INLINE float bias(float x, float bias_amt) {
		if (last_amt != bias_amt) {
			last_exponent = std::log(bias_amt) * -1.4427f;
		}
		return std::pow(x, last_exponent);
	}

	void create(c_cs_player* player);
	void update(const vec3_t& angle);
	void reset();

	float get_min_rotation();
	float get_max_rotation();

	void increment_layer_cycle(c_animation_layers* layer, bool loop);
	void increment_layer_weight(c_animation_layers* layer);
	bool is_layer_sequence_finished(c_animation_layers* layer, float time);
	void set_layer_cycle(c_animation_layers* layer, float_t cycle);
	void set_layer_rate(c_animation_layers* layer, float rate);
	void set_layer_weight(c_animation_layers* layer, float weight);
	void set_layer_weight_rate(c_animation_layers* layer, float prev);
	void set_layer_sequence(c_animation_layers* layer, int activity);
	float get_layer_ideal_weight_from_seq_cycle(c_animation_layers* layer, int index);
	int select_sequence_from_activity_modifier(int iActivity);
	void update_layer(c_animation_layers* layer, int sequence, float rate, float cycle, float weight, int index);
};

struct weapon_info_t
{
#ifdef LEGACY
public:
	PAD(0x4);
public:
	const char* weapon_name{};

	PAD(0xC);

	int max_ammo_1{};
	int max_ammo_2{};
	int default_clip1{};
	int default_clip2{};
	int max_reserve{};

	PAD(0x4);

	const char* world_model{};
	const char* view_model{};
	const char* world_dropped_model{};

	PAD(0x48);

	const char* ammo_type{};

	PAD(4);

	const char* hud_name{};
	const char* deprecated_weapon_name{};

	PAD(56);
	int		weapon_type{};
	int		in_game_price{};
	int		kill_award{};
	const char* animation_prefix{};
	float	cycletime{};
	float	cycletime_alt{};
	float	time_to_idle{};
	float	idle_interval{};
	bool	is_full_auto{};

	PAD(0x3);

	int		dmg{};
	float	armor_ratio{};
	int		bullets{};
	float	penetration{};
	float	flinch_velocity_modifier_large{};
	float	flinch_velocity_modifier_small{};
	float	range{};
	float	range_modifier{};
	float	throw_velocity{};

	PAD(0xC);

	bool	has_silencer{};
	PAD(0x3);

	const char* silencer_model{};
	int		crosshair_min_distance{};
	int		crosshair_delta_distance{};
	float	max_speed{};
	float	max_speed_alt{};
	float	spread{};
	float	spread_alt{};
	float	inaccuracy_crouch{};
	float	inaccuracy_crouch_alt{};
	float	inaccuracy_stand{};
	float	inaccuracy_stand_alt{};
	float	inaccuracy_jump_initial{};
	float	inaccuracy_jump{};
	float	inaccuracy_jump_alt{};
	float	inaccuracy_land{};
	float	inaccuracy_land_alt{};
	float	inaccuracy_ladder{};
	float	inaccuracy_ladder_alt{};
	float	inaccuracy_fire{};
	float	inaccuracy_fire_alt{};
	float	inaccuracy_move{};
	float	inaccuracy_move_alt{};
	float	inaccuracy_reload{};
#else
	PAD(20);
	std::uint32_t max_ammo_1;
	PAD(12);
	std::uint32_t max_ammo_2;
	PAD(84);
	char* N00000985;
	PAD(8);
	char* hud_name;
	char* weapon_name;
	PAD(56);
	std::uint32_t weapon_type;
	PAD(36);
	std::uint32_t dmg;
	float crosshair_delta_distance;
	float armor_ratio;
	std::uint32_t bullets;
	float penetration;
	float flinch_velocity_modifier_large;
	float flinch_velocity_modifier_small;
	float range;
	float range_modifier;
	float throw_velocity;
	PAD(20);
	std::uint32_t crosshair_delta_dist;
	std::uint32_t crosshair_min_dist;
	float max_speed;
	float max_speed_alt;
	PAD(12);
	float inaccuracy_crouch;
	float inaccuracy_crouch_alt;
	float inaccuracy_stand;
	float inaccuracy_stand_alt;
	float inaccuracy_jump;
	float inaccuracy_jump_alt;
	float inaccuracy_land;
	float inaccuracy_land_alt;
	PAD(96);
	bool unk;
	PAD(4);
	bool hide_viewmodel_in_zoom;
#endif
};
#pragma endregion

#pragma region GAMERULES
struct view_vectors_t
{
	vec3_t view{};
	vec3_t hull_min{};
	vec3_t hull_max{};
	vec3_t duck_hull_min{};
	vec3_t duck_hull_max{};
	vec3_t duck_view{};
	vec3_t obs_hull_min{};
	vec3_t obs_hull_max{};
	vec3_t dead_view_height{};
};
#pragma endregion

#pragma region BEAM
struct beam_info_t
{
	beam_types_t type{};

	void* start_ent{};
	int start_attachment{};

	void* end_ent{};
	int end_attachment{};

	vec3_t start{};
	vec3_t end{};

	int model_index{};
	const char* model_name{};

	int halo_index{};
	const char* halo_name{};
	float halo_scale{};

	float life{};
	float width{};
	float end_width{};
	float fade_lenght{};
	float amplitude{};

	float brightness{};
	float speed{};

	int start_frame{};
	float frame_rate{};

	float red{};
	float green{};
	float blue{};

	bool renderable{};

	int segments{};

	int flags{};

	vec3_t center{};
	float start_radius{};
	float end_radius{};

	beam_info_t()
	{
		type = BEAM_NORMAL;
		segments = -1;
		model_name = NULL;
		halo_name = NULL;
		model_index = -1;
		halo_index = -1;
		renderable = true;
		flags = 0;
	}
};
#pragma endregion

#pragma region HUD
#ifdef LEGACY
struct notice_text_t
{
	wchar_t text[512];
	int unk0;
	float unk1;
	float unk2;
	int unk3;
	float time;
	int unk4;
	float fade;
	int unk5;
};

struct kill_feed_t
{
	PAD(0x7C);
	c_utl_vector<notice_text_t> notices{};
};
#else
class c_ui_panel
{
public:
	VFUNC(get_child_count(), int(__thiscall*)(decltype(this)), 48);
	VFUNC(get_child(int n), c_ui_panel* (__thiscall*)(decltype(this), int), 49, n);
	VFUNC(has_class(const char* name), bool(__thiscall*)(decltype(this), const char*), 139, name);
	VFUNC(set_attribute_float(const char* name, float value), void(__thiscall*)(void*, const char*, float), 288, name, value);
};
#endif
#pragma rendregion

#pragma region STATIC_PROPS
class c_client_alpha_property
{
public:
	virtual void* get_unknown() = 0;
	virtual void set_alpha_modulation(std::uint8_t a) = 0;
};

class c_static_prop
{
public:
#ifdef LEGACY
	PAD(72);
	c_client_alpha_property* alpha_property;
	PAD(112);
	vec4_t diffuse_modulation;
#else
	PAD(16);
	vec3_t origin;
	PAD(24);
	std::uint32_t alpha;
	PAD(20);
	c_client_alpha_property* alpha_property; //0x004C
	PAD(160);
	vec4_t diffuse_modulation;
#endif
};

class c_static_prop_manager
{
public:
	void* unk;
	void* unk2;
	PAD(20);
	c_static_prop* props_base;
	std::uint32_t props_count;
	std::uint32_t props_unk;
	std::uint32_t props_unk2;
	PAD(24);
	bool level_initialized;
	bool client_initialized;
	PAD(2);
	vec3_t last_view_origin;
	float last_view_factor;
	std::uint32_t last_cpu_level;
	std::uint32_t last_gpu_level;
};
#pragma endregion

#pragma region MATERIAL_CONFIG
struct MaterialVideoMode_t {
	int m_Width;
	int m_Height;
	int m_Format;
	int m_RefreshRate;
};

struct material_system_config_t {
	MaterialVideoMode_t m_VideoMode;
	float m_fMonitorGamma;
	float m_fGammaTVRangeMin;
	float m_fGammaTVRangeMax;
	float m_fGammaTVExponent;
	bool m_bGammaTVEnabled;
	bool m_bTripleBuffered;
	int m_nAASamples;
	int m_nForceAnisotropicLevel;
	int m_nSkipMipLevels;
	int m_nDxSupportLevel;
	int m_nFlags;
	bool m_bEditMode;
	char m_nProxiesTestMode;
	bool m_bCompressedTextures;
	bool m_bFilterLightmaps;
	bool m_bFilterTextures;
	bool m_bReverseDepth;
	bool m_bBufferPrimitives;
	bool m_bDrawFlat;
	bool m_bMeasureFillRate;
	bool m_bVisualizeFillRate;
	bool m_bNoTransparency;
	bool m_bSoftwareLighting;
	bool m_bAllowCheats;
	char m_nShowMipLevels;
	bool m_bShowLowResImage;
	bool m_bShowNormalMap;
	bool m_bMipMapTextures;
	char m_nFullbright;
	bool m_bFastNoBump;
	bool m_bSuppressRendering;
	bool m_bDrawGray;
	bool m_bShowSpecular;
	bool m_bShowDiffuse;
	int  m_nWindowedSizeLimitWidth;
	int  m_nWindowedSizeLimitHeight;
	int  m_nAAQuality;
	bool m_bShadowDepthTexture;
	bool m_bMotionBlur;
	bool m_bSupportFlashlight;
	bool m_bPaintEnabled;
	PAD(0xC);
};
#pragma endregion