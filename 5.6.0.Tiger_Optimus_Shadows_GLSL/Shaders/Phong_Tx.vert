#version 400

#define NUMBER_OF_LIGHT_COUNT 4 

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_ModelViewMatrixInvTrans;  
uniform mat4 u_ShadowMatrix[NUMBER_OF_LIGHT_COUNT];

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

out vec3 v_position_EC;
out vec3 v_normal_EC;
out vec2 v_tex_coord;
out vec4 v_shadow_coord[NUMBER_OF_LIGHT_COUNT];

void main(void) {	
	v_position_EC = vec3(u_ModelViewMatrix*vec4(a_position, 1.0f));
	v_normal_EC = normalize(u_ModelViewMatrixInvTrans*a_normal);  
	v_tex_coord = a_tex_coord;

	for(int i=0;i<NUMBER_OF_LIGHT_COUNT;i++)
	v_shadow_coord[i] = u_ShadowMatrix[i] * vec4(a_position, 1.0f);

	gl_Position = u_ModelViewProjectionMatrix * vec4(a_position, 1.0f);
}