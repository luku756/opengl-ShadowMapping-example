// axes object
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) { // draw coordinate axes
						  // initialize vertex buffer object
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// initialize vertex array object
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


#define NUMBER_OF_OBJECT 8
#define OBJECT_ID_FLOOR 2
#define OBJECT_ID_TIGER 3
#define OBJECT_ID_OPTIMUS 0
#define OBJECT_ID_DRAGON 1

void draw_axes(void) {
	// assume ShaderProgram_simple is used
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

void draw_axes(glm::mat4 mvp, float scale) {
	glUseProgram(h_ShaderProgram_simple);
	//ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(scale, scale, scale));
	//ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewProjectionMatrix = glm::scale(mvp, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_simple, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(3.0f);
	draw_axes();
	glLineWidth(1.0f);
}


// floor object
GLuint rectangle_VBO, rectangle_VAO;
GLfloat rectangle_vertices[6][8] = {  // vertices enumerated counterclockwise
{ 0.0f, 0.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 0.0f },
{ 1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 0.0f },
{ 1.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 1.0f },
{ 0.0f, 0.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 0.0f },
{ 1.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 1.0f },
{ 0.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 1.0f }
};


GLuint quad_VBO, quad_VAO;
GLfloat quad_vertices[6][8] = {  // vertices enumerated counterclockwise
	
{ -1.0f, -1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 0.0f },
{ 1.0f, -1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 0.0f },
{ 1.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 1.0f },
{ -1.0f, -1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 0.0f },
{ 1.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 1.0f, 1.0f },
{ -1.0f, 1.0f, 0.0f,		 0.0f, 0.0f, 1.0f,		 0.0f, 1.0f }
};

Material_Parameters material_floor;

void prepare_floor(void) { // Draw coordinate axes.
						   // Initialize vertex buffer object.
	glGenBuffers(1, &rectangle_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), &rectangle_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &rectangle_VAO);
	glBindVertexArray(rectangle_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, rectangle_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_floor.ambient_color[0] = 0.0f;
	material_floor.ambient_color[1] = 0.05f;
	material_floor.ambient_color[2] = 0.0f;
	material_floor.ambient_color[3] = 1.0f;

	material_floor.diffuse_color[0] = 0.2f;
	material_floor.diffuse_color[1] = 0.5f;
	material_floor.diffuse_color[2] = 0.2f;
	material_floor.diffuse_color[3] = 1.0f;

	material_floor.specular_color[0] = 0.24f;
	material_floor.specular_color[1] = 0.5f;
	material_floor.specular_color[2] = 0.24f;
	material_floor.specular_color[3] = 1.0f;

	material_floor.specular_exponent = 2.5f;

	material_floor.emissive_color[0] = 0.0f;
	material_floor.emissive_color[1] = 0.0f;
	material_floor.emissive_color[2] = 0.0f;
	material_floor.emissive_color[3] = 1.0f;

	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_FLOOR);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_INDEX_FLOOR]);

	My_glTexImage2D_from_file("Data/grass_tex.jpg");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void set_material_floor(void) {
	// assume ShaderProgram_TXPS is used
	glUniform1i(loc_object_id, OBJECT_ID_FLOOR);
	glUniform4fv(loc_material.ambient_color, 1, material_floor.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_floor.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_floor.specular_color);
	glUniform1f(loc_material.specular_exponent, material_floor.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_floor.emissive_color);
}

