#include "Point2.h"
#include "Vector2.h"
#include "VoronoiDiagramGenerator.h"
#include <vector>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <limits>
#include <Windows.h>

#include "Data/River.h"

#include "FastNoise/FastNoiseLite.h"

// GLEW
#define GLEW_STATIC
#if DEBUG
#include "../lib/GLEW/glew.h"
#else
#include "../lib/release_lib/GLEW/glew.h"
#endif

// GLFW

#if DEBUG
#include "../lib/GLFW/glfw3.h"
#else
#include "../lib/release_lib/GLFW/glfw3.h"
#endif

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// Window dimensions
const GLuint WINDOW_WIDTH = 1080 * 1, WINDOW_HEIGHT = 1080 * 1;
const int IMAGE_SCALE = 4;
const GLuint IMAGE_WIDTH = WINDOW_WIDTH * IMAGE_SCALE, IMAGE_HEIGHT = WINDOW_HEIGHT * IMAGE_SCALE;
// Shaders
const GLchar* vertexShaderSource =
	"#version 330 core\n"
	"layout (location = 0) in vec3 position;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
	"}\0";
const GLchar* fragmentShaderSource =
	"#version 330 core\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n\0";
double normalize(double in, int dimension) {
	return in / (float)dimension*1.8 - 0.9;
}
//globals for use in giving relaxation commands
int relax = 0;
bool startOver = true;
bool relaxForever = false;
bool oneSec = false;

bool draw_line = true;
bool draw_white_dot = true;
bool draw_special_dot = true;
bool save_image = false;
Point2 cur_pos = Point2(0, 0);
double cur_scale = 0;

std::clock_t startOneSec = NULL;
unsigned int oneSecCnt = 0;

bool mouse_left_down = false;


GLuint fbo;
GLuint texture;

GLfloat matrix_2[3][3] = { {2.0f, -4.0f, 2.0f}, {-3.0f, 4.0f, -1.0f}, {1.0f, 0.0f, 0.0f} };
GLfloat matrix_3[4][4] = { {-1.0f, 3.0f, -3.0f, 1.0f},
						  {2.0f, -5.0f, 4.0f, -1.0f},
						  {-1.0f, 0.0f, 1.0f, 0.0f},
						  {0.0f, 2.0f, 0.0f, 0.0f} };


