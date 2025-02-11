#define _CRT_SECURE_NO_WARNINGS

#include"header.h"

GLuint aoTex[2];
GLuint depthBuf, posTex, normTex, objIdTex, diffTex;
void setupFBO()
{
	// Create the textures for position, normal and color
	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_POS, GL_RGB32F, posTex);  // Position
	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_NORM, GL_RGB32F, normTex); // Normal
	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_OBJID, GL_RGB32F, objIdTex);  // Color
	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_DIFFUSE, GL_RGB32F, diffTex);  // Color

	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_AO, GL_RGB32F, aoTex[0]);     // AO pair. 16bit float
	createGBufTex(GL_TEXTURE0 + TEXTURE_INDEX_AO, GL_RGB32F, aoTex[1]);

	// Create and bind the FBO
	glGenFramebuffers(1, &deferredFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);

	// The depth buffer
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scene_width, scene_height);

	// Attach the textures to the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, objIdTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, diffTex, 0);

	GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
						GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3, GL_NONE };
	glDrawBuffers(6, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create and bind the FBO
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// Attach the texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoTex[0], 0);

	GLenum drawBuffers2[] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(6, drawBuffers2);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

// for tiger animation
int cur_frame_tiger = 0;
float rotation_angle_tiger = 0.0f;
int rotation_speed_tiger = 100;

#include "Objects.h"

void set_up_dragon_light() {

	int i;
	glm::vec4 position_EC;
	glm::mat4 ModelMatrix;

	ModelMatrix = glm::rotate(glm::mat4(1.0f), rotation_angle_tiger * 2, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-200.0f, 200.0f, 0.0f));

	glm::vec4 pos = glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glm::vec4 position_WC = ModelMatrix * pos;

	ViewMatrix_SHADOW[2] = glm::lookAt(glm::vec3(position_WC.x, position_WC.y, position_WC.z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 dir = glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]);
	glm::vec3 dir_wc = glm::mat3(ModelMatrix) * dir;

	glUseProgram(h_ShaderProgram_TXPS);

	position_EC = ViewMatrix * position_WC;
	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glm::vec3 direction_EC2 = glm::mat3(ViewMatrix) * dir_wc;
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC2[0]);

	glUseProgram(0);
}

