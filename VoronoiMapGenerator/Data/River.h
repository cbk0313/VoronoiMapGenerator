#pragma once
#include <vector>
#include "Buffer.h"
#include "../Cell.h"
#include "Triangle.h"

class RiverEdge;
class RiverLines;
struct Cell;
template<template<typename> class Q, typename T, typename ...args>
class UniqueBuffer;

struct RiverPoint;

class GenerateSetting;

class Diagram;

namespace River {
	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator() (const std::pair<T1, T2>& p) const {
			auto hash1 = std::hash<T1>{}(p.first);
			auto hash2 = std::hash<T2>{}(p.second);
			return hash1 ^ hash2; // 해시 값의 XOR를 사용하여 결합
		}
	};

	// 사용자 정의 비교 함수 (필요한 경우)
	struct pair_equal {
		template <class T1, class T2>
		bool operator() (const std::pair<T1, T2>& p1, const std::pair<T1, T2>& p2) const {
			return p1.first == p2.first && p1.second == p2.second;
		}
	};

	struct RiverPriorityComp {
		bool operator() (Cell* A, Cell* B) {
			if (A->GetDetail().GetTerrain() == Terrain::COAST && B->GetDetail().GetTerrain() == Terrain::COAST) {
				if (A->GetDetail().GetElevation() < B->GetDetail().GetElevation()) {
					return false;
				}
				else {
					return true;
				}
			}
			else {
				if (A->GetDetail().GetElevation() < B->GetDetail().GetElevation()) {
					return true;
				}
				else {
					if (A->GetDetail().GetElevation() == B->GetDetail().GetElevation()) {
						if (A->GetDetail().GetMoisture() < B->GetDetail().GetMoisture()) {
							return true;
						}
					}
					return false;
				}
			}
		}

		bool operator() (std::pair<Cell*, Cell*>& A, std::pair<Cell*, Cell*>& B) {
			
			if (A.first->GetDetail().GetTerrain() == Terrain::COAST && B.first->GetDetail().GetTerrain() == Terrain::COAST) {
				if (A.first->GetDetail().GetElevation() < B.first->GetDetail().GetElevation()) {
					return false;
				}
				else {
					return true;
				}
			}
			else {

				if (A.first->GetDetail().GetElevation() < B.first->GetDetail().GetElevation()) {
					return true;
				}
				else {
					if (A.first->GetDetail().GetElevation() == B.first->GetDetail().GetElevation()) {
						if (A.first->GetDetail().GetMoisture() < B.first->GetDetail().GetMoisture()) {
							return true;
						}
					}
					return false;
				}
			}
		}

	};



}


using RiverContainer = std::pair<Cell*, Cell*>;


using RiverPos = std::pair<int, int>;
using RiverEdgeMap = std::unordered_map<RiverPos, RiverEdge*, River::pair_hash, River::pair_equal>;
using RiverOutMap = std::unordered_map<unsigned int, std::vector<RiverEdge*>>;
using RiverLinkMap = std::unordered_map<unsigned int, std::vector<Cell*>>;
using RiverCntMap = std::unordered_map<unsigned int, unsigned int>;

using RiverPointVector = std::vector<RiverPoint>;


class RiverEdge {

	Diagram* diagram;

	bool is_start;
	bool is_end;
	Cell* start; // 나중에 유니크 번호로 변경?
	Cell* end;
	Cell* owner;
	RiverLines* river_lines;
	int dist;
	int power;
	std::vector<RiverEdge*> prevs;
	std::vector<RiverEdge*> nexts;
	std::vector<Cell*> links;

public:


	//RiverEdge() : is_start(false), start(nullptr), end(nullptr), dist(0) {};
	RiverEdge(Diagram* l_diagram, RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance)
		: is_start(false), is_end(false), start(nullptr), end(nullptr), owner(nullptr), river_lines(nullptr), dist(0), power(0)
	{
		Initialize(l_diagram, lines, startCell, endCell, river_owner, pre_edge, next_edge, distance);
	};

