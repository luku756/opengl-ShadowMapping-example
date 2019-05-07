#version 430


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

uniform sampler2D u_base_texture;
uniform sampler2DShadow  u_shadow_texture[NUMBER_OF_LIGHT_COUNT];


uniform bool u_flag_texture_mapping = true;
uniform bool u_flag_fog = false;
uniform bool u_flag_ambient_occlusion = false;
uniform int u_flag_view_mode = 0;

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
in vec2 v_tex_coord;

uniform mat3 u_ModelViewMatrixInvTrans;
uniform mat4 u_ShadowMatrix[NUMBER_OF_LIGHT_COUNT];

layout (location = 0) out vec4 final_color;
layout(location = 1) out vec3 PositionData;
layout(location = 2) out vec3 NormalData;
layout(location = 3) out vec3 ObjIDData;
layout(location = 4) out vec3 diffData;
layout(location = 5) out vec3 AoData;

const int kernelSize = 64;

uniform int Pass;   // Pass number
uniform vec3 SampleKernel[kernelSize];

uniform mat4 ProjectionMatrix;

vec4 lighting_equation_textured_defferd_id_ao(in vec3 P_EC, in vec3 N_EC, in int obj_id, in vec4 base_color, in float ao) {
	if (u_flag_ambient_occlusion == false) {//미사용
		ao = 1;
	}
	
	//그림자 효과
	vec4 v_shadow_coord[NUMBER_OF_LIGHT_COUNT];

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		v_shadow_coord[i] = u_ShadowMatrix[i] * vec4(P_EC, 1.0f);
	}

	vec4 color_sum;
	float local_scale_factor, tmp_float, shadow_factor;
	vec3 L_EC;

	color_sum = object_materials[obj_id].emissive_color + u_global_ambient_color * ao * base_color;
	float f;
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		if (u_light[i].shadow_on) {
			shadow_factor = textureProj(u_shadow_texture[i], v_shadow_coord[i]);

		}
		else {
			shadow_factor = 1.0f;
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
			vec4 local_color_sum = u_light[i].ambient_color* ao * object_materials[obj_id].ambient_color;

			if (shadow_factor > zero_f) {
				tmp_float = dot(N_EC, L_EC);
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].diffuse_color*base_color*tmp_float;

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

	//return vec4(0, 0, f, 1.0f);
	return color_sum;
}


void pass1() {
	PositionData = v_position_EC.xyz;
	NormalData = normalize(v_normal_EC);
	
	//texture 사용시에만 diffData 에 diffuse 기록.
	float tex_flag = 0.0f;
	if (u_flag_texture_mapping == true) {
		tex_flag = 1.0f;
		diffData = vec3(texture(u_base_texture, v_tex_coord));
	}

	//빈 공간의 object id가 0이므로 +1.5f 증가.
	//텍스쳐가 있을 경우 g가 1, 없을 경우(기본 diffuse) 0.
	ObjIDData = vec3(object_id + 1.5f, tex_flag, 0.0f);
}

//for deferred
layout(binding = 7) uniform sampler2D PositionTex;
layout(binding = 8) uniform sampler2D NormalTex;
layout(binding = 9) uniform sampler2D objIDTex;
layout(binding = 10) uniform sampler2D diffTex;

//for ssao, blur
layout(binding = 12) uniform sampler2D AoTex;
layout(binding = 11) uniform sampler2D RandTex;
layout(binding = 20) uniform sampler2D kernelTex;

uniform float Radius = 0.55;
const vec2 randScale = vec2(1200 / 4.0, 800 / 4.0);