void build_shadow_map(int index) { // The first pass in shadow mapping
	glm::mat4 ViewProjectionMatrix_SHADOW;

	ViewProjectionMatrix_SHADOW = ProjectionMatrix_SHADOW * ViewMatrix_SHADOW[index];

	glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapping.FBO_ID[index]);
	glViewport(0, 0, ShadowMapping.shadow_map_width, ShadowMapping.shadow_map_height);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

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

	// for drawing dragon	
	ModelViewProjectionMatrix = glm::rotate(ViewProjectionMatrix_SHADOW, rotation_angle_tiger * 2, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = glm::translate(ModelViewProjectionMatrix, glm::vec3(-200.0f, 200.0f, 0.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_shadow, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_DRAGON, 1.0f, 1.0f, 1.0f, GL_CCW);


	glUseProgram(0);

	glFinish();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_screen() {

	prepare_quad();

	ModelViewMatrix = glm::mat4(1.0f);
	ModelViewMatrixInvTrans = glm::mat3(1.0f);
	ModelViewProjectionMatrix = glm::mat4(1.0f);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	draw_quad();
}

void setModelMatrixes(glm::mat4 ModelMatrix) {

	ModelViewMatrix = ViewMatrix * ModelMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

}

void selectPass(int pass) {
	glUniform1i(loc_PASS, pass);
}

void draw_scene() {

	glm::mat4 ModelMatrix;
	set_material_floor();
	glUniform1i(loc_base_texture, TEXTURE_INDEX_FLOOR);
	

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 500.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	setModelMatrixes(ModelMatrix);
	draw_floor();

	set_material_tiger();
	glUniform1i(loc_base_texture, TEXTURE_INDEX_TIGER);
	ModelMatrix = glm::rotate(glm::mat4(1.0f), -rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(200.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	setModelMatrixes(ModelMatrix);
	draw_tiger();

	// for drawing Optimus
	glUseProgram(h_ShaderProgram_TXPS);

	set_material_object(OBJECT_OPTIMUS);
	ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
	ModelMatrix = glm::rotate(ModelMatrix, rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));


	setModelMatrixes(ModelMatrix);
	draw_object(OBJECT_OPTIMUS, 1.0f, 1.0f, 1.0f, GL_CCW);

	// for drawing dragon
	glUseProgram(h_ShaderProgram_TXPS);
	set_material_object(OBJECT_DRAGON);

	ModelMatrix = glm::rotate(glm::mat4(1.0f), rotation_angle_tiger * 2, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-200.0f, 200.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));


	setModelMatrixes(ModelMatrix);
	draw_object(OBJECT_DRAGON, 1.0f, 1.0f, 1.0f, GL_CCW);
}

void pass1() {

	glViewport(0, 0, WINDOW_param.width, WINDOW_param.height);

	glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	selectPass(1);

	draw_scene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void pass2() {

	selectPass(2);

	
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoTex[0], 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	
	glUniformMatrix4fv(loc_ProjMat, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	draw_screen();

}
void pass3() {

	selectPass(3);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_AO);
	glBindTexture(GL_TEXTURE_2D, aoTex[0]);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aoTex[1], 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	draw_screen();

}
void pass4() {

	selectPass(4);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_AO);
	glBindTexture(GL_TEXTURE_2D, aoTex[1]);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);


	glm::mat4 viewmatInv = glm::inverse(ViewMatrix);
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		ShadowMatrix = BiasMatrix * ProjectionMatrix_SHADOW * ViewMatrix_SHADOW[i] * viewmatInv;
		glUniformMatrix4fv(loc_ShadowMatrix_TXPS[i], 1, GL_FALSE, &ShadowMatrix[0][0]);
	}

	draw_screen();

	glUseProgram(0);
}