	void Initialize(Diagram* l_diagram, RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
	int GetDistance() { return dist; };
	Cell* GetOnwer();
	void SetOnwer(Cell* c);
	std::vector<RiverEdge*>& GetPrevs();
	void AddPrev(RiverEdge* e);
	std::vector<RiverEdge*>& GetNexts();
	void AddNext(RiverEdge* e);


	int GetPower() { return power; };
	void SetPower(int num) { power = num; };
	bool IsStart() { return is_start; };
	void SetRiverEnd(bool b) { is_end = b; };
	bool IsEnd() { return is_end; };
	Cell* GetStart() { return start; };
	Cell* GetEnd() { return end; };
	void SetStart(Cell* c) { start = c; };
	void SetEnd(Cell* c) { end = c; };
	void ChangeDist(int num);
	void RefreshDistAndOwner(int num, Cell* river_owner);
	void SetDist(int num);
	void DeleteLine(/*std::vector<bool>& buf*/);
	static RiverEdge* Create(Diagram* l_diagram, RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
	static RiverEdge* CreateStartPoint(Diagram* l_diagram, RiverLines* lines, Cell* c);
	static RiverPos GetPos(Cell* start, Cell* end);

	void ConnectPrev(RiverEdge* e);

	RiverEdge* GetOwnerEdge();

};


struct RiverPoint {

	Cell* cell;
public:
	unsigned int power;
	Point2 point;
	RiverPoint(unsigned int pow, Cell* c, Point2 p = Point2(0, 0)) : power(pow), cell(c) {
		if (c != nullptr) {
			point = c->site.p;
		}
		else {
			point = p;
		}
	};

	Cell* GetCell() {
		return cell;
	}
};



class RiverTriangle {
private:
	static void AddCurvedTri(Triangles& tris, RiverPointVector& point, VertexColor& color, VertexColor c_trans, Point2& cul_norm, Point2& next_norm, Point2& new_p, Point2& pre_p, double& pre_angle_left, double& pre_angle_right, double t, double radius, double scale1, double scale2, double spacing, bool is_end = false);
public:
	static const double matrix_2[3][3];
	static const double matrix_3[4][4];

	// https://m.blog.naver.com/mykepzzang/220578028540
	// https://wtg-study.tistory.com/101
	// https://blog.naver.com/mykepzzang/220578097319
	// https://wtg-study.tistory.com/102

	static void CalcCardinal(RiverPointVector& point, double result[][2], int start = 0);
	static Point2 GetCardinalPoint(double result[][2], double t);
	static Point2 GetCardinalDirection(double result[][2], double prev, double next);

	static void CreateLineTri(Triangles& tris, RiverPointVector& point, VertexColor& color, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false);
	static void CreateCardinalTri(Triangles& tris, RiverPointVector& point, VertexColor& color, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false);
	static void CreateSplineTri(Triangles& tris, RiverPointVector& point, VertexColor& color, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false);
	static void DrawCircle(Triangles& tris, Point2 center, const int num_segments, const double start, const int end, double radius, double river_scale, double power, VertexColor color);

};
class RiverLine {



	bool used;
	Diagram* diagram;
	RiverPointVector points;
	Triangles tris;
	//double radius;
	//double power_sacle;
	//double curv_spacing;

	GenerateSetting& main_setting;

	RiverLine(Diagram* l_diagram, GenerateSetting& setting)
		: used(false)
		, diagram(l_diagram)
		, main_setting(setting)
		//, radius(_radius)
		//, power_sacle(_power_sacle)
		//, curv_spacing(0.02f)

	{};

	RiverLine(const RiverLine& other)
		: used(false)
		, diagram(other.diagram)
		, points(other.points)
		, tris(other.tris)
		, main_setting(other.main_setting)
	{
		//std::cout << "복사 생성자 호출 " << points.size() << std::endl;
	}


public:


	// Create new RiverLine.
	static RiverLine* Create(Diagram* l_diagram, GenerateSetting& setting);
	// Copy other.
	static RiverLine* Create(const RiverLine& other);
	// Sets that it has been used
	void SetUsed();
	bool GetUsed();

	void CreateTriangle(VertexColor& c);
	// If no triangle was created, get an empty vector.
	Triangles& GetTriangle();
	// Add a point where a river flows. 
	void AddPoint(RiverPoint p);
	// Get an array of RiverPoint vector.
	RiverPointVector& GetPointArray();
	// Get the value corresponding to the index of the array
	RiverPoint& GetPoint(unsigned int index);
	RiverPoint& GetFirstPoint();
	RiverPoint& GetEndPoint();
	void AdjustPoint();

};

class RiverCrossing;
using RiverCrossingMap = std::unordered_map<unsigned int, RiverCrossing>;

class RiverCrossing {

	RiverCrossingMap RIVER_CROSSING_MAP = RiverCrossingMap();
	std::vector<RiverLine*> inputs;
	std::vector<RiverLine*> outputs;
	Triangles tris;
	Cell* cell;

public:
	RiverCrossing() {};
	RiverCrossing(Cell* c) : cell(c) {};

	void Clear();
	Cell* GetCell();
	RiverCrossing* Get(unsigned int cell_unique);
	void AddRiver(RiverLine* river);
	void CreateCrossingPointTriagle(VertexColor& color, double radius, double river_scale, double spacing);
	Triangles GetTriangle();
};


class RiverLines {
	friend class RiverEdge;

	double CurveChance;
	double CurveDistance;

	std::vector<RiverLine*> lines;

	RiverEdgeMap RIVER_EDGES_MAP;
	RiverOutMap RIVER_OUT_MAP;
	RiverLinkMap LINKED_RIVER_EDGES;
	RiverLinkMap LINKED_RIVERS;
	RiverCntMap RIVER_CNT;

	GenerateSetting* Setting;
public:
	RiverLines()
		: CurveChance(0)
		, CurveDistance(0)
	{
		RIVER_EDGES_MAP = RiverEdgeMap();
		RIVER_OUT_MAP = RiverOutMap();
		LINKED_RIVER_EDGES = RiverLinkMap();
		LINKED_RIVERS = RiverLinkMap();
		RIVER_CNT = RiverCntMap();
		Setting = nullptr;
	}
	void Initialize(GenerateSetting& setting);

	void AddOceanConnect(Cell* c);
	int GetOceanConnect(Cell* c);


	void Clear();
	RiverEdgeMap& GetRiverEdges();
	RiverOutMap& GetRiverOutEdges();
	RiverEdge* GetRiverEdge(RiverPos pos);
	std::vector<RiverEdge*>& GetRiverOutEdge(unsigned int num);
	std::vector<Cell*>& GetLinkedRiverEdges(unsigned int num);
	void AddLinkRiverEdge(Cell* lakeA, Cell* lakeB);
	bool CheckRiverEdgeLinked(Cell* lakeA, Cell* lakeB);
	std::vector<Cell*>& GetLinkedRiver(unsigned int num);
	void AddLinkRiver(Cell* lakeA, Cell* lakeB);
	bool CheckRiverLinked(Cell* lakeA, Cell* lakeB);
	// Depending on settings, can add random points between points to make the river appear to flow more naturally.
	void AddLine(RiverCrossing* corssing, RiverLine* line);
	std::vector<RiverLine*>& GetArray();
	void AdjustPoint();
	void CreateTriagle(VertexColor& c);
};