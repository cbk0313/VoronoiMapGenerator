#ifndef _VORONOI_DIAGRAM_GENERATOR_H_
#define _VORONOI_DIAGRAM_GENERATOR_H_

#include "RBTree.h"
#include "CircleEventQueue.h"
#include "BeachLine.h"
#include "Diagram.h"
#include <vector>

enum class MapType {
	CONTINENT,
	ISLAND
};

struct BoundingBox {
	double xL;
	double xR;
	double yB;
	double yT;

	BoundingBox() {};
	BoundingBox(double xmin, double xmax, double ymin, double ymax) :
		xL(xmin), xR(xmax), yB(ymin), yT(ymax) {};
};

struct GenerateSetting {
	friend class VoronoiDiagramGenerator;
private:
	MapType type;
	int seed;
	double radius; 
	double lake_scale; 
	double lake_size; 

	unsigned int island_cnt; 
	double island_radius_max;
	double island_radius_min;

	unsigned int lake_cnt;
	double lake_radius_max;
	double lake_radius_min;
public:
	GenerateSetting() 
		: type(MapType::CONTINENT)
		, seed(0)
		, radius(500000) // radius, 0, 0.7, 10, radius / 3, radius / 5, 10, radius / 5, radius / 7)
		, lake_scale(0)
		, lake_size(0.7)
		, island_cnt(10)
		, island_radius_max(333333)
		, island_radius_min(200000)
		, lake_cnt(10)
		, lake_radius_max(200000)
		, lake_radius_min(142857.1428571429) {};
	GenerateSetting(MapType _type, int _seed, double _radius, double _lake_scale, double _lake_size, unsigned int _island_cnt, double _island_radius_max, double _island_radius_min, unsigned int _lake_cnt, double _lake_radius_max, double _lake_radius_min)
		: type(_type)
		, seed(_seed)
		, radius(_radius)
		, lake_scale(_lake_scale)
		, lake_size(_lake_size)
		, island_cnt(_island_cnt)
		, island_radius_max(_island_radius_max)
		, island_radius_min(_island_radius_min)
		, lake_cnt(_lake_cnt)
		, lake_radius_max(_lake_radius_max)
		, lake_radius_min(_lake_radius_min)
	{};


	inline void SetMapType(MapType new_type) { type = new_type; };
	inline void SetSeed(int new_seed) { seed = new_seed; };
	inline void SetRadius(double new_radius) { radius = new_radius; };
	inline void SetLakeScale(double new_scale) { lake_scale = new_scale; };
	inline void SetLakeSize(double new_size) { lake_size = new_size; };
	inline void SetIslandCount(unsigned int new_cnt) { island_cnt = new_cnt; };
	inline void SetIslandRadiusMax(double new_radius) { island_radius_max = new_radius; };
	inline void SetIslandRadiusMin(double new_radius) { island_radius_min = new_radius; };
	inline void SetLakeCount(unsigned int new_cnt) { lake_cnt = new_cnt; };
	inline void SetLakeRadiusMax(double new_radius) { lake_radius_max = new_radius; };
	inline void SetLakeRadiusMin(double new_radius) { lake_radius_min = new_radius; };

	inline MapType GetMapType() { return type; };
	inline int GetSeed() { return seed; };
	inline double GetRadius() { return radius; };
	inline double GetLakeScale() { return lake_scale; };
	inline double GetLakeSize() { return lake_size; };
	inline unsigned int GetIslandCount() { return island_cnt; };
	inline double GetIslandRadiusMax() { return island_radius_max; };
	inline double GetIslandRadiusMin() { return island_radius_min; };
	inline unsigned int GetLakeCount() { return lake_cnt; };
	inline double GetLakeRadiusMax() { return lake_radius_max; };
	inline double GetLakeRadiusMin() { return lake_radius_min; };

};
struct CellVector;
class VoronoiDiagramGenerator {

private:

	bool has_created_ocean;
	//;
	CircleEventQueue* circleEventQueue;
	//std::vector<Point2*>* siteEventQueue;
	BoundingBox	boundingBox;
	GenerateSetting setting;
	int max_elevation;
	Diagram* diagram;

	void SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, Color& elev_rate_c);

public:
	VoronoiDiagramGenerator() : diagram(nullptr), max_elevation(0), has_created_ocean(false), circleEventQueue(nullptr), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* GetDiagram();

	void compute(std::vector<Point2>& sites, BoundingBox bbox, bool reset = true);
	void relax();

	void relaxLoop(int num);

	void CreateWorld();

	inline void SetSetting(GenerateSetting newSetting) { setting = newSetting; };
	inline GenerateSetting& GetSetting() { return setting; };

	void printBeachLine();

	//BeachLine
	RBTree<BeachSection>* beachLine;
	treeNode<BeachSection>* addBeachSection(Site* site);
	inline void detachBeachSection(treeNode<BeachSection>* section);
	void removeBeachSection(treeNode<BeachSection>* section);
	double leftBreakpoint(treeNode<BeachSection>* section, double directrix);
	double rightBreakpoint(treeNode<BeachSection>* section, double directrix);

	inline double CalcDistance(const Point2& a, const Point2& b);
	inline double GetRandom();

	void SetupOcean();

	void CreateLand();
	void CreateTestLand();
	void RemoveLake();
	void CreateLake();
	void CreateRiver();
	void SetupElevation(CellVector& coastBuffer);
	void SetupPeak(CellVector& coastBuffer);
	void SetupCoast(CellVector& coastBuffer);
	void SetupLandUnion();
	void SetupIsland();
	void SetupBiome();
	void SetupEdgePos();
	void SetupColor();

	void SaveImage(double dimension, double w, double h);

	std::pair<double, double> GetMinDist(std::vector<std::pair<Point2, double>>& points, Point2& center, double radius);
	
};

inline double VoronoiDiagramGenerator::CalcDistance(const Point2& a, const Point2& b) {
	double x = (b.x - a.x);
	double y = (b.y - a.y);
	return sqrt((x * x) + (y * y));
}

// Returns a real number between 0 and 1
inline double VoronoiDiagramGenerator::GetRandom() {
	return rand() / ((double)RAND_MAX);
}


inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

#endif