GLvoid drawCardinal(RiverPointVector& point, unsigned int dimension, double radius, double river_scale) {
	// init

	GLfloat result[3][2];
	memset(result, 0, sizeof(result));

	// 행렬 (2) 와 (3) 의 곱
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][0] += matrix_2[i][j] * point[j].point.x;
			result[i][1] += matrix_2[i][j] * point[j].point.y;
		}
	}

	const int num_segments = 100; // 원을 근사하기 위한 세그먼트 수
  // 원의 반지름

	GLfloat t = 0.0f;
	GLfloat x, y;


	//glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
	//glBegin(GL_TRIANGLES);
	////glVertex2f(cx, cy); // 원의 중심
	//for (int i = 0; i < num_segments; i++) {
	//	float theta = 2.0f * 3.1415926f * float(i) / float(num_segments); // 현재 각도
	//	float x_ = radius * (river_scale * point[0].power + 1) * cosf(theta); // x 좌표
	//	float y_ = radius * (river_scale * point[0].power + 1) * sinf(theta); // y 좌표
	//	glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
	//	glVertex2f(normalize(x_ + point[0].cell->site.p.x, dimension), -normalize(y_ + point[0].cell->site.p.y, dimension)); // 원의 점
	//
	//	theta = 2.0f * 3.1415926f * float(i - 1) / float(num_segments); // 현재 각도
	//	x_ = radius * (river_scale * point[0].power + 1) * cosf(theta); // x 좌표
	//	y_ = radius * (river_scale * point[0].power + 1) * sinf(theta); // y 좌표
	//	glVertex2f(normalize(x_ + point[0].cell->site.p.x, dimension), -normalize(y_ + point[0].cell->site.p.y, dimension)); // 원의 점
	//	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	//	glVertex2f(normalize(point[0].cell->site.p.x, dimension), -normalize(point[0].cell->site.p.y, dimension)); // 원의 점
	//	
	//}
	//glEnd();

	/*glPointSize(3);
	glBegin(GL_POINTS);
	glColor4f(0, 1, 0, 1);
	glVertex3d(normalize(point[0].cell->site.p.x, dimension), -normalize(point[0].cell->site.p.y, dimension), 0);
	glEnd();*/

	
	glBegin(GL_LINE_STRIP);
	
	while (t < 1) {
		if (t < 0.5) {

			if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
				glColor4f(0, 1, 0, 1);
			}
			else {
				glColor4f(1, 0, 0, 1);
			}
		}
		else {
			if (point[1].GetCell()->GetDetail().GetElevation() == point[2].GetCell()->GetDetail().GetElevation()) {
				glColor4f(0, 1, 0, 1);
			}
			else {
				glColor4f(1, 0, 0, 1);
			}
		}
		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);
		glVertex2f(normalize(x, dimension), -normalize(y, dimension));
		t += 0.01f;
	}
	glEnd();

	t = 0.0f;
	Point2 pre_p = point[0].point;
	Point2 pre_norm = point[0].point;
	
	glBegin(GL_TRIANGLES);
	while (t < 1) {
		
		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);
		
		double x2 = result[2][0] + (t + 0.01) * (result[1][0] + result[0][0] * (t + 0.01));
		double y2 = result[2][1] + (t + 0.01) * (result[1][1] + result[0][1] * (t + 0.01));
		Color c;
		if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
			c = Color(0, 1, 0);
		}
		else {
			c = Color(1, 0, 0);
		}
		glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
	/*	std::cout << point[0].power << "\n";
		std::cout << point[2].power << "\n";*/
		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = (new_p - pre_p);
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[0].power * (1 - t) + point[2].power * t) * river_scale + 1;
		
		if (t == 0) {
			t += 0.01f;
			continue;
		}
		t += 0.01f;
		double sacle2 = (point[0].power * (1 - t) + point[2].power * t) * river_scale + 1;
		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;
			glColor4f(c.r, c.g, c.b, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(c.r, c.g, c.b, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));

			glColor4f(c.r, c.g, c.b, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(c.r, c.g, c.b, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));
			
			glColor4f(c.r, c.g, c.b, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(c.r, c.g, c.b, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));
			glVertex2f(normalize(new_p.x + PerpB.x, dimension), -normalize(new_p.y + PerpB.y, dimension));

			glColor4f(c.r, c.g, c.b, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(c.r, c.g, c.b, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));
			glVertex2f(normalize(new_p.x - PerpB.x, dimension), -normalize(new_p.y - PerpB.y, dimension));
		}
		pre_p = new_p;
	}
	glEnd();
}

