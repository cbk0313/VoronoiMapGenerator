#ifndef _VORONOI_DIAGRAM_GENERATOR_H_
#define _VORONOI_DIAGRAM_GENERATOR_H_

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

struct CellVector;
class VoronoiDiagramGenerator {
public:
	enum ImageFlag {
		ISLAND		= 0b00001,
		OCEAN		= 0b00010,
		LAKE		= 0b00100,
		COAST		= 0b01000,
		RIVER		= 0b10000,
	};

private:

	bool has_created_ocean;
	bool has_set_color;

	int image_flag;

	int max_elevation;
	unsigned int max_moisture;
	Diagram* diagram;
	//;
	CircleEventQueue* circleEventQueue;
	//std::vector<Point2*>* siteEventQueue;
	std::vector<Point2> sites;
	BoundingBox	boundingBox;
	GenerateSetting setting;
	//BoundingBox bbox;


	void SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, Color& elev_rate_c);

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
	void SetupCoast(CellVector& coastBuffer);
	void SetupLandUnion();
	void SetupIsland();
	void SetupBiome();
	void CreateRiver();
	void SetupMoisture();
	void SetupEdgePos();
	void SetupColor(int flag = ALL_IMAGE);
	void SetupRiverTriangle(Color c);
	void CreateTriangle();

public:
	VoronoiDiagramGenerator() : has_created_ocean(false), has_set_color(false), image_flag(ALL_IMAGE), max_elevation(0), max_moisture(0), diagram(nullptr), circleEventQueue(nullptr), boundingBox(BoundingBox()), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* GetDiagram();

	unsigned int GetMaxElevation() { return max_elevation; }
	unsigned int GetMaxMoisture() { return max_moisture; }

	void CreateSite(unsigned int dimension, unsigned int numSites);

	void Compute(bool reset = true);
	void Relax();

	void RelaxLoop(int num);

	void CreateWorld(bool create_tri = true);

	inline void SetSetting(GenerateSetting newSetting) { setting = newSetting; };
	inline GenerateSetting& GetSetting() { return setting; };

	void printBeachLine();



	void SaveAllImage(double dimension, unsigned int w, unsigned int h);
	void SaveImage(const char* filename, double dimension, unsigned int w, unsigned int h);

	std::pair<double, double> GetMinDist(std::vector<std::pair<Point2, double>>& points, Point2& center, double radius);
	
};




inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

#endif