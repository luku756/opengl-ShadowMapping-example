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
	bool shadow_on;
};

struct MATERIAL {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 emissive_color;
	float specular_exponent;
};

#define NUMBER_OF_OBJECT 8
uniform MATERIAL object_materials[NUMBER_OF_OBJECT];
uniform int object_id;

uniform vec4 u_global_ambient_color;
#define NUMBER_OF_LIGHTS_SUPPORTED 4
#define NUMBER_OF_LIGHT_COUNT 4 
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];
//uniform MATERIAL u_material;

uniform sampler2D u_base_texture;
uniform sampler2DShadow  u_shadow_texture[NUMBER_OF_LIGHT_COUNT];

uniform bool u_flag_texture_mapping = true;
uniform bool u_flag_fog = false;

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
in vec2 v_tex_coord;
in vec4 v_shadow_coord[NUMBER_OF_LIGHT_COUNT];	//그림자가 안 되는 이유. 이게 오브젝트마다 있는데...;;;

layout (location = 0) out vec4 final_color;
layout(location = 1) out vec3 PositionData;
layout(location = 2) out vec3 NormalData;
layout(location = 3) out vec3 ObjIDData;
layout(location = 4) out vec3 diffData;
layout(location = 5) out float AoData;
/*layout(location = 4) out vec3 ambientData;
layout(location = 5) out vec3 specualrData;
layout(location = 6) out vec3 emissiveData;
layout(location = 7) out vec3 specexponentData*/;

const int kernelSize = 64;

uniform int Pass;   // Pass number
uniform vec3 SampleKernel[kernelSize];
uniform float Radius = 0.55;

uniform mat4 ProjectionMatrix;

vec4 lighting_equation_textured_defferd(in vec3 P_EC, in vec3 N_EC, in vec4 diffColor, in vec4 ambientColor, 
	in vec4 emissiveColor, in vec4 specColor, in float specExponet) {
	vec4 color_sum;
	float local_scale_factor, tmp_float, shadow_factor;
	vec3 L_EC;

	color_sum = emissiveColor + u_global_ambient_color * diffColor;

	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		if (u_light[i].shadow_on) {
			shadow_factor = textureProj(u_shadow_texture[i], v_shadow_coord[i]);
		}
		else
			shadow_factor = 1.0f;

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
			vec4 local_color_sum = u_light[i].ambient_color * ambientColor;

			if (shadow_factor > zero_f) {
				tmp_float = dot(N_EC, L_EC);
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].diffuse_color*diffColor*tmp_float;

					vec3 H_EC = normalize(L_EC - normalize(P_EC));
					tmp_float = dot(N_EC, H_EC);
					if (tmp_float > zero_f) {
						local_color_sum += u_light[i].specular_color *
							specColor * pow(tmp_float, specExponet);
					}
				}
			}

			color_sum += local_scale_factor * local_color_sum;
		}
	}
	return color_sum;
}


vec4 lighting_equation_textured_defferd_id(in vec3 P_EC, in vec3 N_EC, in int obj_id, in vec4 base_color) {
	vec4 color_sum;
	float local_scale_factor, tmp_float, shadow_factor;
	vec3 L_EC;
	
	color_sum = object_materials[obj_id].emissive_color + u_global_ambient_color * base_color;

	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		if (u_light[i].shadow_on) {
			shadow_factor = textureProj(u_shadow_texture[i], v_shadow_coord[i]);
		}
		else
			shadow_factor = 1.0f;

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
			vec4 local_color_sum = u_light[i].ambient_color * object_materials[obj_id].ambient_color;

			if (shadow_factor > zero_f) {
				tmp_float = dot(N_EC, L_EC);
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].diffuse_color* base_color *tmp_float;

					vec3 H_EC = normalize(L_EC - normalize(P_EC));
					tmp_float = dot(N_EC, H_EC);
					if (tmp_float > zero_f) {
						local_color_sum += u_light[i].specular_color *
							object_materials[obj_id].specular_color * pow(tmp_float, object_materials[obj_id].specular_exponent);
					}
				}
			}

			color_sum += local_scale_factor * local_color_sum;
		}
	}
	return color_sum;
}


void pass1() {
	// Store position, normal, and diffuse color in textures
	PositionData = v_position_EC.xyz;
	NormalData = normalize(v_normal_EC);
	
	//diffData = vec3(object_materials[object_id].diffuse_color);
	//diffData = vec3(object_materials[object_id].diffuse_color);
	//ambientData = vec3(object_materials[object_id].ambient_color);
	//specualrData = vec3(object_materials[object_id].specular_color);
	//emissiveData = vec3(object_materials[object_id].emissive_color);
	//specexponentData = vec3(object_materials[object_id].specular_exponent, object_id+1.5f,1.0f);
	float tex_flag = 0.0f;
	if (u_flag_texture_mapping == true) {
		tex_flag = 1.0f;
		diffData = vec3(texture(u_base_texture, v_tex_coord));
	}
	else {
		diffData = vec3(object_materials[object_id].diffuse_color);
	}
	ObjIDData = vec3(object_id + 1.5f, tex_flag,0.0f);//텍스쳐가 있을 경우 g가 1, 없을 경우(기본 diffuse) 0.
}