void draw_spline(RiverPointVector& point, unsigned int dimension, double radius, double river_scale) {
	GLfloat result[4][2];
	GLfloat t = 0.0f;
	GLfloat x, y;
	int SIZE = point.size();
	// Section 1.
	// quadratic function
	memset(result, 0, sizeof(result));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][0] += matrix_2[i][j] * point[j].point.x;
			result[i][1] += matrix_2[i][j] * point[j].point.y;
		}
	}


	if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
		glColor4f(0, 1, 0, 1);
	}
	else {
		glColor4f(1, 0, 0, 1);
	}

	glBegin(GL_LINE_STRIP);
	while (t < 0.5f) {
		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);
		glVertex2f(normalize(x, dimension), -normalize(y, dimension));
		t += 0.01f;
	}
	glEnd();

	t = 0.0f;
	Point2 pre_p = point[0].point;
	Point2 pre_norm = point[0].point;
	glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	while (t < 0.5f) {

		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);

		double x2 = result[2][0] + (t + 0.01) * (result[1][0] + result[0][0] * (t + 0.01));
		double y2 = result[2][1] + (t + 0.01) * (result[1][1] + result[0][1] * (t + 0.01));


		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = (new_p - pre_p);
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[0].power * ((0.5 - t) * 2) + point[1].power * (t * 2)) * river_scale + 1;
		if (t == 0) {
			t += 0.01f;
			continue;
		}
		t += 0.01f;
		double sacle2 = (point[0].power * ((0.5 - t) * 2) + point[1].power * (t * 2)) * river_scale + 1;
		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;
			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));
			glVertex2f(normalize(new_p.x + PerpB.x, dimension), -normalize(new_p.y + PerpB.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));
			glVertex2f(normalize(new_p.x - PerpB.x, dimension), -normalize(new_p.y - PerpB.y, dimension));
		}
		pre_p = new_p;
	}
	glEnd();

	// Section 2.
	// cubic spline
	for (int cubic_case = 0; cubic_case < SIZE - 3; cubic_case++)
	{
		memset(result, 0, sizeof(result));
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result[i][0] += matrix_3[i][j] * point[j + cubic_case].point.x;
				result[i][1] += matrix_3[i][j] * point[j + cubic_case].point.y;
			}
		}
		
		if (point[cubic_case + 1].GetCell()->GetDetail().GetElevation() == point[cubic_case + 2].GetCell()->GetDetail().GetElevation()) {
			glColor4f(0, 1, 0, 1);
		}
		else {
			glColor4f(1, 0, 0, 1);
		}

		t = 0.0f;
		glBegin(GL_LINE_STRIP);
		while (t < 1.0f) {
			x = (result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f;
			y = (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f;
			glVertex2f(normalize(x, dimension), -normalize(y, dimension));
			t += 0.01f;
		}
		glEnd();





		t = 0.0f;
		//Point2 pre_p = point[0]->site.p;
		//Point2 pre_norm = point[0]->site.p;
		
		glBegin(GL_TRIANGLES);
		while (t < 1.0f) {

			x = (result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f;
			y = (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f;

			double x2 = (result[3][0] + (t + 0.01f) * (result[2][0] + (t + 0.01f) * (result[1][0] + result[0][0] * (t + 0.01f)))) * 0.5f;
			double y2 = (result[3][1] + (t + 0.01f) * (result[2][1] + (t + 0.01f) * (result[1][1] + result[0][1] * (t + 0.01f)))) * 0.5f;


			Point2 new_p = Point2(x, y);
			Point2 next_p = Point2(x2, y2);
			Point2 norm = (new_p - pre_p);
			Point2 norm2 = (next_p - new_p);
			double sacle1 = (point[cubic_case + 1].power * (1 - t) + point[cubic_case + 2].power * t) * river_scale + 1;
			if (t == 0) {
				t += 0.01f;
				continue;
			}
			t += 0.01f;
			double sacle2 = (point[cubic_case + 1].power * (1 - t) + point[cubic_case + 2].power * t) * river_scale + 1;
			if (norm != Point2(0, 0)) {
				norm = norm.Normailize();
				norm2 = norm2.Normailize();
				Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
				Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;
				glColor4f(1, 0, 0, 1);
				glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
				glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
				glColor4f(1, 0, 0, 0);
				glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));

				glColor4f(1, 0, 0, 1);
				glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
				glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
				glColor4f(1, 0, 0, 0);
				glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));

				glColor4f(1, 0, 0, 1);
				glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
				glColor4f(1, 0, 0, 0);
				glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));
				glVertex2f(normalize(new_p.x + PerpB.x, dimension), -normalize(new_p.y + PerpB.y, dimension));

				glColor4f(1, 0, 0, 1);
				glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
				glColor4f(1, 0, 0, 0);
				glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));
				glVertex2f(normalize(new_p.x - PerpB.x, dimension), -normalize(new_p.y - PerpB.y, dimension));
			}
			pre_p = new_p;
		}
		glEnd();




	}

	// Section 3.
	// quadratic function
	memset(result, 0, sizeof(result));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][0] += matrix_2[i][j] * point[j + SIZE - 3].point.x;
			result[i][1] += matrix_2[i][j] * point[j + SIZE - 3].point.y;
		}
	}

	if (point[SIZE - 2].GetCell()->GetDetail().GetElevation() == point[SIZE - 1].GetCell()->GetDetail().GetElevation()) {
		glColor4f(0, 1, 0, 1);
	}
	else {
		glColor4f(1, 0, 0, 1);
	}

	t = 0.5f;
	glBegin(GL_LINE_STRIP);
	while (t < 1.0f) {
		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);
		glVertex2f(normalize(x, dimension), -normalize(y, dimension));
		t += 0.01f;
	}
	glEnd();



	t = 0.51f;
	glBegin(GL_TRIANGLES);
	while (t < 1.0f) {

		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);

		double x2 = result[2][0] + (t + 0.01) * (result[1][0] + result[0][0] * (t + 0.01));
		double y2 = result[2][1] + (t + 0.01) * (result[1][1] + result[0][1] * (t + 0.01));


		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = (new_p - pre_p);
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[SIZE - 2].power * (1 - t) + point[SIZE - 1].power * t) * river_scale + 1;
		if (t == 0) {
			t += 0.01f;
			continue;
		}
		t += 0.01f;
		double sacle2 = (point[SIZE - 2].power * (1 - t) + point[SIZE - 1].power * t) * river_scale + 1;
		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;
			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glVertex2f(normalize(pre_p.x, dimension), -normalize(pre_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x + PerpA.x, dimension), -normalize(pre_p.y + PerpA.y, dimension));
			glVertex2f(normalize(new_p.x + PerpB.x, dimension), -normalize(new_p.y + PerpB.y, dimension));

			glColor4f(1, 0, 0, 1);
			glVertex2f(normalize(new_p.x, dimension), -normalize(new_p.y, dimension));
			glColor4f(1, 0, 0, 0);
			glVertex2f(normalize(pre_p.x - PerpA.x, dimension), -normalize(pre_p.y - PerpA.y, dimension));
			glVertex2f(normalize(new_p.x - PerpB.x, dimension), -normalize(new_p.y - PerpB.y, dimension));
		}
		pre_p = new_p;
	}
	glEnd();
}


