//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 13
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture;
GLint loc_flag_texture_mapping;
GLint loc_flag_fog;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> 
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 ModelMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

/* flags */
int camera_move_mouse = 0;
int camera_rotation_mouse = 0;
int camera_rotation_axis_u = 0;
int camera_rotation_axis_v = 0;
int camera_rotation_axis_n = 0;
unsigned int timestamp_tiger = 0;
unsigned int timestamp_nod;
int tiger_follow_flag = 0;

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_U,
	CAMERA_I,
	CAMERA_O,
	CAMERA_P,
	CAMERA_A,
	CAMERA_T,
	CAMERA_G,
	CAMERA_B,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;
int cur_cam_num = 0;
glm::mat4 ModelMatrix_tiger, ModelMatrix_tiger_eye;
glm::mat4 ModelMatrix_follow_tiger, ModelMatrix_back_of_tiger;
int tiger_eye_flag = 0;
int tiger_nod_flag = 0;
int tiger_back_flag = 0;

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) { //viewMatrix 설정만
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

void set_ViewMatrix_tiger_eye_20171694(void) {
	glm::mat4 Matrix_CAMERA_T_inverse;
	glm::mat4 ModelMatrix_tiger_nod = glm::mat4(1.0f);

	if (tiger_nod_flag == 1) {
		ModelMatrix_tiger_nod = glm::rotate(glm::mat4(1.0f),
			sinf(float(timestamp_nod) * 1.5f * TO_RADIAN) * 0.3f, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	Matrix_CAMERA_T_inverse = ModelMatrix_tiger * ModelMatrix_tiger_eye * ModelMatrix_tiger_nod;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_T_inverse);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void set_ViewMatrix_follow_tiger_20171694(void) {
	glm::mat4 Matrix_CAMERA_T_inverse;

	Matrix_CAMERA_T_inverse = ModelMatrix_tiger * ModelMatrix_follow_tiger;;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_T_inverse);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void set_ViewMatrix_back_of_tiger_20171694(void) {
	glm::mat4 Matrix_CAMERA_T_inverse;

	Matrix_CAMERA_T_inverse = ModelMatrix_tiger * ModelMatrix_back_of_tiger;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_T_inverse);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void set_current_camera(int camera_num) { //입력받은 카메라 번호대로 current_camera에 복사
	Camera* pCamera = &camera_info[camera_num]; 

	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) { //2-5
	/*CAMERA_1: original view
	Camera* pCamera = &camera_info[CAMERA_1];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;*/

	Camera* pCamera = &camera_info[CAMERA_U];
	//CAMERA_U : 탱크쪽에서 전체 사원 전경
	pCamera = &camera_info[CAMERA_U];
	pCamera->pos[0] = -2331.4365f; pCamera->pos[1] = 6300.6997f; pCamera->pos[2] = 5079.6177f;
	pCamera->uaxis[0] = -0.9874f; pCamera->uaxis[1] = -0.1543f; pCamera->uaxis[2] = -0.0355f;
	pCamera->vaxis[0] = 0.0540f; pCamera->vaxis[1] = -0.5388f; pCamera->vaxis[2] = 0.8407f;
	pCamera->naxis[0] = -0.1488f; pCamera->naxis[1] = 0.8282; pCamera->naxis[2] = 0.5403f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_I : 원형 홀의 위쪽 창문을 통해 바라본 내부
	pCamera = &camera_info[CAMERA_I];
	pCamera->pos[0] = -408.3369f; pCamera->pos[1] = -435.0943f; pCamera->pos[2] = 1653.1161f; 
	pCamera->uaxis[0] = 0.7930f; pCamera->uaxis[1] = -0.6026f; pCamera->uaxis[2] = -0.0899f;
	pCamera->vaxis[0] = 0.5238f; pCamera->vaxis[1] = 0.5990f; pCamera->vaxis[2] = 0.6057f;
	pCamera->naxis[0] = -0.3111f; pCamera->naxis[1] = -0.5274f; pCamera->naxis[2] = 0.7901f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_O : 사각형 홀의 계단 위쪽에서 바라본 내부
	pCamera = &camera_info[CAMERA_O];
	pCamera->pos[0] = -660.2274f; pCamera->pos[1] = -1747.5459f; pCamera->pos[2] = 804.9532f; 
	pCamera->uaxis[0] = -0.8792f; pCamera->uaxis[1] = -0.4765f; pCamera->uaxis[2] = 0.0042f;
	pCamera->vaxis[0] = 0.1466f; pCamera->vaxis[1] = -0.2621f; pCamera->vaxis[2] = 0.9537f;
	pCamera->naxis[0] = -0.4534f; pCamera->naxis[1] = 0.8392f; pCamera->naxis[2] = 0.3002f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_P: 복도형 홀의 한쪽 동상 위에서 복도를 바라본다.
	pCamera = &camera_info[CAMERA_P];
	pCamera->pos[0] = -460.8753f; pCamera->pos[1] = -4978.0728f; pCamera->pos[2] = 642.1788f;
	pCamera->uaxis[0] = -0.0036f; pCamera->uaxis[1] = -1.0000f; pCamera->uaxis[2] = 0.0012f;
	pCamera->vaxis[0] = 0.6250f; pCamera->vaxis[1] = -0.0013f; pCamera->vaxis[2] = 0.7806f;
	pCamera->naxis[0] = -0.7806f; pCamera->naxis[1] = 0.0036f; pCamera->naxis[2] = 0.6250f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_A : 사원 외부에서 사각형 홀의 외벽을 바라본다.
	pCamera = &camera_info[CAMERA_A];
	pCamera->pos[0] = -3542.3547f; pCamera->pos[1] = -4789.5693f; pCamera->pos[2] = 3536.6978f; 
	pCamera->uaxis[0] = 0.2677f; pCamera->uaxis[1] = -0.9447f; pCamera->uaxis[2] = -0.1896f;
	pCamera->vaxis[0] = 0.6684f; pCamera->vaxis[1] = 0.0403f; pCamera->vaxis[2] = 0.7427f;
	pCamera->naxis[0] = -0.6940f; pCamera->naxis[1] = -0.3255f; pCamera->naxis[2] = 0.6422f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	pCamera = &camera_info[CAMERA_T];
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;
	pCamera = &camera_info[CAMERA_G];
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;
	pCamera = &camera_info[CAMERA_B];
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	set_current_camera(CAMERA_U);
	cur_cam_num = 0;
}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) { //2-2
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_TXPS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
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

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//sun_temple
GLuint* sun_temple_VBO;
GLuint* sun_temple_VAO;
int* sun_temple_n_triangles;
int* sun_temple_vertex_offset;
GLfloat** sun_temple_vertices;
GLuint* sun_temple_texture_names;

int flag_fog;
bool* flag_texture_mapping;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 1.0f, 1.0f, 1.0f, 1.0f);

	for (i = 0; i < scene.n_lights; i++) {
		glUniform1i(loc_light[i].light_on, 1);
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			1.0f);

		glUniform4f(loc_light[i].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 0.5f, 0.5f, 0.5f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 20.0f, 0.0f, 0.0f, 1.0f); // .w != 0.0f for no ligth attenuation
	}

	glUseProgram(0);
}

