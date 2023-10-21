#pragma once

class c_beam;
class beam_t;

enum beam_types_t
{
	beam_normal = 0,
	beam_disk = 2,
	beam_cylinder,
	beam_follow,
	beam_ring,
	beam_spline,
	beam_ring_point,
	beam_laser,
	beam_tesla,
};

enum beam_flags_t
{
	beam_start_entity = 0x00000001,
	beam_end_entity = 0x00000002,
	beam_fade_in = 0x00000004,
	beam_fade_out = 0x00000008,
	beam_sine_no_ise = 0x00000010,
	beam_solid = 0x00000020,
	beam_shade_in = 0x00000040,
	beam_shade_out = 0x00000080,
	beam_only_no_is_once = 0x00000100,
	beam_notile = 0x00000200,
	beam_use_hitboxes = 0x00000400,
	beam_start_visible = 0x00000800,
	beam_end_visible = 0x00001000,
	beam_is_active = 0x00002000,
	beam_forever = 0x00004000,
	beam_halobeam = 0x00008000,
	beam_reversed = 0x00010000,
	num_beam_flags = 17
};

struct beam_info_t
{
	beam_types_t type{};

	c_csplayer* start_ent{};
	int start_attachment{};

	c_csplayer* end_ent{};
	int end_attachment{};

	vector3d start{};
	vector3d end{};

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

	vector3d center{};
	float start_radius{};
	float end_radius{};

	beam_info_t()
	{
		type = beam_normal;
		segments = -1;
		model_name = NULL;
		halo_name = NULL;
		model_index = -1;
		halo_index = -1;
		renderable = true;
		flags = 0;
	}
};

class c_view_render_beams
{
public:
	c_view_render_beams(void);
	virtual ~c_view_render_beams(void);

public:
	virtual void init_beams(void);
	virtual void shut_down_beams(void);
	virtual void clear_beams(void);

	virtual void update_temp_ent_beams();

	virtual void draw_beam(beam_t* beam);
	virtual void draw_beam(c_beam* beam, c_trace_filter* entity_beam_trace_filter = NULL);

	virtual void killed_dead_beams(c_client_entity* dead_entity);

	virtual void create_beam_ents(int start_ent, int end_ent, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float amplitude, float brightness, float speed,
		int start_frame, float framerate, float r, float g, float b, int type = -1);
	virtual beam_t* create_beam_ents(beam_info_t& beam_info);

	virtual void create_beam_ent_point(int start_entity, const vector3d* start, int end_entity, const vector3d* end, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length,
		float amplitude, float brightness, float speed, int start_frame, float framerate, float r, float g, float b);
	virtual beam_t* create_beam_ent_point(beam_info_t& beam_info);

	virtual void create_beam_points(vector3d& start, vector3d& end, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float amplitude, float brightness, float speed,
		int start_frame, float framerate, float r, float g, float b);
	virtual beam_t* create_beam_points(beam_info_t& beam_info);

	virtual void create_beam_ring(int start_ent, int end_ent, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float amplitude, float brightness, float speed,
		int start_frame, float framerate, float r, float g, float b, int flags);
	virtual beam_t* create_beam_ring(beam_info_t& beam_info);

	virtual void create_beam_ring_point(const vector3d& center, float start_radius, float end_radius, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float amplitude,
		float brightness, float speed, int start_frame, float framerate, float r, float g, float b, int flags);
	virtual beam_t* create_beam_ring_point(beam_info_t& beam_info);

	virtual void create_beam_circle_points(int type, vector3d& start, vector3d& end, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float amplitude, float brightness,
		float speed, int start_frame, float framerate, float r, float g, float b);
	virtual beam_t* create_beam_circle_points(beam_info_t& beam_info);

	virtual void create_beam_follow(int start_ent, int model_index, int halo_index, float halo_scale, float life, float width, float end_width, float fade_length, float r, float g, float b, float brightness);
	virtual beam_t* create_beam_follow(beam_info_t& beam_info);
};