void move_screen(double x, double y, double z) {
	cur_pos = cur_pos + Point2(x, y);
	glTranslatef(x, y, z);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		++relax;
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		relax += 10;
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		startOver = true;
	if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		if (relaxForever) relaxForever = false;
		else relaxForever = true;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		startOneSec = std::clock();
		oneSec = true;
		oneSecCnt = 0;
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		draw_line = !draw_line;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS && !(mode & GLFW_MOD_SHIFT)) {
		draw_white_dot = !draw_white_dot;
	}
	
	if (key == GLFW_KEY_D && action == GLFW_PRESS && (mode & GLFW_MOD_SHIFT) ) {
		draw_special_dot = !draw_special_dot;
	}
	
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		save_image = true;
	}

	if (key == GLFW_KEY_LEFT) {
		move_screen(-0.01f, 0.0f, 0);
	}
	if (key == GLFW_KEY_RIGHT) {
		move_screen(0.01f, 0.0f, 0);
	}
	if (key == GLFW_KEY_UP) {
		move_screen(0.0f, 0.01f, 0);
	}
	if (key == GLFW_KEY_DOWN) {
		move_screen(0.0f, -0.01f, 0);
	}

	
}

void screen_dump()
{
	auto start = std::clock();
	//W: window with H: window height
	glReadBuffer(GL_FRONT);
	char* pixel_data = new char[IMAGE_WIDTH * IMAGE_HEIGHT * 300];
	//char pixel_data[IMAGE_WIDTH * IMAGE_HEIGHT * 300];
	//glReadPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);

	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	FILE* out = nullptr;
	char buff[256];
	const char* filename = "voronoi_map_opengl.bmp";
	fopen_s(&out, filename, "wb");
	char* data = pixel_data;
	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));
	bf.bfType = 'MB';
	bf.bfSize = sizeof(bf) + sizeof(bi) + IMAGE_WIDTH * IMAGE_HEIGHT * 3;
	bf.bfOffBits = sizeof(bf) + sizeof(bi);
	bi.biSize = sizeof(bi);
	bi.biWidth = IMAGE_WIDTH;
	bi.biHeight = IMAGE_HEIGHT;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biSizeImage = IMAGE_WIDTH * IMAGE_HEIGHT * 3;
	fwrite(&bf, sizeof(bf), 1, out);
	fwrite(&bi, sizeof(bi), 1, out);
	fwrite(data, sizeof(unsigned char), IMAGE_HEIGHT * IMAGE_WIDTH * 3, out);
	fclose(out);
	delete[] pixel_data;

	auto duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "file saved: " << filename << " " << duration << "ms \n";
}

void mouse_button_callback(GLFWwindow* window, int key, int action, int mods)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouse_left_down = true;
	}
	if (key == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouse_left_down = false;
	}
}

double pre_xpos = 0, pre_ypos = 0;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	double x = xpos - pre_xpos, y = ypos - pre_ypos;
	double scale = (1 + pow(cur_scale, 2));
	x *= 0.001 / scale;
	y *= 0.001 / scale;
	//std::cout << xpos << std::endl;
	if (mouse_left_down) {
		move_screen(x, -y, 0);
	}
	pre_xpos = xpos;
	pre_ypos = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	double temp_offset = yoffset * 0.01;
	yoffset *= 0.03;
	cur_scale += yoffset;
	
	if (cur_scale <= 0) {
		cur_scale = 0;
	}
	else {
		//std::cout << cur_scale << "\n";

		double value = sqrt(cur_scale);

		glScalef(1 + yoffset, 1 + yoffset, 1);
		move_screen(cur_pos.x * temp_offset / value, cur_pos.y * temp_offset / value, 0);
		//if (yoffset > 0) {
		//	glScalef(xoffset, xoffset, 1);
		//}
		//else if (yoffset < 0) {
		//	// 휠을 아래로 스크롤한 경우의 처리
		//}
	}
	
}

