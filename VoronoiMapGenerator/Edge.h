#ifndef _EDGE_H_
#define _EDGE_H_

#include "Point2.h"

#include "Data/Color.h"

struct Site;
struct Cell;

struct Vertex {
	Point2 point;
	VertexColor color;
	int elev = 0;
	std::vector<Cell*> cells;
	bool is_short = false;

	bool Find(Cell* c);
	void Insert(Cell* c);
	void Reset() {
		cells.clear();
		is_short = false;
		elev = 0;
		color = VertexColor();
	}
};

struct Edge {
	VertexColor color;
	Point2 p;
	Site* lSite;
	Site* rSite;

	Vertex* vertA;
	Vertex* vertB;

	Edge() : lSite(nullptr), rSite(nullptr), vertA(nullptr), vertB(nullptr) {};
	Edge(Site* _lSite, Site* _rSite) : lSite(_lSite), rSite(_rSite), vertA(nullptr), vertB(nullptr) {};
	Edge(Site* lS, Site* rS, Vertex* vA, Vertex* vB) : lSite(lS), rSite(rS), vertA(vA), vertB(vB) {};

	void setStartPoint(Site* _lSite, Site* _rSite, Vertex* vertex);
	void setEndPoint(Site* _lSite, Site* _rSite, Vertex* vertex);
};

struct HalfEdge {
	Site* site;
	Edge* edge;
	double angle;

	HalfEdge() : site(nullptr), edge(nullptr), angle(0) {};
	HalfEdge(Edge* e, Site* lSite, Site* rSite);

	inline Vertex* startPoint();
	inline Vertex* endPoint();
};

inline Vertex* HalfEdge::startPoint() {
	return (edge->lSite == site) ? edge->vertA : edge->vertB;
}

inline Vertex* HalfEdge::endPoint() {
	return (edge->lSite == site) ? edge->vertB : edge->vertA;
}

#endif