#include "Point2.h"
#include "Vector2.h"
#include "VoronoiDiagramGenerator.h"
#include <vector>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <limits>

#include "FastNoise/FastNoiseLite.h"

// GLEW
#define GLEW_STATIC
#include "../lib/GLEW/glew.h"

// GLFW
#include "../lib/GLFW/glfw3.h"
// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// Window dimensions
const GLuint WINDOW_WIDTH = 1200, WINDOW_HEIGHT = 1200;
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
	numSites = sqrt(numSites);
	int pow_site = pow(numSites, 2);
	tmpSites.reserve(pow_site);
	sites.reserve(pow_site);

	Point2 s;

	double step = dimension / numSites;
	double half_step = step * 0.9;
	
	srand(seed);
	for (unsigned int i = 0; i < numSites; ++i) {
		for (unsigned int j = 0; j < numSites; ++j) {
			
			s.x = (i * step) + (rand() / ((double)RAND_MAX)) * (half_step);
			s.y = (j * step) + (rand() / ((double)RAND_MAX)) * (half_step);
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

int main() {
	unsigned int nPoints = 1500;
	unsigned int dimension = 1000000;

	int seed = 0;
	double radius = dimension / 2.1;

	unsigned int loop_cnt = 3;
	unsigned int pointSize = 5;

	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();
	Diagram* diagram = nullptr;
	
	std::vector<Point2>* sites = nullptr;
	BoundingBox bbox;

	bool draw_line = true;

	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Voronoi Diagram Generator", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();
	// Define the viewport dimensions
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	// Uncommenting this call will result in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	while (!glfwWindowShouldClose(window)) {
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
			std::cout << "How many points? ";
			//nPoints = 1000;
			//std::cin >> nPoints;
			genRandomSites(seed, *sites, bbox, dimension, nPoints);
			start = std::clock();
			if (diagram) delete diagram;
			diagram = vdg.compute(*sites, bbox);

			diagram = vdg.relaxLoop(loop_cnt, diagram);
			vdg.SetLand(seed, radius, diagram);

			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			delete sites;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	

		for (Edge* e : diagram->edges) {
			if (e->vertA && e->vertB) {
				Point2& p1 = *e->vertA;
				Point2& p2 = *e->vertB;
				glBegin(GL_TRIANGLES);
				glColor4f(e->lSite->cell->detail.color.r, e->lSite->cell->detail.color.g, e->lSite->cell->detail.color.b, e->lSite->cell->detail.color.a);
				glVertex3d(normalize(e->lSite->cell->site.p[0], dimension), -normalize(e->lSite->cell->site.p[1], dimension), 0.0);
				glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
				glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
				glEnd();


				if (e->rSite) {
					glBegin(GL_TRIANGLES);
					glColor4f(e->rSite->cell->detail.color.r, e->rSite->cell->detail.color.g, e->rSite->cell->detail.color.b, e->lSite->cell->detail.color.a);
					glVertex3d(normalize(e->rSite->cell->site.p[0], dimension), -normalize(e->rSite->cell->site.p[1], dimension), 0.0);
					glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
					glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
					glEnd();
				}
			}
		}



		if (draw_line) {
			for (Edge* e : diagram->edges) {
				Point2& p1 = *e->vertA;
				Point2& p2 = *e->vertB;

				glBegin(GL_LINES);
				glColor4f(0, 0, 0, 1);
				glVertex3d(normalize(p1[0], dimension), -normalize(p1[1], dimension), 0.0);
				glVertex3d(normalize(p2[0], dimension), -normalize(p2[1], dimension), 0.0);
				glEnd();
			}
		}

		for (Cell* c : diagram->cells) {
			size_t edgeCount = c->halfEdges.size();
			for (HalfEdge* hf : c->halfEdges) {
				/*if (!hf->edge->check) {
					hf->edge->check = true;
					std::cout << "checked!\n";
				}*/
				//glBegin(GL_LINES);
				//glBegin(GL_TRIANGLES);
				Point2& p1 = *hf->startPoint();
				Point2& p2 = *hf->endPoint();
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
		glPointSize(pointSize);
		glBegin(GL_POINTS);
		for (Cell* c : diagram->cells) {
			Point2& p = c->site.p;
			if (c->detail.unionfind.unionFindCell(static_cast<int>(Terrain::PEAK))->detail.is_peak) {
				glColor4f(0, 0, 0, 1);
			}
			else if (c->detail.is_flat) {
				glColor4f(0.7, 0.7, 0, 1);
			}
			else {
				glColor4f(1, 1, 1, 1);
			}
			
			glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
		}
		glEnd();

		double color = 0;
		glPointSize(pointSize);
		glBegin(GL_POINTS);
		for (auto item : diagram->islandUnion.unions) {
			auto island = item.second;
			/*
			color = 0;
			std::cout << island.lakeUnion.unions.size() << "\n";
			for (auto lake_union : island.lakeUnion.unions) {

				glColor4f(1 - color, 0, color, 1);
				for (auto lake : lake_union.second) {
					Point2& p = lake->site.p;
					glVertex3d(normalize(p.x, dimension), -normalize(p.y, dimension), 0.0);
				}
				color += 0.2;
			}
			color = 0;
			for (auto peak_union : island.peakUnion.unions) {
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

		if (relax || relaxForever) {
			seed++;
			start = std::clock();
			//diagram = vdg.relax();
			//diagram = vdg.relax();
			sites = new std::vector<Point2>();
			genRandomSites(seed, *sites, bbox, dimension, nPoints);
			delete diagram;
			diagram = vdg.compute(*sites, bbox);
			diagram = vdg.relaxLoop(loop_cnt, diagram);
			vdg.SetLand(seed, radius, diagram);
			duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;

			delete sites;

			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			if (diagram->cells.size() != 4) {
				int x = 0;
			}
			--relax;
			if (relax < 0) relax = 0;
		}
		
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by it.
	glfwTerminate();

	delete diagram;
	return 0;
}