void initialize_flags(void) {
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUseProgram(0);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		//fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_sun_temple(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	sun_temple_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	sun_temple_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	sun_temple_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	sun_temple_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	sun_temple_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		sun_temple_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		sun_temple_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			sun_temple_vertex_offset[materialIdx] = 0;
		else
			sun_temple_vertex_offset[materialIdx] = sun_temple_vertex_offset[materialIdx - 1] + 3 * sun_temple_n_triangles[materialIdx - 1];

		glGenBuffers(1, &sun_temple_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, sun_temple_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			sun_temple_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(sun_temple_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &sun_temple_VAO[materialIdx]);
		glBindVertexArray(sun_temple_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", scene.n_materials);

	// textures
	sun_temple_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, sun_temple_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n\n");
	
	free(sun_temple_vertices);
}

void draw_sun_temple(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);

		int texId = scene.material_list[materialIdx].diffuseTexId;
		glUniform1i(loc_texture, texId);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping[texId]);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		glBindVertexArray(sun_temple_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * sun_temple_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

// TO DO
#define LOC_VERTEX 0
unsigned int timestamp_scene = 0; // the global clock in the scene
int flag_tiger_animation, tiger_stop_flag = 0;
int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0;
float rotation_angle_tiger = 0.0f;

// tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

// ben object
#define N_BEN_FRAMES 30
GLuint ben_VBO, ben_VAO;
int ben_n_triangles[N_BEN_FRAMES];
int ben_vertex_offset[N_BEN_FRAMES];
GLfloat* ben_vertices[N_BEN_FRAMES];
unsigned int timestamp_ben = 0;
float rotation_angle_ben = 0.0f;

// wolf object
#define N_WOLF_FRAMES 17
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];
GLfloat* wolf_vertices[N_WOLF_FRAMES];
unsigned int timestamp_wolf = 0;
float rotation_angle_wolf = 0.0f;
/*-------------------------------------------*/
// optimus object
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;

// cow object
GLuint cow_VBO, cow_VAO;
int cow_n_triangles;
GLfloat* cow_vertices;

// bike object
GLuint bike_VBO, bike_VAO;
int bike_n_triangles;
GLfloat* bike_vertices;

// bus object
GLuint bus_VBO, bus_VAO;
int bus_n_triangles;
GLfloat* bus_vertices;

// tank object
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;


int read_geometry_20171694(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	//fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void prepare_optimus_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/optimus_vnt.geom");
	optimus_n_triangles = read_geometry_20171694(&optimus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	optimus_n_total_triangles += optimus_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &optimus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glBufferData(GL_ARRAY_BUFFER, optimus_n_total_triangles * 3 * n_bytes_per_vertex, optimus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(optimus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &optimus_VAO);
	glBindVertexArray(optimus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void prepare_cow_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, cow_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cow_vn.geom");
	cow_n_triangles = read_geometry_20171694(&cow_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cow_n_total_triangles += cow_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &cow_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glBufferData(GL_ARRAY_BUFFER, cow_n_total_triangles * 3 * n_bytes_per_vertex, cow_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cow_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cow_VAO);
	glBindVertexArray(cow_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void prepare_bike_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, bike_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bike_vnt.geom");
	bike_n_triangles = read_geometry_20171694(&bike_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bike_n_total_triangles += bike_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &bike_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glBufferData(GL_ARRAY_BUFFER, bike_n_total_triangles * 3 * n_bytes_per_vertex, bike_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bike_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bike_VAO);
	glBindVertexArray(bike_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void prepare_bus_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, bus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bus_vnt.geom");
	bus_n_triangles = read_geometry_20171694(&bus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bus_n_total_triangles += bus_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &bus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glBufferData(GL_ARRAY_BUFFER, bus_n_total_triangles * 3 * n_bytes_per_vertex, bus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bus_VAO);
	glBindVertexArray(bus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void prepare_tank_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry_20171694(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
/*-------------------------------------------------------*/
void prepare_wolf_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, wolf_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_WOLF_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom", i);
		wolf_n_triangles[i] = read_geometry_20171694(&wolf_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		wolf_n_total_triangles += wolf_n_triangles[i];

		if (i == 0)
			wolf_vertex_offset[i] = 0;
		else
			wolf_vertex_offset[i] = wolf_vertex_offset[i - 1] + 3 * wolf_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &wolf_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glBufferData(GL_ARRAY_BUFFER, wolf_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_WOLF_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, wolf_vertex_offset[i] * n_bytes_per_vertex,
			wolf_n_triangles[i] * n_bytes_per_triangle, wolf_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_WOLF_FRAMES; i++)
		free(wolf_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &wolf_VAO);
	glBindVertexArray(wolf_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void prepare_ben_20171694(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ben_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_BEN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/ben/ben_vn%d%d.geom", i / 10, i % 10);
		ben_n_triangles[i] = read_geometry_20171694(&ben_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		ben_n_total_triangles += ben_n_triangles[i];

		if (i == 0)
			ben_vertex_offset[i] = 0;
		else
			ben_vertex_offset[i] = ben_vertex_offset[i - 1] + 3 * ben_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &ben_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glBufferData(GL_ARRAY_BUFFER, ben_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_BEN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, ben_vertex_offset[i] * n_bytes_per_vertex,
			ben_n_triangles[i] * n_bytes_per_triangle, ben_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_BEN_FRAMES; i++)
		free(ben_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &ben_VAO);
	glBindVertexArray(ben_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void prepare_tiger_20171694(void) { // vertices enumerated clockwise
	int n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	int n_bytes_per_triangle = 3 * n_bytes_per_vertex;
	int tiger_n_total_triangles = 0;
	char filename[512];
	int i;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry_20171694(&tiger_vertices[i], n_bytes_per_triangle, filename);
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
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);
	}

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_wolf_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 1.0f);
	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);
}

void draw_ben_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 1.0f);
	glBindVertexArray(ben_VAO);
	glDrawArrays(GL_TRIANGLES, ben_vertex_offset[cur_frame_ben], 3 * ben_n_triangles[cur_frame_ben]);
	glBindVertexArray(0);
}

void draw_tiger_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);
	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

void draw_optimus_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 153.0f / 255.0f, 51.0f / 255.0f, 0.0f / 255.0f);
	glBindVertexArray(optimus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);
}
void draw_cow_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 0.0f, 1.0f, 0.0f);
	glBindVertexArray(cow_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cow_n_triangles);
	glBindVertexArray(0);
}
void draw_bike_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 1.0f, 0.0f);
	glBindVertexArray(bike_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bike_n_triangles);
	glBindVertexArray(0);
}
void draw_bus_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 255.0f / 255.0f, 102.0f / 255.0f, 153.0f / 255.0f);
	glBindVertexArray(bus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bus_n_triangles);
	glBindVertexArray(0);
}
void draw_tank_20171694(void) {
	//glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 51.0f / 255.0f, 204.0f / 255.0f, 255.0f / 255.0f);
	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);
}
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
int tiger_mov_loc = 0;
void timer_20171694(int value) {
	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;
	if (tiger_stop_flag == 0) {
		timestamp_tiger = (timestamp_tiger + 1) % 4200;
		tiger_mov_loc = timestamp_tiger * 3; printf("timestamp_tiger : %d\n", timestamp_tiger);
	}
	if (tiger_nod_flag == 1) {
		timestamp_nod = (timestamp_nod + 1) % UINT_MAX;
	}

	rotation_angle_tiger = (timestamp_tiger % 360) * TO_RADIAN;
	cur_frame_tiger = (timestamp_tiger / 4) % N_TIGER_FRAMES;
	cur_frame_ben = timestamp_scene % N_BEN_FRAMES;
	cur_frame_wolf = (timestamp_scene / 3) % N_WOLF_FRAMES;

	//
	timestamp_ben = (timestamp_ben + 1) % UINT_MAX;
	timestamp_wolf = (timestamp_wolf + 1) % UINT_MAX;

	rotation_angle_ben = (timestamp_ben % 360) * TO_RADIAN;
	rotation_angle_wolf = (timestamp_wolf % 360) * TO_RADIAN;

	glutPostRedisplay();
	glutTimerFunc(10, timer_20171694, 0);
}

float tiger_loc_x = 0.0f, tiger_loc_y = 0.0f, tiger_loc_z = 0.0f;
float tiger_loc_x_prev, tiger_loc_x_prev2, tiger_loc_y_prev, tiger_loc_y_prev2, tiger_loc_z_prev, tiger_loc_z_prev2;
int tiger_mov_loc_prev, tiger_mov_loc_prev2;
float rotation_angle_tiger_prev, tiger_r_x, tiger_r_y, tiger_r_z;
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(h_ShaderProgram_simple);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//tiger
	/*ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 200.0f));
	ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -rotation_angle_tiger, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(500.0f, 0.0f, 0.0f));*/
	ModelMatrix_tiger = glm::mat4(1.0f);
	if (timestamp_tiger < 950) { //직진
		tiger_loc_y = float(tiger_mov_loc); tiger_loc_y_prev = tiger_loc_y;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, 0.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		//tiger_mov_loc_prev = tiger_mov_loc;
	}
	if (timestamp_tiger >= 950 && timestamp_tiger < 1100) { //좌회전
		tiger_loc_x = -(float(tiger_mov_loc) - tiger_loc_y); tiger_loc_x_prev = tiger_loc_x;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, 0.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev = tiger_mov_loc;
	}
	if (timestamp_tiger >= 1100 && timestamp_tiger < 1650) { //직진
		tiger_loc_y = tiger_loc_y_prev + float(tiger_mov_loc) - float(tiger_mov_loc_prev);
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, 0.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		tiger_loc_z_prev = float(tiger_mov_loc);
	}
	if (timestamp_tiger >= 1650 && timestamp_tiger < 1810) { //계단 오르기
		tiger_loc_y = tiger_loc_y_prev + float(tiger_mov_loc) - float(tiger_mov_loc_prev); tiger_loc_y_prev2 = tiger_loc_y;
		tiger_loc_z = (float(tiger_mov_loc) - float(tiger_loc_z_prev)) * 0.5f; tiger_loc_z_prev2 = tiger_loc_z;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 30.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		tiger_mov_loc_prev2 = tiger_mov_loc;
	}
	if (timestamp_tiger >= 1810 && timestamp_tiger < 2200) { //계단 위에서
		tiger_loc_x = tiger_loc_x_prev + float(tiger_mov_loc) - float(tiger_mov_loc_prev2); tiger_loc_x_prev2 = tiger_loc_x;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev = tiger_mov_loc; 
	}
	if (timestamp_tiger >= 2200 && timestamp_tiger < 2360) { //계단 내려가기
		tiger_loc_y = tiger_loc_y_prev2 - (float(tiger_mov_loc) - float(tiger_mov_loc_prev)); tiger_loc_y_prev = tiger_loc_y;
		tiger_loc_z = tiger_loc_z_prev2 - (float(tiger_mov_loc) - float(tiger_mov_loc_prev)) * 0.5f;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 30.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		printf("(%f, %f, %f)\n", tiger_loc_x, tiger_loc_y, tiger_loc_z);
	}
	if (timestamp_tiger >= 2360 && timestamp_tiger < 2720) { //회전
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(720.0f, 4497.0f, 0.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, float(timestamp_tiger - 2360) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev = tiger_mov_loc;
	}
	if (timestamp_tiger >= 2720 && timestamp_tiger < 3310) { //후진
		tiger_loc_y = tiger_loc_y_prev - (float(tiger_mov_loc) - float(tiger_mov_loc_prev)); tiger_loc_y_prev2 = tiger_loc_y;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev2 = tiger_mov_loc;
	}
	if (timestamp_tiger >= 3310 && timestamp_tiger < 3470) { //우회전
		tiger_loc_x = tiger_loc_x_prev2 - (float(tiger_mov_loc) - float(tiger_mov_loc_prev2)); tiger_loc_x_prev = tiger_loc_x;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev = tiger_mov_loc;
	}
	if (timestamp_tiger >= 3470 && timestamp_tiger < 4120) { //후진
		tiger_loc_y = tiger_loc_y_prev2 - (float(tiger_mov_loc) - float(tiger_mov_loc_prev));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, 0.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		tiger_mov_loc_prev2 = tiger_mov_loc;
	}
	if (timestamp_tiger >= 4120 && timestamp_tiger < 4210) { //제자리로
		tiger_loc_x = tiger_loc_x_prev - (float(tiger_mov_loc) - float(tiger_mov_loc_prev2));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(tiger_loc_x, tiger_loc_y, tiger_loc_z));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-120.0f, -7000.0f, 50.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_tiger = glm::scale(ModelMatrix_tiger, glm::vec3(0.8f, 0.8f, 0.8f));
	ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix_tiger;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger_20171694();

	//tiger_eye
	if ((tiger_eye_flag == 1) && (tiger_nod_flag == 1)) {
		set_ViewMatrix_tiger_eye_20171694();
	}
	//follow tiger
	if (tiger_follow_flag == 1) {
		set_ViewMatrix_follow_tiger_20171694();
	}
	if (tiger_back_flag == 1) {
		set_ViewMatrix_back_of_tiger_20171694();
	}

	//ben
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 200.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -rotation_angle_ben, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_ben * 4.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(500.0f, 500.0f, 500.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ben_20171694();

	//wolf
	ModelViewMatrix = ViewMatrix;
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, -2800.0f, 1650.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -rotation_angle_wolf, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(700.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(500.0f, 500.0f, 500.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wolf_20171694();
	/*-----------------------------------------------*/
	//optimus
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(800.0f, 0.0f, 200.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_optimus_20171694();

	//cow
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -5000.0f, 100.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(250.0f, 250.0f, 250.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cow_20171694();

	//bike
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -3500.0f, 20.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_bike_20171694();

	//bus
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-1800.0f, -2100.0f, 200.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(40.0f, 40.0f, 40.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_bus_20171694();

	//tank
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(900.0f, -4200.0f, 1200.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tank_20171694();


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	/*--------------------------------------------------------*/
	draw_grid();
	draw_axes();
	draw_sun_temple();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	//Camera* pCamera = &camera_info[CAMERA_A]; //
	Camera* pCamera = &camera_info[cur_cam_num];

	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	
	case 'u':
		tiger_eye_flag = 0; tiger_nod_flag = 0; 
		tiger_follow_flag = 0; tiger_back_flag = 0;
		cur_cam_num = 0; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		set_current_camera(CAMERA_U);
		glutPostRedisplay();
		break;
	case 'i':
		tiger_eye_flag = 0; tiger_nod_flag = 0; 
		tiger_follow_flag = 0; tiger_back_flag = 0;
		cur_cam_num = 1; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		set_current_camera(CAMERA_I);
		glutPostRedisplay();
		break;
	case 'o':
		tiger_eye_flag = 0; tiger_nod_flag = 0; 
		tiger_follow_flag = 0; tiger_back_flag = 0;
		cur_cam_num = 2; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		set_current_camera(CAMERA_O);
		glutPostRedisplay();
		break;
	case 'p':
		tiger_eye_flag = 0; tiger_nod_flag = 0; 
		tiger_follow_flag = 0; tiger_back_flag = 0;
		cur_cam_num = 3; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		set_current_camera(CAMERA_P);
		glutPostRedisplay();
		break;
	case 'a':
		tiger_eye_flag = 0; tiger_nod_flag = 0; 
		tiger_follow_flag = 0; tiger_back_flag = 0;
		cur_cam_num = 4; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		set_current_camera(CAMERA_A);
		pCamera->move = 1;
		glutPostRedisplay();
		break;
	case 'x':
		if (camera_rotation_axis_u == 0) {
			camera_rotation_axis_u = 1;
		}
		else {
			camera_rotation_axis_u = 0;
		}
		break;
	case 'y':
		if (camera_rotation_axis_v == 0) {
			camera_rotation_axis_v = 1;
		}
		else {
			camera_rotation_axis_v = 0;
		}
		break;
	case 'z':
		if (camera_rotation_axis_n == 0) {
			camera_rotation_axis_n = 1;
		}
		else {
			camera_rotation_axis_n = 0;
		}
		break;
	case 's': //tiger stop key
		if (tiger_stop_flag == 0) {
			fprintf(stdout, "Tiger animation mode >>>ON<<<\n");
			tiger_stop_flag = 1; tiger_nod_flag = 0;
		}
		else {
			fprintf(stdout, "Tiger animation mode >>>OFF<<<\n");
			tiger_stop_flag = 0; tiger_nod_flag = 1;
		}
		break;
	case 't': //tiger_eye CAMERA
		tiger_follow_flag = 0; tiger_back_flag = 0;
		if (tiger_eye_flag == 0) {
			fprintf(stdout, "Tiger eye mode >>>ON<<<\n");
			tiger_eye_flag = 1;
			tiger_nod_flag = 1;
			//cur_cam_num = 5; pCamera = &camera_info[cur_cam_num];
			ProjectionMatrix = glm::perspective(pCamera->fovy, scene.camera.aspect, 0.1f, 50000.0f);
			glutPostRedisplay();
		}
		else {
			fprintf(stdout, "Tiger eye mode >>>OFF<<<\n");
			tiger_eye_flag = 0;
			tiger_nod_flag = 0;
			tiger_follow_flag = 1;
			//cur_cam_num = 6; pCamera = &camera_info[cur_cam_num];
			ProjectionMatrix = glm::perspective(pCamera->fovy, scene.camera.aspect, 0.1f, 50000.0f);
			glutPostRedisplay();
		}
		break;
	case 'g': //tiger 쫓아가는 CAMERA
		tiger_eye_flag = 0; tiger_nod_flag = 0;
		tiger_follow_flag = 1; tiger_back_flag = 0;
		//cur_cam_num = 6; pCamera = &camera_info[cur_cam_num];
		ProjectionMatrix = glm::perspective(pCamera->fovy, scene.camera.aspect, 0.1f, 50000.0f);
		glutPostRedisplay();
		break;
	case 'b':
		if (tiger_back_flag == 0) {
			tiger_back_flag = 1;
			//cur_cam_num = 7; pCamera = &camera_info[cur_cam_num];
			ProjectionMatrix = glm::perspective(pCamera->fovy, scene.camera.aspect, 0.1f, 50000.0f);
			glutPostRedisplay();
		}
		else {
			tiger_back_flag = 0;
			tiger_follow_flag = 1;
			//cur_cam_num = 6; pCamera = &camera_info[cur_cam_num];
			ProjectionMatrix = glm::perspective(pCamera->fovy, scene.camera.aspect, 0.1f, 50000.0f);
			glutPostRedisplay();
		}
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void mousewheel_20171694(int wheel, int direction, int x, int y) {
	Camera* pCamera = &camera_info[cur_cam_num];

	if ((glutGetModifiers() == GLUT_ACTIVE_CTRL) && (direction > 0)) { //wheel을 아래에서 위로 scroll, fovy 늘려서 축소(zoom-out)
		if (pCamera->fovy < 175.0f * TO_RADIAN) {
			pCamera->fovy += 1.0f * TO_RADIAN;
		}
		else {
			printf("current_camera.fovy is too small or too big.\n");
		}
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();

		//printf("dir > 0, wheel : %d\nfovy : %f\n", wheel, current_camera.fovy);
	}
	else if ((glutGetModifiers() == GLUT_ACTIVE_CTRL) && (direction < 0)) { 
		if (pCamera->fovy > 5.0f * TO_RADIAN) {
			pCamera->fovy -= 1.0f * TO_RADIAN;
		}
		else {
			printf("current_camera->fovy is too small or too big.\n");
		}
		ProjectionMatrix = glm::perspective(pCamera->fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();

		//printf("dir < 0, wheel : %d\nfovy : %f\n", wheel, current_camera.fovy);
	}
}

void special_20171694(int key, int x, int y) {
	Camera* pCamera = &camera_info[CAMERA_A];
	float del = 20.0f;

	if (pCamera->move == 1) {
		switch (key) {
		case GLUT_KEY_LEFT:
			pCamera->pos[0] -= del * pCamera->uaxis[0];
			pCamera->pos[1] -= del * pCamera->uaxis[1];
			pCamera->pos[2] -= del * pCamera->uaxis[2];
			set_current_camera(CAMERA_A);
			glutPostRedisplay();
			//printf("[POS] (%f, %f, %f)\n", pCamera->pos[0], pCamera->pos[1], pCamera->pos[2]);
			break;
		case GLUT_KEY_RIGHT:
			pCamera->pos[0] += del * pCamera->uaxis[0];
			pCamera->pos[1] += del * pCamera->uaxis[1];
			pCamera->pos[2] += del * pCamera->uaxis[2];
			set_current_camera(CAMERA_A);
			glutPostRedisplay();
			//printf("[POS] (%f, %f, %f)\n", pCamera->pos[0], pCamera->pos[1], pCamera->pos[2]);
			break;
		case GLUT_KEY_UP:
			pCamera->pos[0] += del * pCamera->vaxis[0];
			pCamera->pos[1] += del * pCamera->vaxis[1];
			pCamera->pos[2] += del * pCamera->vaxis[2];
			set_current_camera(CAMERA_A);
			glutPostRedisplay();
			//printf("[POS] (%f, %f, %f)\n", pCamera->pos[0], pCamera->pos[1], pCamera->pos[2]);
			break;
		case GLUT_KEY_DOWN:
			pCamera->pos[0] -= del * pCamera->vaxis[0];
			pCamera->pos[1] -= del * pCamera->vaxis[1];
			pCamera->pos[2] -= del * pCamera->vaxis[2];
			set_current_camera(CAMERA_A);
			glutPostRedisplay();
			//printf("[POS] (%f, %f, %f)\n", pCamera->pos[0], pCamera->pos[1], pCamera->pos[2]);
			break;
		}
	}
}

int prevx, prevy;
void mousepress_20171694(int button, int state, int x, int y) {
	Camera* pCamera = &camera_info[CAMERA_A];

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		prevx = x; prevy = y;
		camera_move_mouse = 1;
	}
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		camera_move_mouse = 0;
	}
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		prevx = x; prevy = y;
		camera_rotation_mouse = 1;
	}
	if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		camera_rotation_mouse = 0;
	}
}

void mousemove_20171694(int x, int y) {
	Camera* pCamera = &camera_info[CAMERA_A];

	/* camera_a 앞뒤 이동 */
	if ((pCamera->move == 1)&&(camera_move_mouse == 1)) {
		int del_y = prevy - y; //마우스가 위로 간 양

		pCamera->pos[0] += del_y / 1.0f * pCamera->naxis[0];
		pCamera->pos[1] += del_y / 1.0f * pCamera->naxis[1];
		pCamera->pos[2] += del_y / 1.0f * pCamera->naxis[2];
		//printf("[POS] (%f, %f, %f)\n", pCamera->pos[0], pCamera->pos[1], pCamera->pos[2]);

		set_current_camera(CAMERA_A);
		glutPostRedisplay();

		prevx = x; prevy = y;
	}

	/* camera_a u, v, n축 회전 */
	if ((pCamera->move == 1)&&(camera_rotation_mouse == 1)) {
		int del_x = prevx - x; //마우스가 왼쪽으로 간 양
		glm::mat3 RotationMatrix;
		glm::vec3 direction;

		if (camera_rotation_axis_u == 1) {
			//printf("u-axis rotating\n");
			RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), 0.01f * TO_RADIAN * (-del_x),
				glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2])));

			direction = RotationMatrix * glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2]);
			pCamera->vaxis[0] = direction.x; pCamera->vaxis[1] = direction.y; pCamera->vaxis[2] = direction.z;
			direction = RotationMatrix * glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);
			pCamera->naxis[0] = direction.x; pCamera->naxis[1] = direction.y; pCamera->naxis[2] = direction.z;
		}
		if (camera_rotation_axis_v == 1) {
			//printf("v-axis rotating\n");
			RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), 0.01f * TO_RADIAN * (-del_x),
				glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2])));

			direction = RotationMatrix * glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2]);
			pCamera->uaxis[0] = direction.x; pCamera->uaxis[1] = direction.y; pCamera->uaxis[2] = direction.z;
			direction = RotationMatrix * glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);
			pCamera->naxis[0] = direction.x; pCamera->naxis[1] = direction.y; pCamera->naxis[2] = direction.z;
		}
		if (camera_rotation_axis_n == 1) {
			//printf("n-axis rotating\n");
			RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), 0.01f * TO_RADIAN * (-del_x),
				glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2])));

			direction = RotationMatrix * glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2]);
			pCamera->uaxis[0] = direction.x; pCamera->uaxis[1] = direction.y; pCamera->uaxis[2] = direction.z;
			direction = RotationMatrix * glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2]);
			pCamera->vaxis[0] = direction.x; pCamera->vaxis[1] = direction.y; pCamera->vaxis[2] = direction.z;
		}
		/*printf("[UVN] u = (%f, %f, %f), v = (%f, %f, %f), n = (%f, %f, %f)\n",
			pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2],
			pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2],
			pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);*/

		set_current_camera(CAMERA_A);
		glutPostRedisplay();

		prevx = x; prevy = y;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, sun_temple_VAO);
	glDeleteBuffers(scene.n_materials, sun_temple_VBO);

	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);
	free(flag_texture_mapping);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) { //2-1
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);

	glutMouseFunc(mousepress_20171694);
	glutMotionFunc(mousemove_20171694);
	glutMouseWheelFunc(mousewheel_20171694);
	glutSpecialFunc(special_20171694);
	glutTimerFunc(100, timer_20171694, 0);
}

