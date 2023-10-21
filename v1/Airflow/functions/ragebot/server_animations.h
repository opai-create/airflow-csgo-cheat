#pragma once

struct local_anims_t;

namespace server_animations
{
	extern void play_landing_animations(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers);
	extern void run(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers);
	extern void recalculate(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers);
}