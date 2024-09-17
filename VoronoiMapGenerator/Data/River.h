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
	int elevation;
	unsigned int power;
	Point2 point;
	RiverPoint(unsigned int pow, int elev, Cell* c, Point2 p = Point2(0, 0)) : cell(c), elevation(elev), power(pow) {
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

	double GetRiverWidth(Diagram* diagram);
};


struct RiverDrawBox {
	Point2& mPreMain;
	const Point2& mNextMain;
	
	Point2& mPreLeft;
	Point2& mNextLeft;

	Point2& mPreRight;
	Point2& mNextRight;

	Point2& mPreNorm;
	Point2& mNextNorm;

	VertexColor& mPreColor;
	const VertexColor& mNextColor;

	RiverDrawBox(
		Point2& preMain, Point2& nextMain, Point2& preLeft,
		Point2& nextLeft, Point2& preRight, Point2& nextRight,
		Point2& preNorm, Point2& nextNorm,
		VertexColor& preColor, VertexColor& nextColor)
		: mPreMain(preMain)
		, mNextMain(nextMain)
		, mPreLeft(preLeft)
		, mNextLeft(nextLeft)
		, mPreRight(preRight)
		, mNextRight(nextRight)
		, mPreNorm(preNorm)
		, mNextNorm(nextNorm)
		, mPreColor(preColor)
		, mNextColor(nextColor)
	{

	}

	void UpdatePrePoint() {
		mPreMain = mNextMain;
		mPreLeft = mNextLeft;
		mPreRight = mNextRight;
		mPreNorm = mNextNorm;
		mPreColor = mNextColor;
	}
};

class RiverTriangle {
private:
	static void AddCurvedTri(Triangles& tris, RiverPointVector& point, RiverDrawBox& drawBox, double& pre_angle_left, double& pre_angle_right, double t, double radius, double spacing, bool is_end = false);
	static VertexColor CalcColor(Diagram* diagram, VertexColor& pre_c, RiverPoint& pre_p, RiverPoint& next_p, const double& color_rate, double t);
public:
	static const double matrix_2[3][3];
	static const double matrix_3[4][4];

	// https://m.blog.naver.com/mykepzzang/220578028540
	// https://wtg-study.tistory.com/101
	// https://blog.naver.com/mykepzzang/220578097319
	// https://wtg-study.tistory.com/102

	static void CalcCardinal(RiverPointVector& point, double result[][2], int start = 0);
	static void CalcSpline(RiverPointVector& point, double result[][2], int cubic_case = 0);

	static void CalcCardinalEdge(RiverPointVector& point, RiverPointVector& left_point, RiverPointVector& right_point, Diagram* diagram, double main_result[][2], double left_result[][2], double right_result[][2], int start = 0);
	static void CalcSplineEdge(RiverPointVector& points, RiverPointVector& left_points, RiverPointVector& right_points, Diagram* diagram, double main_result[][2], double start_cardinal_result[][2], double end_cardinal_result[][2]);
	static void AddPoint(RiverPointVector& left_points, RiverPointVector& right_points, RiverPoint& point, Diagram* diagram, Point2 dir);
	
	static Point2 GetCardinalPoint(double result[][2], double t);
	static Point2 GetCardinalDirection(double result[][2], double t);

	static Point2 GetSplinePoint(double result[][2], double t);
	static Point2 GetSplineDirection(double result[][2], double t);


	static void CreateLineTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in = false, bool fade_out = false);
	static void CreateCardinalTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in = false, bool fade_out = false);
	static void CreateSplineTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in = false, bool fade_out = false);
	static void DrawCircle(Triangles& tris, Point2 center, const int num_segments, const double start, const int end, double radius, VertexColor color);
	
};

class RiverLine {

	bool used;
	Diagram* diagram;
	RiverPointVector points;
	Triangles tris;

	RiverLine(Diagram* l_diagram)
		: used(false)
		, diagram(l_diagram)
	{};

	RiverLine(const RiverLine& other)
		: used(false)
		, diagram(other.diagram)
		, points(other.points)
		, tris(other.tris)
	{
		//std::cout << "복사 생성자 호출 " << points.size() << std::endl;
	}


public:

	Diagram* GetDiagram();
	// Create new RiverLine.
	static RiverLine* Create(Diagram* l_diagram);
	// Copy other.
	static RiverLine* Create(const RiverLine& other);
	// Sets that it has been used
	void SetUsed();
	bool GetUsed();

	void CreateTriangle(double color_rate);
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
	void RepairPoint();
};

class RiverCrossing;

class RiverCrossing {

	//RiverCrossingMap RIVER_CROSSING_MAP = RiverCrossingMap();
	std::vector<RiverLine*> inputs;
	std::vector<RiverLine*> outputs;
	Cell* cell;
	Triangles tris;
public:
	RiverCrossing() {};
	RiverCrossing(Cell* c) : cell(c) {};

	//void Initialize(Diagram* diagram);
	Cell* GetCell();
	Triangles& GetTriangle();

	std::vector<RiverLine*>& GetInputs() { return inputs; };
	std::vector<RiverLine*>& GetOutputs() { return outputs; };
};

class RiverCrossingMap {
	Diagram* mDiagram;
	std::unordered_map<unsigned int, RiverCrossing> mMap;
public:

	RiverCrossingMap() :mDiagram(nullptr) {};
	RiverCrossingMap(Diagram* diagram) : mDiagram(diagram) {};
	std::unordered_map<unsigned int, RiverCrossing>& GetMap() {
		return mMap;
	}
	void Initialize(Diagram* diagram);

	void Clear();
	Diagram* GetDiagram();
	RiverCrossing* Get(unsigned int cell_unique);
	void AddRiver(RiverLine* river);
	void CreateCrossingPointTriagle(Diagram* diagram, double color_rate);
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

	Diagram* mDiagram;
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
		mDiagram = nullptr;
	}
	void Initialize(Diagram* diagram);
	Diagram* GetDiagram();

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
	void AddLine(RiverLine* line);
	std::vector<RiverLine*>& GetArray();
	void AdjustPoint();
	void RepairPoint();
	void CreateTriagle(double color_rate);
};