void draw_scene_with_shadow(void) { // The second pass in shadow mapping
	
	glUseProgram(h_ShaderProgram_TXPS);
	pass1();//geometry
	pass2();//ssao
	pass3();//blur
	pass4();//light
	glutSwapBuffers();
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
	set_up_dragon_light();
	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++)
		build_shadow_map(i);

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
	int light_flag = -1;
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
	case 'v':
		flag.view_mode++;
		flag.view_mode %= 5;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_view_mode, flag.view_mode);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'a':
		flag.ambient_occlusion = 1 - flag.ambient_occlusion;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_flag_ambient_occlusion, flag.ambient_occlusion);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 's': // Currently, shadow is cast for light 0 only.
		for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
			if (light[i].light_on && light[i].shadow_on) {
				light_flag = 0;
			}
			else if (light[i].light_on && (light[i].shadow_on == 0)) {
				light_flag = 1;
			}
		}
		if (light_flag >= 0) {
			for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {

				light[i].shadow_on = light_flag;
				glUseProgram(h_ShaderProgram_TXPS);
				glUniform1i(loc_light[i].shadow_on, light[i].shadow_on);
				glUseProgram(0);
				glutPostRedisplay();

			}
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
	glDeleteTextures(1, &ShadowMapping.shadow_map_ID[0]);

	glDeleteFramebuffers(1, &ShadowMapping.FBO_ID[0]);
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
			{ GL_VERTEX_SHADER, "Shaders/def_phong.vert.glsl" },
			{ GL_FRAGMENT_SHADER, "Shaders/def_phong.frag.glsl" },
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

	loc_PASS = glGetUniformLocation(h_ShaderProgram_TXPS, "Pass");
	loc_posTex = glGetUniformLocation(h_ShaderProgram_TXPS, "PositionTex");
	loc_normTex = glGetUniformLocation(h_ShaderProgram_TXPS, "NormalTex");

	loc_objIDTex = glGetUniformLocation(h_ShaderProgram_TXPS, "objIDTex");
	loc_diffTex = glGetUniformLocation(h_ShaderProgram_TXPS, "diffTex");
	loc_sampleKernal = glGetUniformLocation(h_ShaderProgram_TXPS, "SampleKernel");

	loc_ao = glGetUniformLocation(h_ShaderProgram_TXPS, "AoTex");
	loc_rand = glGetUniformLocation(h_ShaderProgram_TXPS, "RandTex");
	loc_ProjMat = glGetUniformLocation(h_ShaderProgram_TXPS, "ProjectionMatrix");

	loc_object_id = glGetUniformLocation(h_ShaderProgram_TXPS, "object_id");

	loc_base_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		sprintf(string, "u_ShadowMatrix[%d]", i);
		loc_ShadowMatrix_TXPS[i] = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

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
	for (i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		sprintf(string, "u_shadow_texture[%d]", i);
		loc_shadow_texture[i] = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_ambient_occlusion = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_ambient_occlusion");
	loc_flag_view_mode = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_view_mode");
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

	for (i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		glUniform1i(loc_shadow_texture[i], ShadowMapping.texture_unit[i]);
	}
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

	//ViewMatrix = glm::lookAt(glm::vec3(CAMERA_COORDINATE, CAMERA_COORDINATE, CAMERA_COORDINATE), glm::vec3(0.0f, 30.0f, 0.0f),	glm::vec3(0.0f, 1.0f, 0.0f));


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


	// spot_light_MC:

	light[2].light_on = 0;
	light[2].shadow_on = 0;
	light[2].position[0] = 30; light[2].position[1] = 250.0f; // spot light position in MC
	light[2].position[2] = 00; light[2].position[3] = 1.0f;

	light[2].ambient_color[0] = 0.152f; light[2].ambient_color[1] = 0.152f;
	light[2].ambient_color[2] = 0.152f; light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 0.572f; light[2].diffuse_color[1] = 0.572f;
	light[2].diffuse_color[2] = 0.572f; light[2].diffuse_color[3] = 1.0f;

	light[2].specular_color[0] = 0.772f; light[2].specular_color[1] = 0.772f;
	light[2].specular_color[2] = 0.772f; light[2].specular_color[3] = 1.0f;

	light[2].spot_direction[0] = 300.0f; light[2].spot_direction[1] = -500.0f; // spot light direction in MC
	light[2].spot_direction[2] = 00.0f;
	light[2].spot_cutoff_angle = 20.0f;
	light[2].spot_exponent = 8.0f;


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



	glUniform1i(loc_light[2].light_on, light[2].light_on);
	glUniform1i(loc_light[2].shadow_on, light[2].shadow_on);
	// need to supply position in EC for shading
	position_EC = ViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light[2].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);
	// need to supply direction in EC for shading in this example shader
	// note that the viewing transform is a rigid body transform
	// thus transpose(inverse(mat3(ViewMatrix)) = mat3(ViewMatrix)
	glm::vec3 direction_EC2 = glm::mat3(ViewMatrix) * glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1],
		light[2].spot_direction[2]);
	glUniform3fv(loc_light[2].spot_direction, 1, &direction_EC2[0]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);





	glUseProgram(0);
}

//하드코딩으로 해놧음!!!
Material_Parameters object_mat_list[NUMBER_OF_OBJECT];

 