bool sitesOrdered(const Point2& s1, const Point2& s2) {
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

void genRandomSites(int seed, std::vector<Point2>& sites, BoundingBox& bbox, unsigned int dimension, unsigned int numSites) {

	bbox = BoundingBox(0, dimension, dimension, 0);
	std::vector<Point2> tmpSites;
	numSites = (unsigned int)sqrt(numSites);
	unsigned int pow_site = (unsigned int)pow(numSites, 2);
	tmpSites.reserve(pow_site);
	sites.reserve(pow_site);

	Point2 s;

	double step = dimension / numSites;
	int half_step = (int)(step / 1.5);
	
	srand(seed);
	for (unsigned int i = 0; i < numSites; ++i) {
		for (unsigned int j = 0; j < numSites; ++j) {
			
			//s.x = (i * step) + (rand() / ((double)RAND_MAX)) * (half_step);
			s.x = (i * step) + (rand() % (int)half_step);
			s.y = (j * step) + (rand() % (int)half_step);
			//std::cout << "rand: " << (rand() / ((double)RAND_MAX)) * (half_step) << "\n";
			//std::cout << "x: " << s.x << "\n";
			//std::cout << "y: " << s.y << "\n";
			tmpSites.push_back(s);
		}
	}

	//remove any duplicates that exist
	std::sort(tmpSites.begin(), tmpSites.end(), sitesOrdered);
	sites.push_back(tmpSites[0]);
	for (Point2& s : tmpSites) {
		if (s != sites.back()) sites.push_back(s);
	}
}


void draw_image(VoronoiDiagramGenerator* vdg, unsigned int dimension) {

	GLfloat pointSize = 4;
	GLfloat whitePointSize = 1;

	Diagram* diagram = vdg->GetDiagram();

	glBegin(GL_TRIANGLES);
	for (Cell* c : diagram->cells) {
		//for (Edge* e : diagram->edges) {
		for (HalfEdge* hf : c->halfEdges) {
			Edge* e = hf->edge;

			Site* s = e->lSite->cell == c ? e->lSite : e->rSite;
			Site* opposite_s = e->rSite && e->lSite->cell == c ? e->rSite : e->lSite;

			Point2& center = c->site.p;
			Point2& pA = e->vertA->point;
			Point2& pB = e->vertB->point;

			Point2 edge_mp = e->p;
			Color colorA = e->vertA->color;;
			Color colorB = e->vertB->color;;
			Color middle_c = e->color;
			CellDetail& cd = c->GetDetail();
			CellDetail& tcd = opposite_s->cell->GetDetail();
		
			Color center_c = s->cell->GetDetail().GetColor();

			//middle_c = (colorA + colorB) / 2;
			
			glColor4f((GLfloat)std::clamp(center_c.r, 0.0, 1.0), (GLfloat)std::clamp(center_c.g, 0.0, 1.0), (GLfloat)std::clamp(center_c.b, 0.0, 1.0), (GLfloat)center_c.a);
			glVertex3d(normalize(center.x, dimension), -normalize(center.y, dimension), 0.0);


			glColor4f((GLfloat)std::clamp(middle_c.r, 0.0, 1.0), (GLfloat)std::clamp(middle_c.g, 0.0, 1.0), (GLfloat)std::clamp(middle_c.b, 0.0, 1.0), (GLfloat)middle_c.a);
			glVertex3d(normalize(edge_mp.x, dimension), -normalize(edge_mp.y, dimension), 0.0);


			glColor4f((GLfloat)std::clamp(colorA.r, 0.0, 1.0), (GLfloat)std::clamp(colorA.g, 0.0, 1.0), (GLfloat)std::clamp(colorA.b, 0.0, 1.0), (GLfloat)colorA.a);
			glVertex3d(normalize(pA.x, dimension), -normalize(pA.y, dimension), 0.0);




			glColor4f((GLfloat)std::clamp(center_c.r, 0.0, 1.0), (GLfloat)std::clamp(center_c.g, 0.0, 1.0), (GLfloat)std::clamp(center_c.b, 0.0, 1.0), (GLfloat)center_c.a);
			glVertex3d(normalize(center.x, dimension), -normalize(center.y, dimension), 0.0);

			glColor4f((GLfloat)std::clamp(colorB.r, 0.0, 1.0), (GLfloat)std::clamp(colorB.g, 0.0, 1.0), (GLfloat)std::clamp(colorB.b, 0.0, 1.0), (GLfloat)colorB.a);
			glVertex3d(normalize(pB.x, dimension), -normalize(pB.y, dimension), 0.0);

			glColor4f((GLfloat)std::clamp(middle_c.r, 0.0, 1.0), (GLfloat)std::clamp(middle_c.g, 0.0, 1.0), (GLfloat)std::clamp(middle_c.b, 0.0, 1.0), (GLfloat)middle_c.a);
			glVertex3d(normalize(edge_mp.x, dimension), -normalize(edge_mp.y, dimension), 0.0);


		}
	}
	glEnd();


	if (draw_line) {
		for (Edge* e : diagram->edges) {
			Point2& p1 = e->vertA->point;
			Point2& p2 = e->vertB->point;
			Point2& mp = e->p;

			glBegin(GL_LINES);
			glColor4f(0, 0, 0, 1);
			glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
			glVertex3d(normalize(mp[0], dimension), -normalize(mp[1], dimension), 0.0);

			glVertex3d(normalize(mp[0], dimension), -normalize(mp[1], dimension), 0.0);
			glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
			
			glEnd();
		}
	}

	if (false) {


		for (Cell* c : diagram->cells) {
			size_t edgeCount = c->halfEdges.size();
			for (HalfEdge* hf : c->halfEdges) {
				/*if (!hf->edge->check) {
					hf->edge->check = true;
					std::cout << "checked!\n";
				}*/
				//glBegin(GL_LINES);
				//glBegin(GL_LINES);
				//glBegin(GL_TRIANGLES);
				Point2& p1 = hf->startPoint()->point;
				Point2& p2 = hf->endPoint()->point;
				//glColor4d((rand() / (double)RAND_MAX) * 255, (rand() / (double)RAND_MAX) * 255, (rand() / (double)RAND_MAX) * 255, 255);
				//double color1 = (rand() / (double)RAND_MAX);
				//double color2 = (rand() / (double)RAND_MAX);
				//double color3 = (rand() / (double)RAND_MAX);

				//glColor4d(c->color1, c->color2, c->color3, 0.2f);

				//glVertex3d(normalize(c->site.p[0], dimension), -normalize(c->site.p[1], dimension), 0.0);
				//glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
				//glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
				//glEnd();


			}
		}
	}

	if (draw_white_dot || draw_special_dot) {
		
		//glEnable(GL_DEPTH_TEST);
		for (RiverLine& line : diagram->river_lines.GetArray()) {
			RiverPointVector& point_vec = line.GetPointArray();
			RiverPoint pre_c = RiverPoint(0, nullptr);
			bool temp = false;
			const double radius = vdg->GetSetting().GetRiverRadius();
			const double river_scale = vdg->GetSetting().GetRiverPowerScale();
			if (point_vec.size() == 3) {
				/*std::vector<Cell*> cells;
				for (auto c : c_vec) {
					cells.push_back(c.cell);
				}*/
				drawCardinal(point_vec, dimension, radius, river_scale);
			}
			else if (point_vec.size() > 3) {
				/*std::vector<Cell*> cells;
				for (auto c : c_vec) {
					cells.push_back(c.cell);
				}*/
				draw_spline(point_vec, dimension, radius, river_scale);
			}
			else {


				for (RiverPoint c : point_vec) {
					if (!temp) {
						temp = true;
						pre_c = c;
						continue;
					}


					Point2& p1 = pre_c.point;
					Point2& p2 = c.point;
					glColor4f(0, 1, 1, 1);
					glPointSize(8);
					glBegin(GL_POINTS);
					glVertex3d(normalize(p2.x, dimension), -normalize(p2.y, dimension), 0.0);
					glEnd();

					Color color;
					glBegin(GL_LINES);
					if (pre_c.GetCell()->GetDetail().GetElevation() == c.GetCell()->GetDetail().GetElevation()) {
						glColor4f(0, 1, 0, 1);
						color = Color(0, 1, 0);
					}
					else {
						glColor4f(1, 0, 0, 1);
						color = Color(1, 0, 0);
					}
					glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
					glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
					glEnd();



					Point2 norm = (p2 - p1).Normailize();
					double sacle1 = (pre_c.power) * river_scale + 1;
					double sacle2 = (c.power) * river_scale + 1;

					Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
					Point2 PerpB = Point2(-norm.y, norm.x) * radius * sacle2;
					glBegin(GL_TRIANGLES);
					glColor4f(color.r, color.g, color.b, 1);
					glVertex2f(normalize(p2.x, dimension), -normalize(p2.y, dimension));
					glVertex2f(normalize(p1.x, dimension), -normalize(p1.y, dimension));
					glColor4f(color.r, color.g, color.b, 0);
					glVertex2f(normalize(p1.x - PerpA.x, dimension), -normalize(p1.y - PerpA.y, dimension));

					glColor4f(color.r, color.g, color.b, 1);
					glVertex2f(normalize(p2.x, dimension), -normalize(p2.y, dimension));
					glVertex2f(normalize(p1.x, dimension), -normalize(p1.y, dimension));
					glColor4f(color.r, color.g, color.b, 0);
					glVertex2f(normalize(p1.x + PerpA.x, dimension), -normalize(p1.y + PerpA.y, dimension));


					glColor4f(color.r, color.g, color.b, 1);
					glVertex2f(normalize(p2.x, dimension), -normalize(p2.y, dimension));
					glColor4f(color.r, color.g, color.b, 0);
					glVertex2f(normalize(p1.x + PerpA.x, dimension), -normalize(p1.y + PerpA.y, dimension));
					glVertex2f(normalize(p2.x + PerpB.x, dimension), -normalize(p2.y + PerpB.y, dimension));

					glColor4f(color.r, color.g, color.b, 1);
					glVertex2f(normalize(p2.x, dimension), -normalize(p2.y, dimension));
					glColor4f(color.r, color.g, color.b, 0);
					glVertex2f(normalize(p1.x - PerpA.x, dimension), -normalize(p1.y - PerpA.y, dimension));
					glVertex2f(normalize(p2.x - PerpB.x, dimension), -normalize(p2.y - PerpB.y, dimension));
					glEnd();
					pre_c = c;
				}
			}
		}
		//glDisable(GL_DEPTH_TEST);
		for (Cell* c : diagram->cells) {
			Point2& p = c->site.p;
			Point2& p2 = c->GetDetail().GetUnionFind().UnionFindCell(Terrain::PEAK)->site.p;

			//if (c->GetDetail().IsFlat()) {
			//	glPointSize(5);
			//	glBegin(GL_POINTS);
			//	glColor4f(1, 0, 0, 1);
			//	glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
			//	glEnd();
			//}

			if (draw_special_dot && c->GetDetail().IsHighestPeak()) {
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f(0, 0, 0, 1);
				glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_special_dot && c->GetDetail().GetPeak()) {

				/*	glPointSize(8);
					glBegin(GL_POINTS);
					glColor4f((GLfloat)1, (GLfloat)0, (GLfloat)0, (GLfloat)0.5);
					glVertex3d(normalize(p2.x, dimension), -normalize(p2.y, dimension), 0.0);
					glEnd();*/


				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.7, (GLfloat)0.7, (GLfloat)0, (GLfloat)1);
				glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_special_dot && c->GetDetail().GetTerrain() == Terrain::COAST) {
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.4, (GLfloat)0.7, (GLfloat)1, (GLfloat)1);
				glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_white_dot) {
				glPointSize(whitePointSize * 2);
				glBegin(GL_POINTS);
				double temp = (double)c->GetDetail().GetMoisture() / vdg->GetMaxMoisture();
				//temp *= 2;
				//std::cout << "GetMoisture: " << c->GetDetail().GetMoisture() << ", LocalMoisture: " << c->GetDetail().GetLocalMoisture() << ", AreaMoisture: " << c->GetDetail().GetAreaMoisture() << ", GetMaxMoisture: " << vdg->GetMaxMoisture() << "\n";
				glColor4f(1.0 - temp, 1.0 - temp, 1, 1);
				glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				glEnd();
			}

			

		}

		if (draw_special_dot) {


			glPointSize(pointSize);
			glBegin(GL_POINTS);
			for (auto item : diagram->islandUnion.unions) {
				auto island = item.second;
				for (auto lake_union : island.lakeUnion.unions) {

					glColor4f((GLfloat)0, (GLfloat)0, (GLfloat)1, (GLfloat)1);
					for (auto lake : lake_union.second) {
						Point2& p = lake->site.p;
						glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
					}
				}

				/*
				color = 0;
				for (auto peak_union : island.highestPeakUnion.unions) {
					for (auto peak : peak_union.second) {
						Point2& p = peak->site.p;
						glColor4f(0, color, 1 - color, 1);
						glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
					}
					color += 0.4;
				}*/


				//auto island = item.second;
				//for (auto cell : island.land) {

				//	Point2& p = cell->site.p;
				//	glColor4f(1 - color, color, 0, 1);
				//	glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				//}
				//color += 0.5;


			}
			glEnd();
		}

	}

}


