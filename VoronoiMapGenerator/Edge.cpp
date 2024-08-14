#include "Edge.h"
#include "Cell.h"
#include <cmath>
#include "Cell.h"

bool Vertex::Find(Cell* c) {
	for (Cell* cell : cells) {
		if (cell == c) {
			return true;
		}
	}
	return false;
}


void Vertex::Insert(Cell* c) {
	cells.push_back(c);
}


HalfEdge::HalfEdge(Edge* e, Site* lSite, Site* rSite) {
	site = lSite;
	edge = e;
	// 'angle' is a value to be used for properly sorting the
	// halfsegments counterclockwise. By convention, we will
	// use the angle of the line defined by the 'site to the left'
	// to the 'site to the right'.
	// However, border edges have no 'site to the right': thus we
	// use the angle of line perpendicular to the halfsegment (the
	// edge should have both end points defined in such case.)
	if (rSite) {
		angle = atan2(rSite->p.y - lSite->p.y, rSite->p.x - lSite->p.x);
	}
	else {
		Vertex* va = (e->vertA);
		Vertex* vb = (e->vertB);

		angle = (e->lSite == lSite) ? atan2(vb->point.x - va->point.x, va->point.y - vb->point.y) : atan2(va->point.x - vb->point.x, vb->point.y - va->point.y);
	}
}

void Edge::setStartPoint(Site* _lSite, Site* _rSite, Vertex* vertex) {
	if (!vertA && !vertB) {
		vertA = vertex;
		lSite = _lSite;
		rSite = _rSite;
	}
	else if (lSite == _rSite) {
		vertB = vertex;
	}
	else {
		vertA = vertex;
	}
}

void Edge::setEndPoint(Site* _lSite, Site* _rSite, Vertex* vertex) {
	setStartPoint(_rSite, _lSite, vertex);
}

Cell* Edge::GetOppositeCell(Cell* c) {
	return (lSite->cell == c && rSite) ? rSite->cell : lSite->cell;
}