void initialize_materials() {

	glUseProgram(h_ShaderProgram_TXPS);

	char string[256];
	GLint loc_ambient, loc_diff, loc_spec, loc_emissive, loc_spec_expo, loc;
	float* t;
	int len = 4;//지금 오브젝트가 네개 뿐이니까...
	
	for (int i = 0; i < len; i++) {
		sprintf(string, "object_materials[%d].ambient_color", i);
		loc = glGetUniformLocation(h_ShaderProgram_TXPS, string);

		t = object_mat_list[i].ambient_color;
		glUniform4f(loc, t[0],t[1],t[2],t[3]);


		sprintf(string, "object_materials[%d].diffuse_color", i);
		loc = glGetUniformLocation(h_ShaderProgram_TXPS, string);

		 t = object_mat_list[i].diffuse_color;
		glUniform4f(loc, t[0], t[1], t[2], t[3]);



		sprintf(string, "object_materials[%d].specular_color", i);
		loc = glGetUniformLocation(h_ShaderProgram_TXPS, string);

		t = object_mat_list[i].specular_color;
		glUniform4f(loc, t[0], t[1], t[2], t[3]);


		sprintf(string, "object_materials[%d].emissive_color", i);
		loc = glGetUniformLocation(h_ShaderProgram_TXPS, string);

		t = object_mat_list[i].emissive_color;
		glUniform4f(loc, t[0], t[1], t[2], t[3]);



		sprintf(string, "object_materials[%d].specular_exponent", i);
		loc = glGetUniformLocation(h_ShaderProgram_TXPS, string);

		glUniform1f(loc, object_mat_list[i].specular_exponent);


	}


	glUseProgram(0);
}

//오브젝트의 마테리얼 정보를 하나의 배열로 저장한다.
void push_obj_mat(int object_ID, Material_Parameters material_object) {
	object_mat_list[object_ID] = material_object;
}
class Random {
private:
	std::mt19937 generator;
	std::uniform_real_distribution<float> distr01;

public:
	Random() : distr01(0.0f, 1.0f) {
		std::random_device rd;
		generator.seed(rd());
	}

	float nextFloat() {
		return distr01(generator);
	}

	static void shuffle(std::vector<GLfloat> & v) {
		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(v.begin(), v.end(), g);
	}

	glm::vec3 uniformHemisphere() {
		glm::vec3 result;
		
		result.x = (rand() % 1000) - 500;
		result.y = (rand() % 1000) - 500;
		result.z = rand() % 500;
		result = glm::normalize(result);
		return result;
	}

	glm::vec3 uniformCircle() {
		glm::vec3 result(0.0f);

		result.x = (rand() % 1000) - 500;
		result.y = (rand() % 1000) - 500;
		result.z = 0;
		result = glm::normalize(result);

		return result;
	}
};
Random rander;
GLuint buildRandRotationTex() {
	int size = 4;
	std::vector<GLfloat> randDirections(3 * size * size);
	for (int i = 0; i < size * size; i++) {
		glm::vec3 v = rander.uniformCircle();
		randDirections[i * 3 + 0] = v.x;
		randDirections[i * 3 + 1] = v.y;
		randDirections[i * 3 + 2] = v.z;
	//	std::cout << v.x << ", " << v.y << ", " << v.z << std::endl;
	}
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, size, size);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RGB, GL_FLOAT, randDirections.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	return tex;
}

void buildKernel() {

	int kernSize = 64;
	std::vector<float> kern(3 * kernSize);
	for (int i = 0; i < kernSize; i++) {
		glm::vec3 randDir = rander.uniformHemisphere();

		float scale = ((float)(i * i)) / (kernSize * kernSize);
		randDir *= glm::mix(0.1f, 1.0f, scale);

		kern[i * 3 + 0] = randDir.x;
		kern[i * 3 + 1] = randDir.y;
		kern[i * 3 + 2] = randDir.z;
	}
	glUseProgram(h_ShaderProgram_TXPS);
	GLint loc = glGetUniformLocation(h_ShaderProgram_TXPS, "SampleKernel");
	glUniform3fv(loc, kernSize, kern.data());

}

GLuint LoadTex(const char* name, int width, int height) {

	FILE *fp;
	fp = fopen(name, "rb");
	float r, g, b;
	char* c;
	char buf[1000];

	if ((c = fgets(buf, 1000, fp)) != NULL)
		printf("The string is %s\n", c);

//	int size = 4;
	std::vector<GLfloat> vecs(3 * width * height);
	int i=0;
	do {
		c = fgets(buf, 1000, fp);
		if (c != NULL) {
			
			sscanf(c, "%f,%f,%f", &r, &g, &b);
			vecs[i * 3 + 0] = r;
			vecs[i * 3 + 1] = g;
			vecs[i * 3 + 2] = b;
			//printf("The string is %s\n", c);
			//res = fscanf(fp,"%f,%f,%f",&r,&g,&b);
			//printf("%f %f %f\n", r, g, b);
			i++;
		}

	} while (c != NULL);


	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, vecs.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	return tex;
}

