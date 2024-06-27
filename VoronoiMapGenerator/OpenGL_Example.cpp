﻿#include "Point2.h"
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
int Relax = 0;
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



void move_screen(double x, double y, double z) {
	cur_pos = cur_pos + Point2(x, y);
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		++Relax;
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		Relax += 10;
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
	const char* filename = "voronoi_map_opengl.bmp";
	FILE* out = nullptr;
	errno_t err = fopen_s(&out, filename, "wb");
	if (err != 0) {
		std::cout << "File write error!!\n";
		return;
	}

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

		glScalef((GLfloat)(1 + yoffset), (GLfloat)(1 + yoffset), 1.f);
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
			glVertex3d((GLfloat)normalize(center.x, dimension), -(GLfloat)normalize(center.y, dimension), 0.0);


			glColor4f((GLfloat)std::clamp(middle_c.r, 0.0, 1.0), (GLfloat)std::clamp(middle_c.g, 0.0, 1.0), (GLfloat)std::clamp(middle_c.b, 0.0, 1.0), (GLfloat)middle_c.a);
			glVertex3d((GLfloat)normalize(edge_mp.x, dimension), -(GLfloat)normalize(edge_mp.y, dimension), 0.0);


			glColor4f((GLfloat)std::clamp(colorA.r, 0.0, 1.0), (GLfloat)std::clamp(colorA.g, 0.0, 1.0), (GLfloat)std::clamp(colorA.b, 0.0, 1.0), (GLfloat)colorA.a);
			glVertex3d((GLfloat)normalize(pA.x, dimension), -(GLfloat)normalize(pA.y, dimension), 0.0);


			

			glColor4f((GLfloat)std::clamp(center_c.r, 0.0, 1.0), (GLfloat)std::clamp(center_c.g, 0.0, 1.0), (GLfloat)std::clamp(center_c.b, 0.0, 1.0), (GLfloat)center_c.a);
			glVertex3d((GLfloat)normalize(center.x, dimension), -(GLfloat)normalize(center.y, dimension), 0.0);

			glColor4f((GLfloat)std::clamp(colorB.r, 0.0, 1.0), (GLfloat)std::clamp(colorB.g, 0.0, 1.0), (GLfloat)std::clamp(colorB.b, 0.0, 1.0), (GLfloat)colorB.a);
			glVertex3d((GLfloat)normalize(pB.x, dimension), -(GLfloat)normalize(pB.y, dimension), 0.0);

			glColor4f((GLfloat)std::clamp(middle_c.r, 0.0, 1.0), (GLfloat)std::clamp(middle_c.g, 0.0, 1.0), (GLfloat)std::clamp(middle_c.b, 0.0, 1.0), (GLfloat)middle_c.a);
			glVertex3d((GLfloat)normalize(edge_mp.x, dimension), -(GLfloat)normalize(edge_mp.y, dimension), 0.0);


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
			glVertex3d((GLfloat)normalize(p1[0], dimension), -(GLfloat)normalize(p1[1], dimension), 0.0);
			glVertex3d((GLfloat)normalize(mp[0], dimension), -(GLfloat)normalize(mp[1], dimension), 0.0);

			glVertex3d((GLfloat)normalize(mp[0], dimension), -(GLfloat)normalize(mp[1], dimension), 0.0);
			glVertex3d((GLfloat)normalize(p2[0], dimension), -(GLfloat)normalize(p2[1], dimension), 0.0);
			
			glEnd();
		}
	}

	for (RiverLine* line : diagram->river_lines.GetArray()) {
		const double radius = vdg->GetSetting().GetRiverRadius();
		const double river_scale = vdg->GetSetting().GetRiverPowerScale();

		glBegin(GL_TRIANGLES);
		for (Triangle& tri : line->GetTriangle()) {
			for (int i = 0; i < 3; i++) {
				glColor4f((GLfloat)tri.colors[i].r, (GLfloat)tri.colors[i].g, (GLfloat)tri.colors[i].b, (GLfloat)tri.colors[i].a);
				glVertex2f((GLfloat)normalize(tri.points[i].x, dimension), -(GLfloat)normalize(tri.points[i].y, dimension));
			}

		}
		glEnd();
	}


	glBegin(GL_TRIANGLES);
	for (Triangle& tri : diagram->river_cross.GetTriangle()) {
		for (int i = 0; i < 3; i++) {
			//std::cout << "test\n";
			glColor4f((GLfloat)tri.colors[i].r, (GLfloat)tri.colors[i].g, (GLfloat)tri.colors[i].b, (GLfloat)tri.colors[i].a);
			glVertex2f((GLfloat)normalize(tri.points[i].x, dimension), -(GLfloat)normalize(tri.points[i].y, dimension));
		}

	}
	glEnd();


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

				//glVertex3d((GLfloat)normalize(c->site.p[0], dimension), -(GLfloat)normalize(c->site.p[1], dimension), 0.0);
				//glVertex3d((GLfloat)normalize(p1[0], dimension), -(GLfloat)normalize(p1[1], dimension), 0.0);
				//glVertex3d((GLfloat)normalize(p2[0], dimension), -(GLfloat)normalize(p2[1], dimension), 0.0);
				//glEnd();


			}
		}
	}

	if (draw_white_dot || draw_special_dot) {
		
		//glEnable(GL_DEPTH_TEST);


		//glDisable(GL_DEPTH_TEST);
		for (Cell* c : diagram->cells) {
			Point2& p = c->site.p;
			Point2& p2 = c->GetDetail().GetUnionFind().UnionFindCell(Terrain::PEAK)->site.p;

			//if (c->GetDetail().IsFlat()) {
			//	glPointSize(5);
			//	glBegin(GL_POINTS);
			//	glColor4f(1, 0, 0, 1);
			//	glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
			//	glEnd();
			//}

			if (draw_special_dot && c->GetDetail().IsHighestPeak()) {
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f(0, 0, 0, 1);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_special_dot && c->GetDetail().GetPeak()) {

				/*	glPointSize(8);
					glBegin(GL_POINTS);
					glColor4f((GLfloat)1, (GLfloat)0, (GLfloat)0, (GLfloat)0.5);
					glVertex3d((GLfloat)normalize(p2.x, dimension), -(GLfloat)normalize(p2.y, dimension), 0.0);
					glEnd();*/


				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.7, (GLfloat)0.7, (GLfloat)0, (GLfloat)1);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_special_dot && c->GetDetail().GetTerrain() == Terrain::COAST) {
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.4, (GLfloat)0.7, (GLfloat)1, (GLfloat)1);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}
			else if (draw_white_dot) {
				glPointSize(whitePointSize * 2);
				glBegin(GL_POINTS);
				double temp = (double)c->GetDetail().GetMoisture() / vdg->GetMaxMoisture();
				//temp *= 2;
				//std::cout << "GetMoisture: " << c->GetDetail().GetMoisture() << ", LocalMoisture: " << c->GetDetail().GetLocalMoisture() << ", AreaMoisture: " << c->GetDetail().GetAreaMoisture() << ", GetMaxMoisture: " << vdg->GetMaxMoisture() << "\n";
				glColor4f((GLfloat)(1.0 - temp), (GLfloat)(1.0 - temp), 1.f, 1.f);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
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
						glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
					}
				}

				/*
				color = 0;
				for (auto peak_union : island.highestPeakUnion.unions) {
					for (auto peak : peak_union.second) {
						Point2& p = peak->site.p;
						glColor4f(0, color, 1 - color, 1);
						glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
					}
					color += 0.4;
				}*/


				//auto island = item.second;
				//for (auto cell : island.land) {

				//	Point2& p = cell->site.p;
				//	glColor4f(1 - color, color, 0, 1);
				//	glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
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

	double radius = dimension / 2.1;

	unsigned int loop_cnt = 3;

	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();
	vdg.SetSetting(GenerateSetting(MapType::CONTINENT, 0, 0.6666, radius, 0.5, 0.5, 10, radius / 3, radius / 5, 50, radius / 15, radius / 20, 500.f, 0.2f));
	
	//std::vector<Point2>* sites = nullptr;
	//BoundingBox bbox;


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
			Relax = 0;
			//sites = new std::vector<Point2>();
			//std::cout << "How many points? ";
			//nPoints = 1000;
			//std::cin >> nPoints;
			//genRandomSites(seed, *sites, bbox, dimension, nPoints);

			vdg.CreateSite(dimension, nPoints);
			start = std::clock();
			vdg.Compute();
			vdg.RelaxLoop(loop_cnt);
			vdg.CreateWorld();

			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			Diagram* diagram = vdg.GetDiagram();
			size_t lake_cnt = 0;
			for (auto item : diagram->islandUnion.unions) {
				lake_cnt += item.second.lakeUnion.unions.size();
			}
			std::cout << "lake_cnt: " << diagram->islandUnion.unions.size() << "\n";
			//delete sites;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	

		if (Relax || relaxForever || startOneSec) {
			vdg.GetSetting().SetSeed(vdg.GetSetting().GetSeed() + 1);
			//vdg.GetSetting().SetLakeScale(vdg.GetSetting().GetLakeScale() + 0.01);
			//std::cout << "vdg.GetSetting().GetLakeSize(): " << vdg.GetSetting().GetLakeScale() << "\n";
			//lakeScale += 0.01;
			//lakeDense += 0.01;
			//std::cout << "lake size: " << lakeScale << ", lakeDense: " << lakeSize << "\n";

			vdg.CreateSite(dimension, nPoints);
			start = std::clock();
			vdg.Compute();
			vdg.RelaxLoop(loop_cnt);
			vdg.CreateWorld();
			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;

			//delete sites;
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
			--Relax;
			if (Relax < 0) Relax = 0;

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
		if (fame_duration < 33) Sleep(DWORD(33 - fame_duration));
	}

	// Terminate GLFW, clearing any resources allocated by it.
	glfwTerminate();

	return 0;
}