void initialize_OpenGL(void) { //2-3
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();

	/*-----------------------------------------------*/
	timestamp_tiger = timestamp_scene;
	timestamp_nod = timestamp_scene;

	ModelMatrix_tiger_eye = glm::mat4(1.0f);
	ModelMatrix_tiger_eye = glm::translate(ModelMatrix_tiger_eye, glm::vec3(0.0f, -88.0f, 62.0f));
	ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	ModelMatrix_follow_tiger = glm::mat4(1.0f);
	ModelMatrix_follow_tiger = glm::translate(ModelMatrix_follow_tiger, glm::vec3(0.0f, 210.0f, 0.0f));
	ModelMatrix_follow_tiger = ModelMatrix_follow_tiger * ModelMatrix_tiger_eye;

	ModelMatrix_back_of_tiger = ModelMatrix_follow_tiger;
	ModelMatrix_back_of_tiger = glm::rotate(ModelMatrix_back_of_tiger, -180.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

}

void prepare_scene(void) { //2-4
	prepare_axes();
	prepare_grid();
	prepare_sun_temple();

	prepare_tiger_20171694();
	prepare_ben_20171694();
	prepare_wolf_20171694();
	/*--------------------*/
	prepare_bike_20171694();
	prepare_bus_20171694();
	prepare_cow_20171694();
	prepare_optimus_20171694();
	prepare_tank_20171694();
}

void initialize_renderer(void) { //2
	register_callbacks(); //2-1
	prepare_shader_program(); //2-2
	initialize_OpenGL(); //2-3
	prepare_scene(); //2-4
	initialize_camera(); //2-5
}

void initialize_glew(void) { //1-1
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) { //1
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew(); //1-1
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES); //1
	initialize_renderer(); //2

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
