#pragma once
#include <vector>
#include "Data/Buffer.h"
#include "../Cell.h"
#include "Triangle.h"

struct RiverEdge;
struct Cell;
template<template<typename> class Q, typename T, typename ...args>
class UniqueBuffer;

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
			if (A->GetDetail().GetElevation() < B->GetDetail().GetElevation()) {
				return true;
			}
			else {
				if (A->GetDetail().GetMoisture() < B->GetDetail().GetMoisture()) {
					return true;
				}
				return false;
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



class RiverEdge {
	std::vector<RiverEdge*> prevs;
	std::vector<RiverEdge*> nexts;
	std::vector<Cell*> links;
	bool is_start;
	bool is_end;
	Cell* start; // 나중에 유니크 번호로 변경?
	Cell* end;
	Cell* owner;
	int dist;
	int branch_dist;
	int power;

	static RiverEdgeMap RIVER_EDGES_MAP;
	static RiverOutMap RIVER_OUT_MAP;
	static RiverLinkMap LINKED_RIVER_EDGES;
	static RiverLinkMap LINKED_RIVERS;
	static RiverCntMap RIVER_CNT;
	static std::vector< RiverEdge*> RIVER_EDGES;
	static std::queue<RiverEdge*> RIVER_DELETE_QUEUE;
	unsigned int unique;
public:

	static void Clear() {
		RIVER_EDGES_MAP.clear();
		RIVER_OUT_MAP.clear();
		LINKED_RIVER_EDGES.clear();
		LINKED_RIVERS.clear();
		RIVER_CNT.clear();
		for (RiverEdge* e : RIVER_EDGES) {
			delete e;
		}
		RIVER_EDGES.clear();
		//while(!RIVER_DELETE_QUEUE.empty()) RIVER_DELETE_QUEUE.pop();
		RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();
	}

	static RiverEdgeMap& GetRiverEdges() {
		return RIVER_EDGES_MAP;
	}

	static RiverOutMap& GetRiverOutEdges() {
		return RIVER_OUT_MAP;
	}

	static RiverEdge* GetRiverEdge(RiverPos pos) {
		return RIVER_EDGES_MAP[pos];
	}

	static std::vector<RiverEdge*>& GetRiverOutEdge(unsigned int num) {
		return RIVER_OUT_MAP[num];
	}


	static std::vector<Cell*>& GetLinkedRiverEdges(unsigned int num) {
		return LINKED_RIVER_EDGES[num];
	}

	static void AddLinkRiverEdge(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		LINKED_RIVER_EDGES[lakeA->GetUnique()].push_back(lakeB);
		LINKED_RIVER_EDGES[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverEdgeLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : LINKED_RIVER_EDGES[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
		return false;
	}


	static std::vector<Cell*>& GetLinkedRiver(unsigned int num) {
		return LINKED_RIVERS[num];
	}

	static void AddLinkRiver(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		LINKED_RIVERS[lakeA->GetUnique()].push_back(lakeB);
		LINKED_RIVERS[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : LINKED_RIVERS[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
		return false;
	}

	//RiverEdge() : is_start(false), start(nullptr), end(nullptr), dist(0) {};
	RiverEdge(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance)
	{ 
		Initialize(startCell, endCell, river_owner, pre_edge, next_edge, distance);
	};

	void Initialize(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
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
	bool GetRiverEnd() { return is_end; };
	Cell* GetStart() { return start; };
	Cell* GetEnd() { return end; };
	void SetStart(Cell* c) { start = c; };
	void SetEnd(Cell* c) { end = c; };
	void ChangeDist(int num);
	void SetDistAndNextAll(int num, Cell* river_owner);
	void SetDist(int num);
	void DeleteLine(std::vector<bool>& buf);
	static RiverEdge* Create(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
	static RiverEdge* CreateStartPoint(Cell* c);
	static RiverPos GetPos(Cell* start, Cell* end);

	void ConnectPrev(RiverEdge* e);

	RiverEdge* GetOwnerEdge();

	static void AddOceanConnect(Cell* c) {
		int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
		if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
			RIVER_CNT[num] = 1;
		}
		else {
			RIVER_CNT[num]++;
		}
	};
	static int GetOceanConnect(Cell* c) {
		int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
		if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
			return 0;
		}
		else {
			return RIVER_CNT[num];
		}
	}
};


class RiverPoint {

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

using RiverPointVector = std::vector<RiverPoint>;

class RiverLine {
	RiverPointVector points;
	std::vector<Triangle> tris;
	double radius;
	double power_sacle;
public:
	RiverLine(double _radius, double _power_sacle)
		: radius(_radius)
		, power_sacle(_power_sacle)
	{};

	void AddPoint(RiverPoint p) {
		points.push_back(p);
	}
	void CrateTriangle() {

	}
	RiverPointVector& GetPointArray() {
		return points;
	}
	RiverPoint& GetPoint(unsigned int num) {
		return points[num];
	}
	
	RiverPoint& GetFirstPoint() {
		return points[0];
	}

	RiverPoint& GetEndPoint() {
		return points[points.size() - 1];
	}

	Point2 GetFirstDirection() {
		return (points[1].point - points[0].point).Normailize();
	}

	Point2 GetEndDirection() {
		return (points[points.size() - 2].point - points[points.size() - 1].point).Normailize();
	}

	void AdjustFirstPoint() {
		RiverPoint& r_p = GetFirstPoint();
		r_p.point = r_p.point + GetFirstDirection() * radius * (power_sacle * r_p.power + 1);
	}


	void AdjustLastPoint() {
		RiverPoint& r_p = GetEndPoint();
		r_p.point = r_p.point + GetEndDirection() * radius * (power_sacle * r_p.power + 1);
	}

};

class RiverCrossing;
using RiverCrossingMap = std::unordered_map<unsigned int, RiverCrossing>;

class RiverCrossing {


	static RiverCrossingMap RIVER_CROSSING_MAP;
	std::vector<RiverLine*> inputs;
	std::vector<RiverLine*> outputs;
public:
	RiverCrossing() {};
	static void Clear() {
		RIVER_CROSSING_MAP.clear();
	}

	static RiverCrossing* Get(unsigned int cell_unique) {
		if (RIVER_CROSSING_MAP.find(cell_unique) == RIVER_CROSSING_MAP.end()) {
			return nullptr;
		}
		else {
			return &RIVER_CROSSING_MAP[cell_unique];
		}
	}

	static void AddRiver(RiverLine& river) {
		auto first_unique = river.GetFirstPoint().GetCell()->GetUnique();
		auto end_unique = river.GetEndPoint().GetCell()->GetUnique();

		if (RIVER_CROSSING_MAP.find(first_unique) == RIVER_CROSSING_MAP.end()) {
			RIVER_CROSSING_MAP[first_unique] = RiverCrossing();
		}
		
		
		if (RIVER_CROSSING_MAP.find(end_unique) == RIVER_CROSSING_MAP.end()) {
			RIVER_CROSSING_MAP[end_unique] = RiverCrossing();
		}

		RiverCrossing& first_crossing = RIVER_CROSSING_MAP[first_unique];
		RiverCrossing& end_crossing = RIVER_CROSSING_MAP[end_unique];

		first_crossing.outputs.push_back(&river);
		end_crossing.inputs.push_back(&river);
		river.AdjustFirstPoint();
		river.AdjustLastPoint();

	}
};


class RiverLines {
	std::vector<RiverLine> lines;
public:
	void AddLine(RiverLine& line) {
		RiverCrossing::AddRiver(line);
		lines.push_back(line);
	}

	std::vector<RiverLine>& GetArray() {
		return lines;
	}

};