#pragma once
#include <vector>
#include "Data/Buffer.h"
#include "../Cell.h"


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
			return hash1 ^ hash2; // �ؽ� ���� XOR�� ����Ͽ� ����
		}
	};

	// ����� ���� �� �Լ� (�ʿ��� ���)
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
	Cell* start; // ���߿� ����ũ ��ȣ�� ����?
	Cell* end;
	Cell* owner;
	int dist;

	static RiverEdgeMap river_edges;
	static RiverOutMap river_out_edges;
	static RiverLinkMap linked_river_edges;
	static RiverLinkMap linked_rivers;
	static RiverCntMap river_cnt;
public:

	static void Clear() {
		river_edges.clear();
		river_out_edges.clear();
		linked_river_edges.clear();
		linked_rivers.clear();
		river_cnt.clear();
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
		: is_start(false)
		, start(startCell)
		, end(endCell)
		, owner(river_owner)
		, dist(distance)
	{ 
		if (pre_edge != nullptr) {
			prevs.push_back(pre_edge);
			pre_edge->nexts.push_back(this);
			river_out_edges[start->GetUnique()].push_back(this);
		}
		if(next_edge != nullptr) nexts.push_back(next_edge);
	};
	int GetDistance() { return dist; };
	Cell* GetOnwer();
	void SetOnwer(Cell* c);
	std::vector<RiverEdge*>& GetPrevs();
	std::vector<RiverEdge*>& GetNexts();
	bool IsStart() { return is_start; };
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

	void AddNext(RiverEdge* e);
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