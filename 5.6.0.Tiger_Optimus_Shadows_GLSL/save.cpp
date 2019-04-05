#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_shadow, h_ShaderProgram_SHOW_SM;

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color;

// for Phong Shading shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
#define NUMBER_OF_LIGHT_COUNT 2
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_base_texture, loc_flag_texture_mapping, loc_flag_fog;
GLint loc_ShadowMatrix_TXPS, loc_shadow_texture[NUMBER_OF_LIGHT_COUNT];
GLint loc_shadow_texture_SHOW_SM;

// for shadow shaders
GLint loc_ModelViewProjectionMatrix_shadow;

// for drawing shadow map
GLint loc_ModelViewProjectionMatrix_SHOW_SM;
GLint loc_near_clip_dist_SHOW_SM, loc_far_clip_dist_SHOW_SM;

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix;
glm::mat4 ShadowMatrix[NUMBER_OF_LIGHT_COUNT], BiasMatrix, ViewMatrix_SHADOW[NUMBER_OF_LIGHT_COUNT], ProjectionMatrix_SHADOW[NUMBER_OF_LIGHT_COUNT];

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];
float light0_position_WC[2][4], light0_lerp_t; // for light animation

struct _flag {
	int texture_mapping;
	int fog;
	int show_shadow_map;
	int cull_face;
	int tiger_move;
	int tiger_timestamp = 0;
	int tiger_speed;
} flag;

// for shadow mapping
struct _ShadowMapping {
	GLint texture_unit; // Must be equal to N_NORMAL_TEXTURES_USED
	GLuint shadow_map_ID;
	GLsizei shadow_map_width, shadow_map_height;
	GLfloat near_dist, far_dist, shadow_map_border_color[4];
	GLuint FBO_ID;
} ShadowMapping[3];

struct _WINDOW_param {
	int width, height;
} WINDOW_param;

// texture stuffs
#define N_NORMAL_TEXTURES_USED 2

#define TEXTURE_INDEX_FLOOR 0
#define TEXTURE_INDEX_TIGER 1
#define TEXTURE_INDEX_SHADOW 2

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048
#define SHADOW_MAP_NEAR_DIST 200.0f
#define SHADOW_MAP_FAR_DIST 1500.0f

GLuint texture_names[N_NORMAL_TEXTURES_USED];

void My_glTexImage2D_from_file(const char *filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP *tx_pixmap, *tx_pixmap_32;

	int width, height;
	GLvoid *data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);
}

// for tiger animation
int cur_frame_tiger = 0;
float rotation_angle_tiger = 0.0f;
int rotation_speed_tiger = 100;

#include "Objects.h"

