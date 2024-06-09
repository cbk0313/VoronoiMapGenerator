#ifndef _VORONOI_DIAGRAM_GENERATOR_H_
#define _VORONOI_DIAGRAM_GENERATOR_H_

#include "RBTree.h"
#include "CircleEventQueue.h"
#include "BeachLine.h"
#include "Diagram.h"
#include <vector>
#include "Data/River.h"

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

class GenerateSetting {
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

	double river_radius;
	double river_power_scale;
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
		, lake_radius_min(142857.1428571429)
		, river_radius(500)
		, river_power_scale(0.2) 
	{};
	GenerateSetting(MapType _type, int _seed, double _radius, double _lake_scale,
		double _lake_size, unsigned int _island_cnt, double _island_radius_max,
		double _island_radius_min, unsigned int _lake_cnt, double _lake_radius_max,
		double _lake_radius_min, double _river_radius, double _river_power_scale)
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
		, river_radius(_river_radius)
		, river_power_scale(_river_power_scale)
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
	inline void SetRiverRadius(double new_radius) { river_radius = new_radius; };
	inline void SetRiverPowerScale(double new_sacle) { river_power_scale = new_sacle; };

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
	inline double GetRiverRadius() { return river_radius; };
	inline double GetRiverPowerScale() { return river_power_scale; };

};

#define ALL_IMAGE ((VoronoiDiagramGenerator::ISLAND | VoronoiDiagramGenerator::OCEAN | VoronoiDiagramGenerator::LAKE | VoronoiDiagramGenerator::COAST))

struct CellVector;
class VoronoiDiagramGenerator {
public:
	enum ImageFlag {
		ISLAND		= 0b00001,
		OCEAN		= 0b00010,
		LAKE		= 0b00100,
		COAST		= 0b01000,
	};

private:

	bool has_created_ocean;
	bool has_set_color;
	//;
	CircleEventQueue* circleEventQueue;
	//std::vector<Point2*>* siteEventQueue;
	BoundingBox	boundingBox;
	GenerateSetting setting;
	int max_elevation;
	unsigned int max_moisture;
	Diagram* diagram;

	void SetupVertexColor(Vertex* v, Cell* c, Cell* opposite_c, Color& elev_rate_c);


public:
	VoronoiDiagramGenerator() : diagram(nullptr), max_elevation(0), max_moisture(0), has_created_ocean(false), has_set_color(false), circleEventQueue(nullptr), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* GetDiagram();

	unsigned int GetMaxElevation() { return max_elevation; }
	unsigned int GetMaxMoisture() { return max_moisture; }

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

	void SaveAllImage(double dimension, double w, double h);
	void SaveImage(const char* filename, double dimension, double w, double h);

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