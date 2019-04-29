#version 430

// #define DISPLAY_LOD

struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;
	vec4 light_attenuation_factors; // compute this effect only if .w != 0.0f
	vec3 spot_direction;
	float spot_exponent;
	float spot_cutoff_angle;
	bool light_on;
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
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];
uniform MATERIAL u_material;

uniform sampler2D u_base_texture;

uniform bool u_flag_texture_mapping = true;
uniform bool u_flag_fog = false;

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
in vec2 v_tex_coord;
layout (location = 0) out vec4 final_color;
layout(location = 1) out vec3 PositionData;
layout(location = 2) out vec3 NormalData;
layout(location = 3) out vec3 ColorData;

uniform int Pass;   // Pass number


vec3 diffuseModel(vec3 pos, vec3 norm, vec3 diff) {
	//vec3 s = normalize(vec3(Light.Position) - pos);
	//float sDotN = max(dot(s, norm), 0.0);
	//return Light.L * diff * sDotN;


	vec3 L_EC = u_light[0].position.xyz - pos;
	L_EC = normalize(L_EC);
	float nl = max(0, dot(normalize(norm), L_EC.xyz));
	return diff * nl;
}

void pass1() {
	// Store position, normal, and diffuse color in textures
	PositionData = v_position_EC.xyz;
	NormalData = normalize(v_normal_EC);
	ColorData = vec3(u_material.diffuse_color);
}

//layout(binding = 0) uniform sampler2D PositionTex;
//layout(binding = 1) uniform sampler2D NormalTex;
//layout(binding = 2) uniform sampler2D ColorTex;
uniform sampler2D PositionTex;
uniform sampler2D NormalTex;
uniform sampler2D ColorTex;

void pass2() {
	// Retrieve position and normal information from textures
	vec3 pos = vec3(texture(PositionTex, v_tex_coord));
	vec3 norm = vec3(texture(NormalTex, v_tex_coord));
	vec3 diffColor = vec3(texture(ColorTex, v_tex_coord));



	vec3 L_EC;
	L_EC = u_light[0].position.xyz - pos;
	L_EC = normalize(L_EC);
	float nl = max(0, dot(normalize(norm), L_EC.xyz));
	final_color = vec4(diffColor,1.0f) * nl;


	//final_color = vec4(diffuseModel(pos, norm, diffColor), 1.0);
}


void pass3() {
	// Retrieve position and normal information from textures
	vec3 pos = vec3(texture(PositionTex, v_tex_coord));
	vec3 norm = vec3(texture(NormalTex, v_tex_coord));
	vec3 diffColor = vec3(texture(ColorTex, v_tex_coord));


	//vec4 base_color = texture(ColorTex, v_tex_coord);


	vec3 L_EC;
	L_EC = u_light[0].position.xyz - pos;
	L_EC = normalize(L_EC);
	float nl = max(0, dot(normalize(norm), L_EC.xyz));
	final_color = vec4(diffColor, 1.0f) * nl;

//	final_color = base_color;
	//final_color = vec4(diffuseModel(pos, norm, diffColor), 1.0);
}

void main() {
	if (Pass == 1) pass1();
	else if (Pass == 2) pass2();
	else if (Pass == 3) pass3();

	//if (Pass == 1) {
	//	vec3 L_EC;
	//	L_EC = u_light[0].position.xyz - v_position_EC.xyz;
	//	L_EC = normalize(L_EC);
	//	float nl = max(0, dot(normalize(v_normal_EC), L_EC.xyz));
	//	final_color = u_material.diffuse_color * nl;

	//	//final_color = vec4(v_position_EC.xyz, 1.0f);
	//	//final_color = vec4(normalize(v_normal_EC), 1.0f);
	//	//final_color = vec4(u_material.diffuse_color.xyz, 1.0f);
	//}

}


//void main(void) {
//	vec4 base_color, shaded_color;
//	float fog_factor;
//	
//
//	vec3 L_EC;
//	L_EC = u_light[0].position.xyz - v_position_EC.xyz;
//	L_EC = normalize(L_EC);
//	float nl = max(0, dot(normalize(v_normal_EC), L_EC.xyz));
//	final_color= u_material.diffuse_color * nl;
//
//
//	return;
//
//}
