#include "Cell.h"
#include "Edge.h"
#include <algorithm>
#include <limits>
#include "Diagram.h"
#include "Data/Triangle.h"

//unsigned int Cell::s_unique = 0;

//Cell* Cell::GetHasRiver() {
//	return has_river;
//}
//
//void Cell::SetHasRiver(Cell* b) {
//	has_river = b;
//}
Cell::Cell(Diagram* diagram)
	: closeMe(false)
	, detail(CellDetail(this))
{
	unique = diagram->CellUnique++;
}

Cell::Cell(Point2 _site, Diagram* diagram)
	: site(_site, this)
	, closeMe(false)
	, detail(CellDetail(this))
{ 
	unique = diagram->CellUnique++;
};


std::vector<Cell*> Cell::getNeighbors() {
	std::vector<Cell*> neighbors;
	Edge* e;

	size_t edgeCount = halfEdges.size();
	while (edgeCount--) {
		e = halfEdges[edgeCount]->edge;
		if (e->lSite && e->lSite != &site) {
			neighbors.push_back(e->lSite->cell);
		}
		else if (e->rSite && e->rSite != &site) {
			neighbors.push_back(e->rSite->cell);
		}
	}

	return neighbors;
}

cellBoundingBox Cell::getBoundingBox() {
	size_t edgeCount = halfEdges.size();
	double xmin = std::numeric_limits<double>::infinity();
	double ymin = xmin;
	double xmax = -xmin;
	double ymax = xmax;

	Vertex* vert;
	while (edgeCount--) {
		vert = halfEdges[edgeCount]->startPoint();

		double vx = vert->point.x;
		double vy = vert->point.y;

		if (vx < xmin) xmin = vx;
		if (vy < ymin) ymin = vy;
		if (vx > xmax) xmax = vx;
		if (vy > ymax) ymax = vy;
	}

	return cellBoundingBox(xmin, ymin, xmax, ymax);
}

bool Cell::IsInside(Point2 pos) {
	for (HalfEdge* hf : halfEdges) {
		Edge* e = hf->edge;
		Triangle triA = Triangle{site.p, e->vertA->point, e->p };
		Triangle triB = Triangle{site.p, e->vertB->point, e->p };
		if (triA.IsInside(pos) || triB.IsInside(pos)) return true;
	}
	return false;
}

// Return whether a point is inside, on, or outside the cell:
//   -1: point is outside the perimeter of the cell
//    0: point is on the perimeter of the cell
//    1: point is inside the perimeter of the cell
//
int Cell::pointIntersection(double x, double y) {
	// Check if point in polygon. Since all polygons of a Voronoi
	// diagram are convex, then:
	// http://paulbourke.net/geometry/polygonmesh/
	// Solution 3 (2D):
	//   "If the polygon is convex then one can consider the polygon
	//   "as a 'path' from the first vertex. A point is on the interior
	//   "of this polygons if it is always on the same side of all the
	//   "line segments making up the path. ...
	//   "(y - y0) (x1 - x0) - (x - x0) (y1 - y0)
	//   "if it is less than 0 then P is to the right of the line segment,
	//   "if greater than 0 it is to the left, if equal to 0 then it lies
	//   "on the line segment"
	HalfEdge* he;
	size_t edgeCount = halfEdges.size();
	Vertex* p0;
	Vertex* p1;
	double r;

	while (edgeCount--) {
		he = halfEdges[edgeCount];
		p0 = he->startPoint();
		p1 = he->endPoint();
		r = (y - p0->point.y)*(p1->point.x - p0->point.x) - (x - p0->point.x)*(p1->point.y - p0->point.y);

		if (r == 0) {
			return 0;
		}
		if (r > 0) {
			return -1;
		}
	}
	return 1;
}

bool Cell::edgesCCW(HalfEdge* a, HalfEdge* b) {
	return a->angle > b->angle;
}