void build_shadow_map(int shadowindex) { // The first pass in shadow mapping
	glm::mat4 ViewProjectionMatrix_SHADOW;

	ViewProjectionMatrix_SHADOW = ProjectionMatrix_SHADOW[shadowindex] * ViewMatrix_SHADOW[shadowindex];

	glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapping[shadowindex].FBO_ID);	//이 프레임 버퍼에 그리기
	glViewport(0, 0, ShadowMapping[shadowindex].shadow_map_width, ShadowMapping[shadowindex].shadow_map_height);

	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(10.0f, 10.0f);

	glUseProgram(h_ShaderProgram_shadow);

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix_SHADOW, glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_shadow, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_floor();

	ModelViewProjectionMatrix = glm::rotate(ViewProjectionMatrix_SHADOW, -rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = glm::translate(ModelViewProjectionMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_shadow, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger();

	// for drawing Optimus
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix_SHADOW, glm::vec3(0.3f, 0.3f, 0.3f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_shadow, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_OPTIMUS, 1.0f, 1.0f, 1.0f, GL_CCW);

	glUseProgram(0);

	glFinish();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//디폴트 프레임 버퍼(화면) 으로 복귀
}

void draw_scene_with_shadow(void) { // The second pass in shadow mapping
	glm::mat4 ModelMatrix;

	glViewport(0, 0, WINDOW_param.width, WINDOW_param.height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(h_ShaderProgram_TXPS);
	//glUniform1i(loc_shadow_texture, ShadowMapping.texture_unit);

	set_material_floor();
	glUniform1i(loc_base_texture, TEXTURE_INDEX_FLOOR);

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		ShadowMatrix[i] = BiasMatrix * ProjectionMatrix_SHADOW[i] * ViewMatrix_SHADOW[i] * ModelMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		glUniformMatrix4fv(loc_ShadowMatrix_TXPS, 1, GL_FALSE, &ShadowMatrix[i][0][0]);
	draw_floor();

	set_material_tiger();
	glUniform1i(loc_base_texture, TEXTURE_INDEX_TIGER);
	ModelMatrix = glm::rotate(glm::mat4(1.0f), -rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		ShadowMatrix[i] = BiasMatrix * ProjectionMatrix_SHADOW[i] * ViewMatrix_SHADOW[i] * ModelMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		glUniformMatrix4fv(loc_ShadowMatrix_TXPS, 1, GL_FALSE, &ShadowMatrix[i][0][0]);
	draw_tiger();

	glUseProgram(h_ShaderProgram_simple);
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(3.0f);
	draw_axes();
	glLineWidth(1.0f);

	// for drawing Optimus
	glUseProgram(h_ShaderProgram_TXPS);

	set_material_object(OBJECT_OPTIMUS);
	ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
	ModelMatrix = glm::rotate(ModelMatrix, rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		ShadowMatrix[i] = BiasMatrix * ProjectionMatrix_SHADOW[i] * ViewMatrix_SHADOW[i] * ModelMatrix;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		glUniformMatrix4fv(loc_ShadowMatrix_TXPS, 1, GL_FALSE, &ShadowMatrix[i][0][0]);
	draw_object(OBJECT_OPTIMUS, 1.0f, 1.0f, 1.0f, GL_CCW);

	glUseProgram(h_ShaderProgram_simple);
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(3.0f);
	draw_axes();
	glLineWidth(1.0f);

	glUseProgram(0);
}

void draw_shadow_map(void) {
	glViewport(0, 0, WINDOW_param.width, WINDOW_param.height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_SHOW_SM);
	draw_floor(); // Borrow this function to draw a unit rectangle.
	glUseProgram(0);

}

// callbacks
void display(void) {
	build_shadow_map(0);
	build_shadow_map(1);
	//build_shadow_map(2);

	if (flag.show_shadow_map)
		draw_shadow_map();
	else
		draw_scene_with_shadow();

	glutSwapBuffers();
}

void timer_scene(int timer_flag) {
	if (timer_flag) {
		cur_frame_tiger = flag.tiger_timestamp % N_TIGER_FRAMES;
		rotation_angle_tiger = (flag.tiger_timestamp % 360)*TO_RADIAN;
		glutPostRedisplay();
		flag.tiger_timestamp = flag.tiger_timestamp++ % INT_MAX;
		glutTimerFunc(rotation_speed_tiger, timer_scene, flag.tiger_move);
	}
}

void keyboard(unsigned char key, int x, int y) {
	int i;
	glm::vec4 position_EC;

	if ((key >= '0') && (key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1)) {
		int light_ID = (int)(key - '0');

		glUseProgram(h_ShaderProgram_TXPS);
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		glUseProgram(0);

		glutPostRedisplay();
		return;
	}

	switch (key) {
	case 'f':
		flag.fog = 1 - flag.fog;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_fog, flag.fog);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 't':
		flag.texture_mapping = 1 - flag.texture_mapping;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_texture_mapping, flag.texture_mapping);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 's': // Currently, shadow is cast for light 0 only.
		if (light[0].light_on) {
			light[0].shadow_on = 1 - light[0].shadow_on;
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1i(loc_light[0].shadow_on, light[0].shadow_on);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 'a': // Currently, shadow is cast for light 0 only.
		if (light[1].light_on) {
			light[1].shadow_on = 1 - light[1].shadow_on;
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1i(loc_light[1].shadow_on, light[1].shadow_on);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 'd':
		flag.show_shadow_map = 1 - flag.show_shadow_map;
		glutPostRedisplay();
		break;
	case 'm':
		flag.tiger_move = 1 - flag.tiger_move;
		if (flag.tiger_move)
			glutTimerFunc(rotation_speed_tiger, timer_scene, 1);
		break;
	case 'r':
		flag.tiger_speed = (++flag.tiger_speed) % 3;
		switch (flag.tiger_speed) {
		case 0:
			rotation_speed_tiger = 10;
			break;
		case 1:
			rotation_speed_tiger = 100;
			break;
		case 2:
			rotation_speed_tiger = 1000;
			break;
		}
		break;
	case 'l':
		if (light[0].light_on) {
			light0_lerp_t += 0.025f;
			if (light0_lerp_t > 1.0001f)  // for numerical error
				light0_lerp_t = 0.0f;
			for (i = 0; i < 4; i++)
				light[0].position[i] = (1.0f - light0_lerp_t)*light0_position_WC[0][i]
				+ light0_lerp_t * light0_position_WC[1][i];
			ViewMatrix_SHADOW[0] = glm::lookAt(glm::vec3(light[0].position[0], light[0].position[1], light[0].position[2]),
				glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			glUseProgram(h_ShaderProgram_TXPS);
			position_EC = ViewMatrix * glm::vec4(light[0].position[0], light[0].position[1], light[0].position[2], light[0].position[3]);
			glUniform4fv(loc_light[0].position, 1, &position_EC[0]);
			glUseProgram(0);
			glutPostRedisplay();
		}
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	WINDOW_param.width = width;
	WINDOW_param.height = height;

	glViewport(0, 0, width, height);

	aspect_ratio = (float)width / height;
	ProjectionMatrix = glm::perspective(45.0f*TO_RADIAN, aspect_ratio, 1.0f, 1500.0f);

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteVertexArrays(1, &rectangle_VAO);
	glDeleteBuffers(1, &rectangle_VBO);

	glDeleteVertexArrays(1, &object_VAO[0]);
	glDeleteBuffers(1, &object_VBO[0]);

	glDeleteTextures(N_NORMAL_TEXTURES_USED, texture_names);
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		glDeleteTextures(1, &ShadowMapping[i].shadow_map_ID);
		glDeleteFramebuffers(1, &ShadowMapping[i].FBO_ID);
	}
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(rotation_speed_tiger, timer_scene, 1);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	int i;
	char string[256];
	ShaderInfo shader_info_simple[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
	{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_shadow[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Shadow.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Shadow.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_SHOW_SM[3] = {
		{ GL_VERTEX_SHADER, "Shaders/SHOW_SM.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/SHOW_SM.frag" },
	{ GL_NONE, NULL }
	};

	//////////////////
	h_ShaderProgram_simple = LoadShaders(shader_info_simple);
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");

	//////////////////
	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");
	loc_ShadowMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ShadowMatrix");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].shadow_on", i);
		loc_light[i].shadow_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_base_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		sprintf(string, "u_shadow_texture[%d]", i);
		loc_shadow_texture[i] = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

	//////////////////
	h_ShaderProgram_shadow = LoadShaders(shader_info_shadow);
	loc_ModelViewProjectionMatrix_simple = glGetUniformLocation(h_ShaderProgram_shadow, "u_ModelViewProjectionMatrix");

	/////////////////
	h_ShaderProgram_SHOW_SM = LoadShaders(shader_info_SHOW_SM);
	loc_ModelViewProjectionMatrix_SHOW_SM = glGetUniformLocation(h_ShaderProgram_SHOW_SM, "u_ModelViewProjectionMatrix");
	loc_shadow_texture_SHOW_SM = glGetUniformLocation(h_ShaderProgram_SHOW_SM, "u_shadow_texture");
	loc_near_clip_dist_SHOW_SM = glGetUniformLocation(h_ShaderProgram_SHOW_SM, "u_near_clip_dist");
	loc_far_clip_dist_SHOW_SM = glGetUniformLocation(h_ShaderProgram_SHOW_SM, "u_far_clip_dist");
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 0.115f, 0.115f, 0.115f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform1i(loc_light[i].shadow_on, 0); // turn off all shadows initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUniform1i(loc_shadow_texture[0], ShadowMapping[0].texture_unit);
	glUniform1i(loc_shadow_texture[1], ShadowMapping[1].texture_unit);
	glUniform1i(loc_shadow_texture[2], ShadowMapping[2].texture_unit);

	glUseProgram(0);
}

void initialize_flags(void) {
	flag.texture_mapping = 0;
	flag.fog = 0;
	flag.show_shadow_map = 0;
	flag.cull_face = 0;
	flag.tiger_move = 1;
	flag.tiger_speed = 1;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag.fog);
	glUniform1i(loc_flag_texture_mapping, flag.texture_mapping);
	glUseProgram(0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#define CAMERA_COORDINATE 400.0f
	ViewMatrix = glm::lookAt(glm::vec3(CAMERA_COORDINATE, CAMERA_COORDINATE, CAMERA_COORDINATE), glm::vec3(0.0f, 30.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	initialize_lights_and_material();
	initialize_flags();

	glGenTextures(N_NORMAL_TEXTURES_USED, texture_names);
}

void set_up_scene_lights(void) {
	int i;
	glm::vec4 position_EC;

#define SF 1.0f
	light0_position_WC[0][0] = -SF * 500.0f;
	light0_position_WC[0][1] = SF * 600.0f;
	light0_position_WC[0][2] = SF * 400.0f;
	light0_position_WC[0][3] = 1.0f;
	light0_position_WC[1][0] = SF * 500.0f;
	light0_position_WC[1][1] = SF * 600.0f;
	light0_position_WC[1][2] = SF * 400.0f;
	light0_position_WC[1][3] = 1.0f;
	light0_lerp_t = 0.0f;

	// point_light_WC: use light 0
	light[0].light_on = 1;
	light[0].shadow_on = 0;
	for (i = 0; i < 4; i++)
		light[0].position[i] = (1.0f - light0_lerp_t)*light0_position_WC[0][i]
		+ light0_lerp_t * light0_position_WC[1][i]; // point light position in WC

	light[0].ambient_color[0] = 0.13f; light[0].ambient_color[1] = 0.13f;
	light[0].ambient_color[2] = 0.13f; light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = 0.5f; light[0].diffuse_color[1] = 0.5f;
	light[0].diffuse_color[2] = 0.5f; light[0].diffuse_color[3] = 1.5f;

	light[0].specular_color[0] = 0.8f; light[0].specular_color[1] = 0.8f;
	light[0].specular_color[2] = 0.8f; light[0].specular_color[3] = 1.0f;

	// spot_light_WC: do not use light 1 initially
	light[1].light_on = 0;
	light[1].shadow_on = 0;
	light[1].position[0] = 400.0f; light[1].position[1] = 550.0f; // spot light position in WC
	light[1].position[2] = -400.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.152f; light[1].ambient_color[1] = 0.152f;
	light[1].ambient_color[2] = 0.152f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.572f; light[1].diffuse_color[1] = 0.572f;
	light[1].diffuse_color[2] = 0.572f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.772f; light[1].specular_color[1] = 0.772f;
	light[1].specular_color[2] = 0.772f; light[1].specular_color[3] = 1.0f;

	light[1].spot_direction[0] = -300.0f; light[1].spot_direction[1] = -500.0f; // spot light direction in WC
	light[1].spot_direction[2] = 400.0f;
	light[1].spot_cutoff_angle = 10.0f;
	light[1].spot_exponent = 8.0f;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform1i(loc_light[0].shadow_on, light[0].shadow_on);
	// need to supply position in EC for shading
	position_EC = ViewMatrix * glm::vec4(light[0].position[0], light[0].position[1], light[0].position[2],
		light[0].position[3]);
	glUniform4fv(loc_light[0].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform1i(loc_light[1].shadow_on, light[1].shadow_on);
	// need to supply position in EC for shading
	position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1], light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	// need to supply direction in EC for shading in this example shader
	// note that the viewing transform is a rigid body transform
	// thus transpose(inverse(mat3(ViewMatrix)) = mat3(ViewMatrix)
	glm::vec3 direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1],
		light[1].spot_direction[2]);
	glUniform3fv(loc_light[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);
	glUseProgram(0);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_tiger();
	prepare_OPTIMUS();
	set_up_scene_lights();
}

void prepare_shadow_mapping(void) {

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {

		ShadowMapping[i].texture_unit = TEXTURE_INDEX_SHADOW + i;//텍스쳐의 아이디. 모든 텍스쳐가 다른 아이디를 가져야 함
		ShadowMapping[i].shadow_map_width = SHADOW_MAP_WIDTH;
		ShadowMapping[i].shadow_map_height = SHADOW_MAP_HEIGHT;
		ShadowMapping[i].near_dist = SHADOW_MAP_NEAR_DIST;
		ShadowMapping[i].far_dist = SHADOW_MAP_FAR_DIST;
		ShadowMapping[i].shadow_map_border_color[0] = 1.0f;
		ShadowMapping[i].shadow_map_border_color[1] = 0.0f;
		ShadowMapping[i].shadow_map_border_color[2] = 0.0f;
		ShadowMapping[i].shadow_map_border_color[3] = 0.0f;

		// Initialize the shadow map
		glGenTextures(1, &ShadowMapping[i].shadow_map_ID);
		glActiveTexture(GL_TEXTURE0 + ShadowMapping[i].texture_unit);
		glBindTexture(GL_TEXTURE_2D, ShadowMapping[i].shadow_map_ID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, ShadowMapping[i].shadow_map_width, ShadowMapping[i].shadow_map_height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ShadowMapping[i].shadow_map_border_color);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS); // or GL_LESS

		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_shadow_texture[i], ShadowMapping[i].texture_unit);
		glUseProgram(0);

		// Initialize the Frame Buffer Object for rendering shadows
		glGenFramebuffers(1, &ShadowMapping[i].FBO_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapping[i].FBO_ID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMapping[i].shadow_map_ID, 0);
		glDrawBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "Error: the framebuffer object for shadow mapping is not complete...\n");
			exit(-1);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ViewMatrix_SHADOW[i] = glm::lookAt(glm::vec3(light[i].position[0], light[i].position[1], light[i].position[2]),
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ProjectionMatrix_SHADOW[i] = glm::perspective(TO_RADIAN*60.0f, 1.0f, ShadowMapping[i].near_dist, ShadowMapping[i].far_dist);
		BiasMatrix = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);

		ModelViewProjectionMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

		glUseProgram(h_ShaderProgram_SHOW_SM);
		glUniform1i(loc_shadow_texture_SHOW_SM, ShadowMapping[i].texture_unit);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SHOW_SM, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform1f(loc_near_clip_dist_SHOW_SM, ShadowMapping[i].near_dist);
		glUniform1f(loc_far_clip_dist_SHOW_SM, ShadowMapping[i].far_dist);
		glUseProgram(0);
	}

}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	prepare_shadow_mapping();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE3170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	// Phong Shading
	char program_name[64] = "Sogang CSE4170 Tiger_Optimus_Shadows_GLSL";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: '0', '1', 't', 'f', 's', 'd', 'r', 'l', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	// glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}