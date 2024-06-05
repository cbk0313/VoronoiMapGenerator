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
	

	void CreateRiverPoly() {

	}

	


	double matrix_2[3][3] = { {2.0f, -4.0f, 2.0f}, {-3.0f, 4.0f, -1.0f}, {1.0f, 0.0f, 0.0f} };
	double matrix_3[4][4] = { {-1.0f, 3.0f, -3.0f, 1.0f},
							  {2.0f, -5.0f, 4.0f, -1.0f},
							  {-1.0f, 0.0f, 1.0f, 0.0f},
							  {0.0f, 2.0f, 0.0f, 0.0f} };

	void drawCardinal(std::vector<RiverPoint>& point, std::vector<RiverDraw>& draw_arr) {
		// init
		double result[3][2];
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

		// 행렬 (1) 과 새로운 행렬 result 의 곱
		double t = 0.0f;
		double x, y;
		std::vector<Point2> p_arr;
		while (t < 1) {
			/*if (t < 0.5) {

				if (point[0]->GetDetail().GetElevation() == point[1]->GetDetail().GetElevation()) {
					glColor4f(0, 1, 0, 1);
				}
				else {
					glColor4f(1, 0, 0, 1);
				}
			}
			else {
				if (point[1]->GetDetail().GetElevation() == point[2]->GetDetail().GetElevation()) {
					glColor4f(0, 1, 0, 1);
				}
				else {
					glColor4f(1, 0, 0, 1);
				}
			}*/
			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);
			p_arr.push_back(Point2(x, y));
			t += 0.01f;
		}

		for (int i = 1; i < p_arr.size(); i++) {
			double pow1 = point[0].power * (1 - (i - 1) / p_arr.size()) + point[1].power * (((i - 1) / p_arr.size()));
			double pow2 = point[0].power * (1 - (i) / p_arr.size()) + point[1].power * (((i) / p_arr.size()));
			draw_arr.push_back(RiverDraw(RiverPoint(pow1, nullptr, p_arr[i - 1]), RiverPoint(pow2, nullptr, p_arr[i])));
		}

	}

	void draw_spline(std::vector<RiverPoint>& point, std::vector<RiverDraw>& draw_arr) {
		double result[4][2];
		double t = 0.0f;
		double x, y;
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


		//if (point[0]->GetDetail().GetElevation() == point[1]->GetDetail().GetElevation()) {
		//	glColor4f(0, 1, 0, 1);
		//}
		//else {
		//	glColor4f(1, 0, 0, 1);
		//}

		std::vector<Point2> p_arr;
		while (t < 0.5f) {
			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);
			p_arr.push_back(Point2(x, y));
			t += 0.01f;
		}

		for (int i = 1; i < p_arr.size(); i++) {
			double pow1 = point[0].power * (1 - (i - 1) / p_arr.size()) + point[1].power * (((i - 1) / p_arr.size()));
			double pow2 = point[0].power * (1 - (i) / p_arr.size()) + point[1].power * (((i) / p_arr.size()));
			draw_arr.push_back(RiverDraw(RiverPoint(pow1, nullptr, p_arr[i - 1]), RiverPoint(pow2, nullptr, p_arr[i])));
		}

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

			p_arr.clear();
			t = 0.0f;
			while (t < 1.0f) {
				x = (result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f;
				y = (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f;
				p_arr.push_back(Point2(x, y));
				t += 0.01f;
			}

			for (int i = 1; i < p_arr.size(); i++) {
				double pow1 = point[cubic_case + 1].power * (1 - (i - 1) / p_arr.size()) + point[cubic_case + 2].power * (((i - 1) / p_arr.size()));
				double pow2 = point[cubic_case + 1].power * (1 - (i) / p_arr.size()) + point[cubic_case + 2].power * (((i) / p_arr.size()));
				draw_arr.push_back(RiverDraw(RiverPoint(pow1, nullptr, p_arr[i - 1]), RiverPoint(pow2, nullptr, p_arr[i])));
			}


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
		p_arr.push_back(Point2(x, y));
		t = 0.5f;

		while (t < 1.0f) {
			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);
			p_arr.push_back(Point2(x, y));
			t += 0.01f;
		}

		for (int i = 1; i < p_arr.size(); i++) {
			double pow1 = point[0].power * (1 - (i - 1) / p_arr.size()) + point[1].power * (((i - 1) / p_arr.size()));
			double pow2 = point[0].power * (1 - (i) / p_arr.size()) + point[1].power * (((i) / p_arr.size()));
			draw_arr.push_back(RiverDraw(RiverPoint(pow1, nullptr, p_arr[i - 1]), RiverPoint(pow2, nullptr, p_arr[i])));
		}

	}
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