void prepare_scene(void) {
	prepare_axes();
	prepare_floor();
	prepare_tiger();
	prepare_OPTIMUS();
	prepare_dragon();

	push_obj_mat(OBJECT_ID_FLOOR, material_floor);
	push_obj_mat(OBJECT_ID_TIGER, material_tiger);

	srand(time(NULL));
	set_up_scene_lights();
	initialize_materials();
	
}

void prepare_shadow_mapping(int index) {

	ShadowMapping.texture_unit[index] = TEXTURE_INDEX_SHADOW + index;
	ShadowMapping.shadow_map_width = SHADOW_MAP_WIDTH;
	ShadowMapping.shadow_map_height = SHADOW_MAP_HEIGHT;
	ShadowMapping.near_dist = SHADOW_MAP_NEAR_DIST;
	ShadowMapping.far_dist = SHADOW_MAP_FAR_DIST;
	ShadowMapping.shadow_map_border_color[0] = 1.0f;
	ShadowMapping.shadow_map_border_color[1] = 0.0f;
	ShadowMapping.shadow_map_border_color[2] = 0.0f;
	ShadowMapping.shadow_map_border_color[3] = 0.0f;

	glGenTextures(1, &ShadowMapping.shadow_map_ID[index]);

	glActiveTexture(GL_TEXTURE0 + ShadowMapping.texture_unit[index]);
	glBindTexture(GL_TEXTURE_2D, ShadowMapping.shadow_map_ID[index]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, ShadowMapping.shadow_map_width, ShadowMapping.shadow_map_height);

	// Initialize the shadow map
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ShadowMapping.shadow_map_border_color);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS); // or GL_LESS

	//변수 설정

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_shadow_texture[index], ShadowMapping.texture_unit[index]);
	glUseProgram(0);

	// Initialize the Frame Buffer Object for rendering shadows
	glGenFramebuffers(1, &ShadowMapping.FBO_ID[index]);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowMapping.FBO_ID[index]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMapping.shadow_map_ID[index], 0);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Error: the framebuffer object for shadow mapping is not complete...\n");
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ViewMatrix_SHADOW[index] = glm::lookAt(glm::vec3(light[index].position[0], light[index].position[1], light[index].position[2]),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	if (index == 0) {

		ProjectionMatrix_SHADOW = glm::perspective(TO_RADIAN*60.0f, 1.0f, ShadowMapping.near_dist, ShadowMapping.far_dist);
		BiasMatrix = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);

		ModelViewProjectionMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

		glUseProgram(h_ShaderProgram_SHOW_SM);
		glUniform1i(loc_shadow_texture_SHOW_SM, ShadowMapping.texture_unit[0]);
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SHOW_SM, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniform1f(loc_near_clip_dist_SHOW_SM, ShadowMapping.near_dist);
		glUniform1f(loc_far_clip_dist_SHOW_SM, ShadowMapping.far_dist);
		glUseProgram(0);
	}

}



//screen space ambient occlusion
void initialize_ssao() {//build kernel

	// Create and assign the random sample kernel
	buildKernel();
	
	//// Create the texture of random rotation directions
	GLuint rotTex = buildRandRotationTex();
	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_RAND);
	glBindTexture(GL_TEXTURE_2D, rotTex);


}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();

	for (int i = 0; i < NUMBER_OF_LIGHT_COUNT; i++) {
		prepare_shadow_mapping(i);
	}
	initialize_ssao();
	setupFBO();

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
	glutInitWindowSize(scene_width, scene_height);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}