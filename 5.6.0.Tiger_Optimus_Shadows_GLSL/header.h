#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>
#include <vector>
#include <time.h>
#include <random>
#include <iostream>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"


int scene_width = 1200, scene_height = 800;

GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_shadow, h_ShaderProgram_SHOW_SM;

// for simple shaders
GLint loc_ModelViewProjectionMatrix_simple, loc_primitive_color;

// for Phong Shading shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
#define NUMBER_OF_LIGHT_COUNT 4
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_base_texture, loc_flag_texture_mapping, loc_flag_fog, loc_flag_ambient_occlusion, loc_flag_view_mode;
GLint loc_ShadowMatrix_TXPS[NUMBER_OF_LIGHT_COUNT], loc_shadow_texture[NUMBER_OF_LIGHT_COUNT];
GLint loc_shadow_texture_SHOW_SM;
GLint loc_ViewInvMatrix_TXPS;

GLint loc_object_id;	//물체의 오브젝트 아이디.

std::vector<int> object_list;

// for shadow shaders
GLint loc_ModelViewProjectionMatrix_shadow;

// for drawing shadow map
GLint loc_ModelViewProjectionMatrix_SHOW_SM;
GLint loc_near_clip_dist_SHOW_SM, loc_far_clip_dist_SHOW_SM;


//for deffered and ssao
GLint loc_PASS;
GLint loc_posTex, loc_normTex, loc_objIDTex, loc_diffTex;
GLint loc_sampleKernal, loc_ao, loc_rand, loc_ProjMat;


// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.
glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ViewMatrix, ProjectionMatrix;
glm::mat4 ShadowMatrix, BiasMatrix, ViewMatrix_SHADOW[NUMBER_OF_LIGHT_COUNT], ProjectionMatrix_SHADOW;

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
	int ambient_occlusion = 0;
	int view_mode = 0;
} flag;

// for shadow mapping
struct _ShadowMapping {
	GLint texture_unit[NUMBER_OF_LIGHT_COUNT]; // Must be equal to N_NORMAL_TEXTURES_USED
	GLuint shadow_map_ID[NUMBER_OF_LIGHT_COUNT];
	GLsizei shadow_map_width, shadow_map_height;
	GLfloat near_dist, far_dist, shadow_map_border_color[4];
	GLuint FBO_ID[NUMBER_OF_LIGHT_COUNT];
} ShadowMapping;

struct _WINDOW_param {
	int width, height;
} WINDOW_param;

// texture stuffs
#define N_NORMAL_TEXTURES_USED 5

#define TEXTURE_INDEX_FLOOR 0
#define TEXTURE_INDEX_TIGER 1
//쉐도우 맵이 2~5
#define TEXTURE_INDEX_SHADOW 2
#define TEXTURE_INDEX_POS 7
#define TEXTURE_INDEX_NORM 8
#define TEXTURE_INDEX_OBJID 9
#define TEXTURE_INDEX_DIFFUSE 10
#define TEXTURE_INDEX_RAND	 11
#define TEXTURE_INDEX_AO	 12

#define TEXTURE_INDEX_TEST_POS 25
#define TEXTURE_INDEX_TEST_NORMAL 26
#define TEXTURE_INDEX_TEST_RAND 27

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048
#define SHADOW_MAP_NEAR_DIST 200.0f
#define SHADOW_MAP_FAR_DIST 1500.0f

GLuint texture_names[N_NORMAL_TEXTURES_USED];


GLuint deferredFBO;
GLuint ssaoFBO;


//텍스쳐를 생성, texid으로 주어진 번호에 바인드한다.
//texUnit - GL_TEXTURE0 번호로 쉐이더에 binding.
void createGBufTex(GLenum texUnit, GLenum format, GLuint &texid) {
	glActiveTexture(texUnit);
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	glTexStorage2D(GL_TEXTURE_2D, 1, format, scene_width, scene_height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}



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