void pass2() {

	vec3 randDir = normalize(texture(RandTex, v_tex_coord.xy * randScale).xyz);

	vec3 n = normalize(vec3(texture(NormalTex, v_tex_coord)));
	vec3 biTang = cross(n, randDir);
	if (length(biTang) < 0.0001)  // If n and randDir are parallel, n is in x-y plane
		biTang = cross(n, vec3(0, 0, 1));
	biTang = normalize(biTang);
	vec3 tang = cross(biTang, n);
	mat3 toCamSpace = mat3(tang, biTang, n);

	float occlusionSum = 0.0;
	vec3 camPos = vec3(texture(PositionTex, v_tex_coord));

	for (int i = 0; i < kernelSize; i++) {
		vec3 samplePos = camPos + Radius * (toCamSpace * SampleKernel[i]);
		// Project point
		vec4 p = ProjectionMatrix * vec4(samplePos, 1);
		p *= 1.0 / p.w;
		p.xyz = p.xyz * 0.5 + 0.5;

		// Access camera space z-coordinate at that point
		vec3 surfacePos = vec3(texture(PositionTex, p.xy));
		float surfaceZ = surfacePos.z;
		float zDist = surfaceZ - camPos.z;

		// Count points that ARE occluded
		if (zDist >= 0.0 && zDist <= Radius && surfaceZ > samplePos.z)
			occlusionSum += 1.0;
	}


	float occ = 1 - (occlusionSum / 64.0f);
	//occ = pow(occ, 0.3);
	occ = pow(occ, 4);

	//vec3 flag = vec3(texture(objIDTex, v_tex_coord));

	//float tex_flag = flag.y;//텍스쳐를 입힌 오브젝트인가
	//int objid = int(flag.x);//오브젝트의 ID

	//if (objid == 3) {//빈 공간
	//	occ = 1;
	//}
	AoData = vec3(occ,0,0);
}


void pass3() {

	ivec2 pix = ivec2(gl_FragCoord.xy);
	float sum = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; y++) {
			sum += texelFetchOffset(AoTex, pix, 0, ivec2(x, y)).r;
		}
	}

	float ao = sum * (1.0 / 9.0);
	AoData = vec3( ao,0,0);
}

void pass4() {
	// Retrieve position and normal information from textures
	vec3 pos = vec3(texture(PositionTex, v_tex_coord));
	vec3 norm = vec3(texture(NormalTex, v_tex_coord));
	float aoVal = texture(AoTex, v_tex_coord).r;

	vec3 flag = vec3(texture(objIDTex, v_tex_coord));

	float tex_flag = flag.y;//텍스쳐를 입힌 오브젝트인가
	int objid = int(flag.x);//오브젝트의 ID
		
	if (objid == 0) {//빈 공간
		final_color = vec4(0, 0, 0, 0);	}
	else {
		objid--;//obj id는 0부터 시작, 빈 공간을 0으로 하기 위해 +1 해서 기록.		
		vec4 base_color;
		if (tex_flag == 0.0f) {//텍스쳐 미 사용시, 기본 베이스 컬러는 오브젝트의 diffuse color.
			base_color = object_materials[objid].diffuse_color;
		}
		else {
			base_color = texture(diffTex, v_tex_coord);	//텍스쳐 사용시, 기본 베이스 컬러는 텍스쳐 색상.
		}

		vec4 shaded_color = lighting_equation_textured_defferd_id_ao(pos, normalize(norm), objid, base_color, aoVal);

		final_color = shaded_color;
	}
}


void main() {
 if( Pass == 1) pass1();
    else if(Pass==2) pass2();
    else if(Pass == 3) pass3();
	else if (Pass == 4) { 
	 pass4();
	
	 if (u_flag_view_mode == 1) {
		 final_color = texture(PositionTex, v_tex_coord);

	 }
	 else if (u_flag_view_mode == 2) {
		 final_color = texture(NormalTex, v_tex_coord);

	 }
	 else if (u_flag_view_mode == 3) {
		 final_color = vec4(normalize(vec3(texture(objIDTex, v_tex_coord).x,1,0)),1.0f);

	 }
	 else if (u_flag_view_mode == 4) {
		 final_color = texture(AoTex, v_tex_coord);

	 }
 
 };

}

