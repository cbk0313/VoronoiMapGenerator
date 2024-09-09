#pragma once

#include "RBTree.h"
#include "CircleEventQueue.h"
#include "BeachLine.h"
#include "Diagram.h"
#include <vector>
#include "Data/River.h"

#include "Data/Setting.h"


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
		ISLAND =		0b000000000001,
		OCEAN =			0b000000000010,
		LAKE =			0b000000000100,
		COAST =			0b000000001000,
		RIVER =			0b000000010000,
		ISLAND_PAINT =	0b000000100000,
		OCEAN_PAINT =	0b000001000000,
		LAKE_PAINT =	0b000010000000,
		COAST_PAINT =	0b000100000000,
		RIVER_PAINT =	0b001000000000,
	};

private:

	bool has_created_ocean;
	bool has_set_color;

	int image_flag;


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
	void SetupLakeElevtion();
	void SetupBiome();
	void CreateRiver();
	void SetupMoisture();
	void SetupOceanDepth(CellVector& coastBuffer);
	void SetupEdgePos(bool trans_edge);
	void SetupColor(int flag = ALL_IMAGE);
	void SetupRiverTriangle();
	void CreateTriangle();

	std::pair<double, double> GetMinDist(std::vector<std::pair<Point2, double>>& points, Point2& center, double radius);

public:
	VoronoiDiagramGenerator() : has_created_ocean(false), has_set_color(false), image_flag(ALL_IMAGE), diagram(nullptr), circleEventQueue(nullptr), boundingBox(BoundingBox()), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* GetDiagram();


	// Create a site. Must be run first
	void CreateSite();
	// Create cells according to the created site
	void Compute();
	// Makes cells uniform in size. The more run it, the more even it becomes.
	void Relax();
	// Repeat 'Relax()' num times
	void RepeatRelax(int num);

	void CreateWorld(int flag = ALL_IMAGE, bool trans_edge = true, bool create_tri = true);

	inline void SetSetting(GenerateSetting newSetting) { setting = newSetting; };
	inline GenerateSetting& GetSetting() { return setting; };

	//void printBeachLine();

	double CalcIslandColorRate();
	double CalcOceanColorRate();
	double CalcIslandGrayRate();
	double CalcOceanGrayRate();

	static VertexColor CalcIslandColor(double elev, double sea_level, double island_elev_rate);
	static VertexColor CalcOceanColor(double elev, double ocean_elev_rate);

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

