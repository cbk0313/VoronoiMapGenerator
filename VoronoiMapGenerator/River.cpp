#include "Data/River.h"


RiverEdgeMap RiverEdge::RIVER_EDGES_MAP = RiverEdgeMap();
RiverOutMap RiverEdge::RIVER_OUT_MAP = RiverOutMap();
RiverLinkMap RiverEdge::LINKED_RIVER_EDGES = RiverLinkMap();
RiverLinkMap RiverEdge::LINKED_RIVERS = RiverLinkMap();
RiverCntMap RiverEdge::RIVER_CNT = RiverCntMap();

std::vector< RiverEdge*> RiverEdge::RIVER_EDGES = std::vector< RiverEdge*>();
std::queue<RiverEdge*> RiverEdge::RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();


RiverCrossingMap RiverCrossing::RIVER_CROSSING_MAP = RiverCrossingMap();

void RiverEdge::Clear() { 
	
	RIVER_EDGES_MAP.clear();
	RIVER_OUT_MAP.clear();
	LINKED_RIVER_EDGES.clear();
	LINKED_RIVERS.clear();
	RIVER_CNT.clear();
	for (RiverEdge* e : RIVER_EDGES) {
		delete e;
	}
	RIVER_EDGES.clear();
	while(!RIVER_DELETE_QUEUE.empty()) RIVER_DELETE_QUEUE.pop();
	//RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();
	
}
void RiverEdge::Initialize(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	prevs.clear();
	nexts.clear();
	links.clear();
	is_start = false;
	start = startCell;
	end = endCell;
	owner = river_owner;
	dist = distance;
	if (pre_edge != nullptr) {
		prevs.push_back(pre_edge);
		pre_edge->nexts.push_back(this);
		//RIVER_OUT_MAP[start->GetUnique()].push_back(this);
	}

	RIVER_OUT_MAP[start->GetUnique()].push_back(this);
	if (next_edge != nullptr) nexts.push_back(next_edge);
}

Cell* RiverEdge::GetOnwer() {
	return owner;
}
void RiverEdge::SetOnwer(Cell* c) {
	owner = c;
}
std::vector<RiverEdge*>& RiverEdge::GetPrevs() {
	return prevs;
}

void RiverEdge::AddPrev(RiverEdge* e) {
	prevs.push_back(e);
}

std::vector<RiverEdge*>& RiverEdge::GetNexts()
{
	return nexts;
}

void RiverEdge::AddNext(RiverEdge* e) {
	nexts.push_back(e);
}

void RiverEdge::ChangeDist(int num) {
	dist -= num;
	for (RiverEdge* re : nexts) {
		re->ChangeDist(num);
	}
}

void RiverEdge::SetDistAndNextAll(int num, Cell* new_owner) {
	SetDist(num);
	for (RiverEdge* re : nexts) {
		re->SetOnwer(new_owner);
		re->SetDistAndNextAll(num + 1, new_owner);
	}
}

void RiverEdge::SetDist(int num) {
	dist = num;
}


void RiverEdge::DeleteLine(std::vector<bool>& buf) {
	
	if (nexts.size() == 0) {
		for (auto pre_e : prevs) {
			std::vector<RiverEdge*> tmp;
			for (auto e : pre_e->nexts) {
				if (e != this) {
					tmp.push_back(e);
				}
			}
			pre_e->nexts = tmp;
		}
		for (auto pre_e : prevs) {
			pre_e->DeleteLine(buf);
		}
		//if(pre_e != nullptr) pre_e->DeleteLine(map, buf);
		buf[end->GetUnique()] = false;
		//buf[start->GetUnique()] = false;
		if (RIVER_OUT_MAP.find(start->GetUnique()) != RIVER_OUT_MAP.end()) {
			auto& arr = RIVER_OUT_MAP[start->GetUnique()];
			if (arr.size() == 1) {
				RIVER_OUT_MAP.erase(start->GetUnique());
			}
			else {
				std::vector<RiverEdge*> tmp;
				for (auto e : arr) {
					if (e != this) {
						tmp.push_back(e);
					}
				}
				RIVER_OUT_MAP[start->GetUnique()] = tmp;

			}
		}
		for (auto pre_e : prevs) {
			if (pre_e->GetRiverEnd() && pre_e->GetNexts().size() <= 1) {
				pre_e->SetRiverEnd(false);
				//std::cout << "test\n";
			}
		}

		RIVER_EDGES_MAP.erase(RiverEdge::GetPos(start, end));
		RIVER_DELETE_QUEUE.push(this);
		//delete this;
	}

}

RiverEdge* RiverEdge::Create(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	RiverEdge* e;
	if (RIVER_DELETE_QUEUE.empty()) {
		e = new RiverEdge(startCell, endCell, river_owner, pre_edge, next_edge, distance);
		e->unique = (unsigned int)(RIVER_EDGES.size());
		RIVER_EDGES.push_back(e);
	}
	else {
		e = RIVER_DELETE_QUEUE.front();
		RIVER_DELETE_QUEUE.pop();
		e->Initialize(startCell, endCell, river_owner, pre_edge, next_edge, distance);
	}
	return e;
}

RiverEdge* RiverEdge::CreateStartPoint(Cell* c) {
	RiverEdge* e = Create(c, c, c, nullptr, nullptr, 0);
	e->is_start = true;
	return e;
}


RiverPos RiverEdge::GetPos(Cell* start, Cell* end) {
	return std::make_pair(start->GetUnique(), end->GetUnique());
}


void RiverEdge::ConnectPrev(RiverEdge* e) {
	prevs.push_back(e);
	e->GetNexts().push_back(this);
}

RiverEdge* RiverEdge::GetOwnerEdge() {
	return RIVER_EDGES_MAP[RiverEdge::GetPos(GetOnwer(), GetOnwer())];
}



const double RiverTriangle::matrix_2[3][3] = { {2.0f, -4.0f, 2.0f}, {-3.0f, 4.0f, -1.0f}, {1.0f, 0.0f, 0.0f} };
const double RiverTriangle::matrix_3[4][4] = { {-1.0f, 3.0f, -3.0f, 1.0f},
						  {2.0f, -5.0f, 4.0f, -1.0f},
						  {-1.0f, 0.0f, 1.0f, 0.0f},
						  {0.0f, 2.0f, 0.0f, 0.0f} };

std::vector<RiverLine*> RiverLine::RIVER_LINE_ARR = std::vector<RiverLine*>();
unsigned int RiverLine::ADDED_COUNT = 0;