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
#include "Data/Setting.h"
#include "Data/Heightmap.h"

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
	if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		if (relaxForever) relaxForever = false;
		else relaxForever = true;
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

	// Drawing main map
	glBegin(GL_TRIANGLES);
	for (Triangle& tri : diagram->GetTriangles()) {
		for (int i = 0; i < 3; i++) {
			glColor4f((GLfloat)tri.colors[i].rgb.r, (GLfloat)tri.colors[i].rgb.g, (GLfloat)tri.colors[i].rgb.b, (GLfloat)tri.colors[i].rgb.a);
			glVertex3d((GLfloat)normalize(tri.points[i].x, dimension), -(GLfloat)normalize(tri.points[i].y, dimension), 0.0);
		}
	}
	glEnd();

	// Drawing cell edge line

	glBegin(GL_LINES);
	if (draw_line) {
		for (Edge* e : diagram->GetEdges()) {
			Point2& p1 = e->vertA->point;
			Point2& p2 = e->vertB->point;
			Point2& mp = e->p;

			glColor4f(0, 0, 0, 1);
			glVertex3d((GLfloat)normalize(p1[0], dimension), -(GLfloat)normalize(p1[1], dimension), 0.0);
			glVertex3d((GLfloat)normalize(mp[0], dimension), -(GLfloat)normalize(mp[1], dimension), 0.0);

			glVertex3d((GLfloat)normalize(mp[0], dimension), -(GLfloat)normalize(mp[1], dimension), 0.0);
			glVertex3d((GLfloat)normalize(p2[0], dimension), -(GLfloat)normalize(p2[1], dimension), 0.0);
			
			
		}
	}
	glEnd();

	// Drawing River
	glBegin(GL_TRIANGLES);
	for (RiverLine* line : diagram->GetRiverLines().GetArray()) {
		const double radius = vdg->GetSetting().GetRiverRadius();
		const double river_scale = vdg->GetSetting().GetRiverPowerScale();

		
		for (Triangle& tri : line->GetTriangle()) {
			for (int i = 0; i < 3; i++) {
				glColor4f((GLfloat)tri.colors[i].rgb.r, (GLfloat)tri.colors[i].rgb.g, (GLfloat)tri.colors[i].rgb.b, (GLfloat)tri.colors[i].rgb.a);
				glVertex2f((GLfloat)normalize(tri.points[i].x, dimension), -(GLfloat)normalize(tri.points[i].y, dimension));
			}

		}
		
	}
	glEnd();

	// Drawing RiverClossing
	glBegin(GL_TRIANGLES);
	for (Triangle& tri : diagram->GetRiverCrossing().GetTriangle()) {
		for (int i = 0; i < 3; i++) {
			//std::cout << "test\n";
			glColor4f((GLfloat)tri.colors[i].rgb.r, (GLfloat)tri.colors[i].rgb.g, (GLfloat)tri.colors[i].rgb.b, (GLfloat)tri.colors[i].rgb.a);
			glVertex2f((GLfloat)normalize(tri.points[i].x, dimension), -(GLfloat)normalize(tri.points[i].y, dimension));
		}

	}
	glEnd();

	if (draw_white_dot || draw_special_dot) {

		for (Cell* c : diagram->GetCells()) {
			Point2& p = c->site.p;
			Point2& p2 = c->GetDetail().GetUnionFind().UnionFindCell(Terrain::PEAK)->site.p;


			//Drawing flat dot
			//if (c->GetDetail().IsFlat()) {
			//	glPointSize(5);
			//	glBegin(GL_POINTS);
			//	glColor4f(1, 0, 0, 1);
			//	glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
			//	glEnd();
			//}

			// Drawing coast dot
			if (draw_special_dot && c->GetDetail().GetTerrain() == Terrain::COAST) {
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.4, (GLfloat)0.7, (GLfloat)1, (GLfloat)1);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}
			// Drawing dot ots according to moisture
			else if (draw_white_dot) {
				glPointSize(whitePointSize * 2);
				glBegin(GL_POINTS);
				double temp = (double)c->GetDetail().GetMoisture() / vdg->GetMaxMoisture();
				glColor4f((GLfloat)(1.0 - temp), (GLfloat)(1.0 - temp), 1.f, 1.f);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}

			/*if (c->GetDetail().IsFlat()) {
				glPointSize(whitePointSize * 2);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)(0), (GLfloat)(1.0), 0.f, 1.f);
				glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
				glEnd();
			}*/

			

		}

		if (draw_special_dot) {
			
			
			for (auto item : diagram->GetIslandUnion().unions) {
				auto island = item.second;

				//Drawing lake dot
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0, (GLfloat)0, (GLfloat)1, (GLfloat)1);
				for (auto lake_union : island.GetLakeUnion()) {
					for (auto lake : lake_union.second) {
						Point2& p = lake->site.p;
						glVertex3d((GLfloat)normalize(p.x, dimension), -(GLfloat)normalize(p.y, dimension), 0.0);
					}
					
				}
				glEnd();

				// Drawing peak dot
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f((GLfloat)0.7, (GLfloat)0.7, (GLfloat)0, (GLfloat)1);
				for (auto peak_union : island.GetPeakUnion()) {
					for (auto peak : peak_union.second) {
						glVertex3d((GLfloat)normalize(peak->site.p.x, dimension), -(GLfloat)normalize(peak->site.p.y, dimension), 0.0);
					}

				}
				glEnd();

				// Drawing highestPeak dot
				glPointSize(pointSize);
				glBegin(GL_POINTS);
				glColor4f(0, 0, 0, 1);
				for (auto peak_union : island.GetHighestPeakUnion()) {
					for (auto peak : peak_union.second) {
						glVertex3d((GLfloat)normalize(peak->site.p.x, dimension), -(GLfloat)normalize(peak->site.p.y, dimension), 0.0);
					}

				}
				glEnd();

			}
			glEnd();
		}

	}

}