void draw_floor(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(rectangle_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


void prepare_quad(void) { // Draw coordinate axes.
						   // Initialize vertex buffer object.
	glGenBuffers(1, &quad_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &quad_VAO);
	glBindVertexArray(quad_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//material_floor.ambient_color[0] = 0.0f;
	//material_floor.ambient_color[1] = 0.05f;
	//material_floor.ambient_color[2] = 0.0f;
	//material_floor.ambient_color[3] = 1.0f;

	//material_floor.diffuse_color[0] = 0.2f;
	//material_floor.diffuse_color[1] = 0.5f;
	//material_floor.diffuse_color[2] = 0.2f;
	//material_floor.diffuse_color[3] = 1.0f;

	//material_floor.specular_color[0] = 0.24f;
	//material_floor.specular_color[1] = 0.5f;
	//material_floor.specular_color[2] = 0.24f;
	//material_floor.specular_color[3] = 1.0f;

	//material_floor.specular_exponent = 2.5f;

	//material_floor.emissive_color[0] = 0.0f;
	//material_floor.emissive_color[1] = 0.0f;
	//material_floor.emissive_color[2] = 0.0f;
	//material_floor.emissive_color[3] = 1.0f;

	//glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_FLOOR);
	//glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_INDEX_FLOOR]);

	//My_glTexImage2D_from_file("Data/grass_tex.jpg");

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void draw_quad(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat *tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;

int read_geometry(GLfloat **object, int bytes_per_primitive, const char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void prepare_tiger(void) { // vertices enumerated clockwise
	glUniform1i(loc_object_id, OBJECT_ID_TIGER);
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles*n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.728281f;
	material_tiger.specular_color[1] = 0.655802f;
	material_tiger.specular_color[2] = 0.466065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;

	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_INDEX_TIGER]);

	My_glTexImage2D_from_file("Data/tiger_tex2.jpg");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void set_material_tiger(void) {
	glUniform1i(loc_object_id, OBJECT_ID_TIGER);
	// assume ShaderProgram_TXPS is used
	glUniform4fv(loc_material.ambient_color, 1, material_tiger.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_tiger.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_tiger.specular_color);
	glUniform1f(loc_material.specular_exponent, material_tiger.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_tiger.emissive_color);
}

void draw_tiger(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}



// other objects

#define N_OBJECTS	3 // objects other than the moving tiger
#define OBJECT_OPTIMUS 0
#define OBJECT_DRAGON 1

GLuint object_VBO[N_OBJECTS], object_VAO[N_OBJECTS];
int object_n_triangles[N_OBJECTS];
GLfloat *object_vertices[N_OBJECTS];

Material_Parameters material_object[N_OBJECTS];

void set_up_object(int object_ID, const char *filename, int n_bytes_per_vertex) {
	object_n_triangles[object_ID] = read_geometry(&object_vertices[object_ID],
		3 * n_bytes_per_vertex, filename);
	// Error checking is needed here.

	// Initialize vertex buffer object.
	glGenBuffers(1, &object_VBO[object_ID]);

	glBindBuffer(GL_ARRAY_BUFFER, object_VBO[object_ID]);
	glBufferData(GL_ARRAY_BUFFER, object_n_triangles[object_ID] * 3 * n_bytes_per_vertex,
		object_vertices[object_ID], GL_STATIC_DRAW);

	// As the geometry data exists now in graphics memory, ...
	free(object_vertices[object_ID]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &object_VAO[object_ID]);
	glBindVertexArray(object_VAO[object_ID]);

	glBindBuffer(GL_ARRAY_BUFFER, object_VBO[object_ID]);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(LOC_VERTEX);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(LOC_NORMAL);
	//if (n_bytes_per_vertex == 8 * sizeof(float)) {
		glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(LOC_TEXCOORD);
	//}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void push_obj_mat(int object_ID, Material_Parameters material_object);

void initialize_object_material(int object_ID,
	float a_r, float a_g, float a_b, float a_a,
	float d_r, float d_g, float d_b, float d_a,
	float s_r, float s_g, float s_b, float s_a, float s_e,
	float e_r, float e_g, float e_b, float e_a) {
	material_object[object_ID].ambient_color[0] = a_r;
	material_object[object_ID].ambient_color[1] = a_g;
	material_object[object_ID].ambient_color[2] = a_b;
	material_object[object_ID].ambient_color[3] = a_a;

	material_object[object_ID].diffuse_color[0] = d_r;
	material_object[object_ID].diffuse_color[1] = d_g;
	material_object[object_ID].diffuse_color[2] = d_b;
	material_object[object_ID].diffuse_color[3] = d_a;

	material_object[object_ID].specular_color[0] = s_r;
	material_object[object_ID].specular_color[1] = s_g;
	material_object[object_ID].specular_color[2] = s_b;
	material_object[object_ID].specular_color[3] = s_a;

	material_object[object_ID].specular_exponent = s_e;

	material_object[object_ID].emissive_color[0] = e_r;
	material_object[object_ID].emissive_color[1] = e_g;
	material_object[object_ID].emissive_color[2] = e_b;
	material_object[object_ID].emissive_color[3] = e_a;

	push_obj_mat(object_ID, material_object[object_ID]);
}

void set_material_object(int object_ID) {
	glUniform1i(loc_object_id, object_ID);
	// assume ShaderProgram_PS is used
	glUniform4fv(loc_material.ambient_color, 1, material_object[object_ID].ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_object[object_ID].diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_object[object_ID].specular_color);
	glUniform1f(loc_material.specular_exponent, material_object[object_ID].specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_object[object_ID].emissive_color);
}

void draw_object(int object_ID, float r, float g, float b, unsigned int polygon_orientation) {
	glFrontFace(polygon_orientation);

	glUniform3f(loc_primitive_color, r, g, b);
	glBindVertexArray(object_VAO[object_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * object_n_triangles[object_ID]);
	glBindVertexArray(0);
}

void prepare_OPTIMUS(void) {
	set_up_object(OBJECT_OPTIMUS, "Data/optimus_vnt.geom", 8 * sizeof(float));
	initialize_object_material(OBJECT_OPTIMUS,
		0.1745f, 0.01175f, 0.01175f, 1.0f,
		0.61424f, 0.04136f, 0.04136f, 1.0f,
		0.727811f, 0.626959f, 0.626959f, 1.0f, 76.8f,
		0.1f, 0.1f, 0.1f, 1.0f);
}


void prepare_dragon(void) {
	set_up_object(OBJECT_DRAGON, "Data/dragon_vnt.geom", 8 * sizeof(float));
	initialize_object_material(OBJECT_DRAGON,
		0.329412f, 0.223529f, 0.027451f, 1.0f,
		0.780392f, 0.568627f, 0.113725f, 1.0f,
		0.992157f, 0.941176f, 0.807843f, 1.0f, 76.8f,
		0.0f, 0.0f, 0.0f, 1.0f);
}