//for deferred
uniform sampler2D PositionTex;
uniform sampler2D NormalTex;
uniform sampler2D objIDTex;
uniform sampler2D diffTex;
//uniform sampler2D ambientTex;
//uniform sampler2D sepcularTex;
//uniform sampler2D emissiveTex;
//uniform sampler2D specExponentTex;

//for ssao, blur
uniform sampler2D AoTex;
uniform sampler2D RandTex;

const vec2 randScale = vec2(800.0 / 4.0, 600.0 / 4.0);

void pass2() {
	// Create the random tangent space matrix
	vec3 randDir = normalize(texture(RandTex, v_tex_coord.xy * randScale).xyz);
	vec3 n = normalize(texture(NormalTex, v_tex_coord).xyz);
	vec3 biTang = cross(n, randDir);
	if (length(biTang) < 0.0001)  // If n and randDir are parallel, n is in x-y plane
		biTang = cross(n, vec3(0, 0, 1));
	biTang = normalize(biTang);
	vec3 tang = cross(biTang, n);
	mat3 toCamSpace = mat3(tang, biTang, n);

	float occlusionSum = 0.0;
	vec3 camPos = texture(PositionTex, v_tex_coord).xyz;
	for (int i = 0; i < kernelSize; i++) {
		vec3 samplePos = camPos + Radius * (toCamSpace * SampleKernel[i]);

		// Project point
		vec4 p = ProjectionMatrix * vec4(samplePos, 1);
		p *= 1.0 / p.w;
		p.xyz = p.xyz * 0.5 + 0.5;

		// Access camera space z-coordinate at that point
		float surfaceZ = texture(PositionTex, p.xy).z;
		float zDist = surfaceZ - camPos.z;

		// Count points that ARE occluded
		if (zDist >= 0.0 && zDist <= Radius && surfaceZ > samplePos.z) occlusionSum += 1.0;
	}

	float occ = occlusionSum / kernelSize;
	//AoData = 1.0 - occ;
	//FragColor = vec4(AoData, AoData, AoData, 1);

}
void pass3() {}
void pass4() {
	// Retrieve position and normal information from textures
	vec3 pos = vec3(texture(PositionTex, v_tex_coord));
	vec3 norm = vec3(texture(NormalTex, v_tex_coord));
	vec3 diffColor = vec3(texture(diffTex, v_tex_coord));
	//vec3 ambientColor = vec3(texture(ambientTex, v_tex_coord));
	//vec3 emissiveColor = vec3(texture(emissiveTex, v_tex_coord));
	//vec3 specColor = vec3(texture(sepcularTex, v_tex_coord));
	//vec3 specExponet = vec3(texture(specExponentTex, v_tex_coord));

	vec3 flag = vec3(texture(objIDTex, v_tex_coord));

	float tex_flag = flag.y;

	int objid = int(flag.x);
		
	if (objid == 0) {//빈 공간
		final_color = vec4(0, 0, 0, 0);
	}
	else {
		objid--;
		
		vec4 base_color;
		if (tex_flag == 0.0f) {//텍스쳐 미 사용시, 기본 베이스 컬러는 오브젝트의 diffuse color.
			base_color = object_materials[objid].diffuse_color;
		}
		else {
			base_color = vec4(diffColor,1.0f);	//텍스쳐 사용시, 기본 베이스 컬러는 텍스쳐 색상.
		}

		//vec4 shaded_color = lighting_equation_textured_defferd(pos, normalize(norm), vec4(diffColor, 1.0f), vec4(ambientColor, 1.0f), vec4(emissiveColor, 1.0f), vec4(specColor, 1.0f), specExponet.x);
		vec4 shaded_color = lighting_equation_textured_defferd_id(pos, normalize(norm), objid, base_color);

		final_color = shaded_color;
	}
//	final_color = object_materials[0].diffuse_color;
}


void main() {
 if( Pass == 1) pass1();
    else if(Pass==2) pass2();
    else if(Pass == 3) pass3();
    else if(Pass == 4) pass4();

	//if (Pass == 3) {
	//	vec4 base_color, shaded_color;
	//	float fog_factor;

	//	if (u_flag_texture_mapping)
	//		base_color = texture(u_base_texture, v_tex_coord);
	//	else
	//		base_color = u_material.diffuse_color;

	//	shaded_color = lighting_equation_textured_shadow(v_position_EC, normalize(v_normal_EC), base_color);

	//	/*if (u_flag_fog) {
	//		fog_factor = (FOG_FAR_DISTANCE - length(v_position_EC.xyz)) / (FOG_FAR_DISTANCE - FOG_NEAR_DISTANCE);
	//		fog_factor = clamp(fog_factor, 0.0f, 1.0f);
	//		final_color = mix(FOG_COLOR, shaded_color, fog_factor);
	//	}
	//	else*/
	//		final_color = shaded_color;
	//}
	
}