int main() {
	unsigned int nPoints = 10000;
	unsigned int dimension = 1000000;

	int seed = 0;//18;
	double radius = dimension / 2.1;

	unsigned int loop_cnt = 3;

	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();
	vdg.SetSetting(GenerateSetting(MapType::CONTINENT, seed, radius, 0.5, 0.5, 10, radius / 3, radius / 5, 50, radius / 15, radius / 20, 500.f, 0.2f));
	
	std::vector<Point2>* sites = nullptr;
	BoundingBox bbox;


	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Voronoi Diagram Generator", nullptr, nullptr);
	//GLFWwindow* window2 = glfwCreateWindow(IMAGE_WIDTH, IMAGE_HEIGHT, "Voronoi Diagram Generator", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();
	// Define the viewport dimensions

	glEnable(GL_BLEND);

	// 블렌딩 기능을 설정합니다.
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	while (!glfwWindowShouldClose(window)) {
		auto fame_start = std::clock();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		std::clock_t start;
		double duration;

		if (startOver) {
			std::cout << "\nUsage - When the OpenGL window has focus:\n"
				"\tPress 'R' to perform Lloyd's relaxation once.\n"
				"\tPress 'T' to perform Lloyd's relaxation ten times.\n"
				"\tPress 'Y' to toggle continuous Lloyd's relaxation.\n"
				"\tPress 'X' to generate a new diagram with a different number of sites.\n"
				"\tPress 'Esc' to exit.\n\n";
			startOver = false;
			relaxForever = false;
			relax = 0;
			sites = new std::vector<Point2>();
			//std::cout << "How many points? ";
			//nPoints = 1000;
			//std::cin >> nPoints;
			genRandomSites(seed, *sites, bbox, dimension, nPoints);
			start = std::clock();
			vdg.compute(*sites, bbox);

			vdg.relaxLoop(loop_cnt);
			vdg.CreateWorld();

			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			Diagram* diagram = vdg.GetDiagram();
			size_t lake_cnt = 0;
			for (auto item : diagram->islandUnion.unions) {
				lake_cnt += item.second.lakeUnion.unions.size();
			}
			std::cout << "lake_cnt: " << diagram->islandUnion.unions.size() << "\n";
			delete sites;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	

		if (relax || relaxForever || startOneSec) {
			vdg.GetSetting().SetSeed(vdg.GetSetting().GetSeed() + 1);
			//vdg.GetSetting().SetLakeScale(vdg.GetSetting().GetLakeScale() + 0.01);
			//std::cout << "vdg.GetSetting().GetLakeSize(): " << vdg.GetSetting().GetLakeScale() << "\n";
			//lakeScale += 0.01;
			//lakeDense += 0.01;
			//std::cout << "lake size: " << lakeScale << ", lakeDense: " << lakeSize << "\n";

			start = std::clock();
			//diagram = vdg.relax();
			//diagram = vdg.relax();
			sites = new std::vector<Point2>();
			genRandomSites(vdg.GetSetting().GetSeed(), *sites, bbox, dimension, nPoints);

			vdg.compute(*sites, bbox);
			vdg.relaxLoop(loop_cnt);
			vdg.CreateWorld();
			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;

			delete sites;
			Diagram* diagram = vdg.GetDiagram();
			size_t lake_cnt = 0;
			for (auto item : diagram->islandUnion.unions) {
				lake_cnt += item.second.lakeUnion.unions.size();
			}

			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			std::cout << "lake_cnt: " << lake_cnt << "\n";
			std::cout << "seed: " << vdg.GetSetting().GetSeed() << "\n";
			if (diagram->cells.size() != 4) {
				int x = 0;
			}
			--relax;
			if (relax < 0) relax = 0;

			if (startOneSec) {
				oneSecCnt++;
				if (std::clock() - startOneSec >= 1000) {
					startOneSec = false;
					std::cout << "1 sec: " << oneSecCnt << "\n";
				}
			}
		}
		
		// Swap the screen buffers
		
		if (save_image) {
			save_image = false;
			glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);

			glGenFramebuffers(1, &fbo);
			glGenTextures(1, &texture);

			// 텍스처 바인딩
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			// FBO 바인딩
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);


			draw_image(&vdg, dimension);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			screen_dump();
			vdg.SaveAllImage(dimension, IMAGE_WIDTH, IMAGE_HEIGHT);
		}
		else {
			
			//glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			
			draw_image(&vdg, dimension);
			
		}
		
		glfwSwapBuffers(window);

		
		glfwPollEvents();

		double fame_duration = 1000 * (std::clock() - fame_start) / (double)CLOCKS_PER_SEC;
		if (fame_duration < 33) Sleep(33 - fame_duration);
	}

	// Terminate GLFW, clearing any resources allocated by it.
	glfwTerminate();

	return 0;
}
