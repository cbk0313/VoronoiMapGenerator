#pragma once

#include "RBTree.h"
#include "CircleEventQueue.h"
#include "BeachLine.h"
#include "Diagram.h"
#include <vector>
#include "Data/River.h"

#include "Data/Setting.h"


#if !IS_UNREAL_PLUIN
#define UVoronoiDiagramGenerator VoronoiDiagramGenerator
#endif 



struct BoundingBox {
	double xL;
	double xR;
	double yB;
	double yT;

	BoundingBox() : xL(0), xR(0), yB(0), yT(0) {};
	BoundingBox(double xmin, double xmax, double ymin, double ymax) :
		xL(xmin), xR(xmax), yB(ymin), yT(ymax) {};
};


#define ALL_IMAGE ((VoronoiDiagramGenerator::ISLAND | VoronoiDiagramGenerator::OCEAN | VoronoiDiagramGenerator::LAKE | VoronoiDiagramGenerator::COAST | VoronoiDiagramGenerator::RIVER))

class Heightmap;
struct CellVector;
class VoronoiDiagramGenerator {
public:
	enum ImageFlag {
		ISLAND			= 0b00000001,
		OCEAN			= 0b00000010,
		LAKE			= 0b00000100,
		COAST			= 0b00001000,
		RIVER			= 0b00010000,
		ISLAND_PAINT	= 0b00100000,
		OCEAN_PAINT		= 0b01000000,
		COAST_PAINT		= 0b10000000,
	};

private:

	bool has_created_ocean;
	bool has_set_color;

	int image_flag;

	int max_elevation;
	int min_elevation;
	unsigned int max_moisture;

	Diagram* diagram;
	//;
	CircleEventQueue* circleEventQueue;
	//std::vector<Point2*>* siteEventQueue;
	std::vector<Point2> sites;
	BoundingBox	boundingBox;
	GenerateSetting setting;
	//BoundingBox bbox;


	void SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, VertexColor& elev_rate_c);

	//BeachLine
	RBTree<BeachSection>* beachLine;
	treeNode<BeachSection>* addBeachSection(Site* site);
	inline void detachBeachSection(treeNode<BeachSection>* section);
	void removeBeachSection(treeNode<BeachSection>* section);
	double leftBreakpoint(treeNode<BeachSection>* section, double directrix);
	double rightBreakpoint(treeNode<BeachSection>* section, double directrix);

	void SetupOcean();

	void CreateLand();
	void CreateTestLand();
	void RemoveLake();
	void CreateLake();
	void SetupElevation(CellVector& coastBuffer);
	void SetupPeak(CellVector& coastBuffer);
	void ExpandCoast(CellVector& coastBuffer);
	void SetupCoast(CellVector& coastBuffer);
	void SetupLandUnion();
	void SetupIsland();
	void SetupBiome();
	void CreateRiver();
	void SetupMoisture();
	void SetupOceanDepth(CellVector& coastBuffer);
	void SetupEdgePos(bool trans_edge);
	void SetupColor(int flag = ALL_IMAGE);
	void SetupRiverTriangle(VertexColor c);
	void CreateTriangle();

	std::pair<double, double> GetMinDist(std::vector<std::pair<Point2, double>>& points, Point2& center, double radius);

public:
	VoronoiDiagramGenerator() : has_created_ocean(false), has_set_color(false), image_flag(ALL_IMAGE), max_elevation(1), min_elevation(-1), max_moisture(0), diagram(nullptr), circleEventQueue(nullptr), boundingBox(BoundingBox()), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* GetDiagram();

	int GetMaxElevation() { return max_elevation; }
	unsigned int GetMaxMoisture() { return max_moisture; }
	int GetMinElevation() { return min_elevation; }

	// Create a site. Must be run first
	void CreateSite();
	// Create cells according to the created site
	void Compute();
	// Makes cells uniform in size. The more run it, the more even it becomes.
	void Relax();
	// Repeat 'Relax()' num times
	void RepeatRelax(int num);

	void CreateWorld(bool trans_edge = true, bool create_tri = true);

	inline void SetSetting(GenerateSetting newSetting) { setting = newSetting; };
	inline GenerateSetting& GetSetting() { return setting; };

	//void printBeachLine();


	void SaveAllImage(unsigned int w, unsigned int h);
	void SaveImage(int flag, const char* filename, unsigned int w, unsigned int h, bool restore = true);
	unsigned char* GetImage(int flag, unsigned int w, unsigned int h, bool restore = true);
	//void SaveGrayscaleImage(Heightmap& map, const char* filename, int width, int height);
	void CreateHeightmap(Heightmap& out_map, bool clear_image, int flag, unsigned int w, unsigned int h, bool restore = true);
};




inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