int main() {
	unsigned int nPoints = 10000;
	unsigned int dimension = 1000000;
	double radius = dimension / 2.1;

	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();
	vdg.SetSetting(GenerateSetting(MapType::CONTINENT, 0, 0.6666, radius, 0.5, 0.5, 10, radius / 3, radius / 5, 50, radius / 15, radius / 20, 500.f, 0.2f, 0.02f, 1, 0.1));
	




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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		std::clock_t start;
		double duration;

		if (startOver) {
			std::cout << "\nUsage - When the OpenGL window has focus:\n"
				"\tPress 'R' to perform Lloyd's relaxation once.\n"
				"\tPress 'T' to perform Lloyd's relaxation ten times.\n"
				"\tPress 'Y' to toggle continuous Lloyd's relaxation.\n"
				"\tPress 'S' Save the generated image.\n"
				"\tPress 'D' Shows or hides the dot of cells.\n"
				"\tPress 'L' Shows or hides the edge line of cells.\n"
				"\tPress 'SHIFT + D' Shows or hides the special dot of cells..\n"
				"\tPress 'Esc' to exit.\n\n";
			relaxForever = false;
			Relax = 0;
		}
		


	

		if (Relax || relaxForever || startOver) {
			startOver = false;

			GenerateSetting& setting = vdg.GetSetting();
			vdg.CreateSite(dimension, nPoints);

			start = std::clock();

			vdg.Compute();
			vdg.RepeatRelax(3);
			vdg.CreateWorld();
			
			//Heightmap* map = vdg.CreateHeightmap(ALL_IMAGE, IMAGE_WIDTH, IMAGE_HEIGHT);
			//delete map;

			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
			std::cout << "Computing a diagram of " << nPoints << " points(seed=" << setting.GetSeed() <<") took " << duration << "ms.\n";

			--Relax;
			if (Relax < 0) Relax = 0;
			setting.SetSeed(setting.GetSeed() + 1);
		}
		
		
		
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
			vdg.SaveAllImage(IMAGE_WIDTH, IMAGE_HEIGHT);

			vdg.Save16BitGrayscalePNG(VoronoiDiagramGenerator::ISLAND, "voronoi_map_island_grayscale.png", IMAGE_WIDTH, IMAGE_HEIGHT);
			vdg.Save16BitGrayscalePNG(VoronoiDiagramGenerator::LAKE, "voronoi_map_lake_grayscale.png", IMAGE_WIDTH, IMAGE_HEIGHT);
			vdg.Save16BitGrayscalePNG(VoronoiDiagramGenerator::RIVER, "voronoi_map_river_grayscale.png", IMAGE_WIDTH, IMAGE_HEIGHT);
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
