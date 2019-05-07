#version 330

struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;
	vec4 light_attenuation_factors; // compute this effect only if .w != 0.0f
	vec3 spot_direction;
	float spot_exponent;
	float spot_cutoff_angle;
	bool light_on;
	bool shadow_on;
};

struct MATERIAL {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 emissive_color;
	float specular_exponent;
};

uniform vec4 u_global_ambient_color;
#define NUMBER_OF_LIGHTS_SUPPORTED 4
#define NUMBER_OF_LIGHT_COUNT 4 
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];
uniform MATERIAL u_material;

uniform sampler2D u_base_texture;
uniform sampler2DShadow  u_shadow_texture[NUMBER_OF_LIGHT_COUNT];

uniform bool u_flag_texture_mapping = true;
uniform bool u_flag_fog = false;

uniform mat4 u_ViewInvMatrix;
uniform mat4 u_ShadowMatrix[NUMBER_OF_LIGHT_COUNT];

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
in vec2 v_tex_coord;

layout(location = 0) out vec4 final_color;

vec4 lighting_equation_textured(in vec3 P_EC, in vec3 N_EC, in vec4 base_color) {

	vec4 v_shadow_coord[NUMBER_OF_LIGHT_COUNT];

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		v_shadow_coord[i] = u_ShadowMatrix[i] * u_ViewInvMatrix * vec4(P_EC, 1.0f);

	}
//	return vec4(v_shadow_coord[2].xyz, 1.0f);

	vec4 color_sum;
	float local_scale_factor, tmp_float, shadow_factor;
	vec3 L_EC;

	color_sum = u_material.emissive_color + u_global_ambient_color * base_color;

	float sf[4];
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		if (u_light[i].shadow_on) {
			shadow_factor = textureProj(u_shadow_texture[i], v_shadow_coord[i]);
			sf[i] = shadow_factor;
		}
		else {
			shadow_factor = 1.0f;
			sf[i] = shadow_factor;
		}

		local_scale_factor = one_f;
		if (u_light[i].position.w != zero_f) { // point light source
			L_EC = u_light[i].position.xyz - P_EC.xyz;

			if (u_light[i].light_attenuation_factors.w != zero_f) {
				vec4 tmp_vec4;

				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);
				tmp_vec4.y = sqrt(tmp_vec4.z);
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f / dot(tmp_vec4, u_light[i].light_attenuation_factors);
			}

			L_EC = normalize(L_EC);

			if (u_light[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				float spot_cutoff_angle = clamp(u_light[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light[i].spot_direction);

				tmp_float = dot(-L_EC, spot_dir);
				if (tmp_float >= cos(radians(u_light[i].spot_cutoff_angle))) {
					tmp_float = pow(tmp_float, u_light[i].spot_exponent);
				}
				else
					tmp_float = zero_f;
				local_scale_factor *= tmp_float;
			}
		}
		else {  // directional light source
			L_EC = normalize(u_light[i].position.xyz);
		}

		if (local_scale_factor > zero_f) {
			vec4 local_color_sum = u_light[i].ambient_color * u_material.ambient_color;

			if (shadow_factor > zero_f) {
				tmp_float = dot(N_EC, L_EC);
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].diffuse_color*base_color*tmp_float;

					vec3 H_EC = normalize(L_EC - normalize(P_EC));
					tmp_float = dot(N_EC, H_EC);
					if (tmp_float > zero_f) {
						local_color_sum += u_light[i].specular_color *
							u_material.specular_color * pow(tmp_float, u_material.specular_exponent);
					}
				}
			}

			color_sum += local_scale_factor * local_color_sum;
		}
	}
	//return vec4(sf[0], sf[1], sf[2], sf[3]);
	return color_sum;
}



#define FOG_COLOR vec4(0.7f, 0.7f, 0.7f, 1.0f)
#define FOG_NEAR_DISTANCE 350.0f
#define FOG_FAR_DISTANCE 700.0f

void main(void) {
	vec4 base_color, shaded_color;
	float fog_factor;

	if (u_flag_texture_mapping)
		base_color = texture(u_base_texture, v_tex_coord);
	else
		base_color = u_material.diffuse_color;

	shaded_color = lighting_equation_textured(v_position_EC, normalize(v_normal_EC), base_color);

	if (u_flag_fog) {
		fog_factor = (FOG_FAR_DISTANCE - length(v_position_EC.xyz)) / (FOG_FAR_DISTANCE - FOG_NEAR_DISTANCE);
		fog_factor = clamp(fog_factor, 0.0f, 1.0f);
		final_color = mix(FOG_COLOR, shaded_color, fog_factor);
	}
	else
		final_color = shaded_color;

	//final_color = vec4(v_normal_EC,1.0f);
	//final_color = base_color;

//shaded_color =lighting_equation_albedo(v_position_EC, normalize(v_normal_EC), base_color);
//final_color = shaded_color;

	vec3 L_EC;
	L_EC = u_light[0].position.xyz - v_position_EC.xyz;
	L_EC = normalize(L_EC);
	float nl = max(0, dot(normalize(v_normal_EC), L_EC.xyz));
	//final_color = u_material.diffuse_color * nl;


	vec4 v_shadow_coord[NUMBER_OF_LIGHT_COUNT];

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		v_shadow_coord[i] = u_ShadowMatrix[i] * u_ViewInvMatrix * vec4(v_position_EC, 1.0f);
	}

	int i = 1;
	//if (u_light[i].shadow_on) {
	float shadow_factor = textureProj(u_shadow_texture[i], v_shadow_coord[i]);
	//shadow_factor = 1 - shadow_factor;
	//sf[i] = shadow_factor;
//}
//else {
//	shadow_factor = 1.0f;
	//sf[i] = shadow_factor;
//}
	//final_color = vec4(0, shadow_factor, 0, 1.0f);

}
