#version 330

uniform sampler2D u_shadow_texture;
uniform float u_near_clip_dist;
uniform float u_far_clip_dist;

in vec2 v_tex_coord;
layout (location = 0) out vec4 final_color;

void main(void) {
	float depth;
	depth = texture(u_shadow_texture, v_tex_coord).x;
	depth = (2.0f * u_near_clip_dist) 
		/ (u_far_clip_dist + u_near_clip_dist - depth * (u_far_clip_dist - u_near_clip_dist));
		if(depth >= 1)
	final_color = vec4(0.0, 0.0, depth, 1.0f);  
	else
	final_color = vec4(0.0, 0.0, 0, 1.0f);  
}
