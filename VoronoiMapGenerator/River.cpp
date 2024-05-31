#include "Data/River.h"


RiverEdgeMap RiverEdge::river_edges = RiverEdgeMap();
RiverOutMap RiverEdge::river_out_edges = RiverOutMap();
RiverLinkMap RiverEdge::linked_river_edges = RiverLinkMap();
RiverLinkMap RiverEdge::linked_rivers = RiverLinkMap();
RiverCntMap RiverEdge::river_cnt = RiverCntMap();

Cell* RiverEdge::GetOnwer() {
	return owner;
}
void RiverEdge::SetOnwer(Cell* c) {
	owner = c;
}
std::vector<RiverEdge*>& RiverEdge::GetPrevs() {
	return prevs;
}
std::vector<RiverEdge*>& RiverEdge::GetNexts()
{
	return nexts;
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
		if (river_out_edges.find(start->GetUnique()) != river_out_edges.end()) {
			auto& arr = river_out_edges[start->GetUnique()];
			if (arr.size() == 1) {
				river_out_edges.erase(start->GetUnique());
			}
			else {
				std::vector<RiverEdge*> tmp;
				for (auto e : arr) {
					if (e != this) {
						tmp.push_back(e);
					}
				}
				river_out_edges[start->GetUnique()] = tmp;

			}
		}
		

		river_edges.erase(RiverEdge::GetPos(start, end));
		delete this;
	}

}

RiverEdge* RiverEdge::Create(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	return new RiverEdge(startCell, endCell, river_owner, pre_edge, next_edge, distance);
}

RiverEdge* RiverEdge::CreateStartPoint(Cell* c) {
	RiverEdge* e = new RiverEdge(c, c, c, nullptr, nullptr, 0);
	e->is_start = true;
	return e;
}


RiverPos RiverEdge::GetPos(Cell* start, Cell* end) {
	return std::make_pair(start->GetUnique(), end->GetUnique());
}


void RiverEdge::AddNext(RiverEdge* e) {
	nexts.push_back(e);
}

void RiverEdge::ConnectPrev(RiverEdge* e) {
	prevs.push_back(e);
	e->GetNexts().push_back(this);
}

RiverEdge* RiverEdge::GetOwnerEdge() {
	return river_edges[RiverEdge::GetPos(GetOnwer(), GetOnwer())];
}

