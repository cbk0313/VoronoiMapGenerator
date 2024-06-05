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

	static RiverEdgeMap river_edges;
	static RiverOutMap river_out_edges;
	static RiverLinkMap linked_river_edges;
	static RiverLinkMap linked_rivers;
	static RiverCntMap river_cnt;
	static std::vector< RiverEdge*> RIVER_EDGES;
	static std::queue<RiverEdge*> RIVER_DELETE_QUEUE;
	unsigned int unique;
public:

	static void Clear() {
		river_edges.clear();
		river_out_edges.clear();
		linked_river_edges.clear();
		linked_rivers.clear();
		river_cnt.clear();
		for (RiverEdge* e : RIVER_EDGES) {
			delete e;
		}
		RIVER_EDGES.clear();
		while(!RIVER_DELETE_QUEUE.empty()) RIVER_DELETE_QUEUE.pop();
	}

	static RiverEdgeMap& GetRiverEdges() {
		return river_edges;
	}

	static RiverOutMap& GetRiverOutEdges() {
		return river_out_edges;
	}

	static RiverEdge* GetRiverEdge(RiverPos pos) {
		return river_edges[pos];
	}

	static std::vector<RiverEdge*>& GetRiverOutEdge(unsigned int num) {
		return river_out_edges[num];
	}


	static std::vector<Cell*>& GetLinkedRiverEdges(unsigned int num) {
		return linked_river_edges[num];
	}

	static void AddLinkRiverEdge(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		linked_river_edges[lakeA->GetUnique()].push_back(lakeB);
		linked_river_edges[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverEdgeLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : linked_river_edges[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
		return false;
	}


	static std::vector<Cell*>& GetLinkedRiver(unsigned int num) {
		return linked_rivers[num];
	}

	static void AddLinkRiver(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		linked_rivers[lakeA->GetUnique()].push_back(lakeB);
		linked_rivers[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : linked_rivers[lakeA->GetUnique()]) {
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
		if (river_cnt.find(num) == river_cnt.end()) {
			river_cnt[num] = 1;
		}
		else {
			river_cnt[num] = river_cnt[num] + 1;
		}
	};
	static int GetOceanConnect(Cell* c) {
		int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
		if (river_cnt.find(num) == river_cnt.end()) {
			return 0;
		}
		else {
			return river_cnt[num];
		}
	}
};


struct RiverPoint {
	unsigned int power;
	Point2 point;
	Cell* cell;
	RiverPoint(unsigned int pow, Cell* c, Point2 p = Point2(0, 0)) : power(pow), cell(c) {
		if (c != nullptr) {
			point = c->site.p;
		}
		else {
			point = p;
		}
	};
};

class RiverDraw {

	std::vector<Triangle> tris;

public:
	RiverPoint start;
	RiverPoint end;
	RiverDraw(RiverPoint p1, RiverPoint p2) : start(p1), end(p2) {};
};