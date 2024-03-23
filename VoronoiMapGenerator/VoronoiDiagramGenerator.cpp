#include "VoronoiDiagramGenerator.h"
#include "Vector2.h"

#include "FastNoise/FastNoiseLite.h"

#include "Epsilon.h"
#include <algorithm>
#include <iostream>

#include <queue>

using std::cout;
using std::cin;
using std::endl;

void VoronoiDiagramGenerator::printBeachLine() {
	treeNode<BeachSection>* section = beachLine->getFirst(beachLine->getRoot());

	while (section) {
		cout << section->data.site->p << endl;
		section = section->next;
	}
	if(section) cout << section->data.site->p << endl;
	cout << endl << endl;
}

bool pointComparator(Point2* a, Point2* b) {
	double r = b->y - a->y;
	if (r < 0) return true;
	else if (r == 0) {
		if (b->x - a->x < 0) return true;
		else return false;
	}
	else return false;
}

Diagram* VoronoiDiagramGenerator::compute(std::vector<Point2>& sites, BoundingBox bbox) {

	siteEventQueue = new std::vector<Point2*>();
	boundingBox = bbox;

	for (size_t i = 0; i < sites.size(); ++i) {
		//sanitize sites by quantizing to integer multiple of epsilon
		sites[i].x = round(sites[i].x / EPSILON)*EPSILON;
		sites[i].y = round(sites[i].y / EPSILON)*EPSILON;

		siteEventQueue->push_back(&(sites[i]));
	}

	Diagram* diagram = new Diagram();
	circleEventQueue = new CircleEventQueue();
	beachLine = new RBTree<BeachSection>();

	// Initialize site event queue
	std::sort(siteEventQueue->begin(), siteEventQueue->end(), pointComparator);

	// process queue
	Point2* site = siteEventQueue->empty() ? nullptr : siteEventQueue->back();
	if (!siteEventQueue->empty()) siteEventQueue->pop_back();
	treeNode<CircleEvent>* circle;

	// main loop
	for (;;) {
		// figure out whether to handle a site or circle event
		// for this we find out if there is a site event and if it is
		// 'earlier' than the circle event
		circle = circleEventQueue->firstEvent;

		// add beach section
		if (site && (!circle || site->y < circle->data.y || (site->y == circle->data.y && site->x < circle->data.x))) {
			// first create cell for new site
			Cell* cell = diagram->createCell(*site);
			// then create a beachsection for that site
			addBeachSection(&cell->site, diagram);

			site = siteEventQueue->empty() ? nullptr : siteEventQueue->back();
			if (!siteEventQueue->empty()) siteEventQueue->pop_back();
		}

		// remove beach section
		else if (circle)
			removeBeachSection(circle->data.beachSection, diagram);

		// all done, quit
		else
			break;
	}

	// wrapping-up:
	//   connect dangling edges to bounding box
	//   cut edges as per bounding box
	//   discard edges completely outside bounding box
	//   discard edges which are point-like
	diagram->clipEdges(boundingBox);

	//   add missing edges in order to close open cells
	diagram->closeCells(boundingBox);

	diagram->finalize();

	delete circleEventQueue;
	circleEventQueue = nullptr;

	delete siteEventQueue;
	siteEventQueue = nullptr;

	delete beachLine;
	beachLine = nullptr;

	return diagram;
}

bool halfEdgesCW(HalfEdge* e1, HalfEdge* e2) {
	return e1->angle < e2->angle;
}

Diagram* VoronoiDiagramGenerator::relax(Diagram* diagram) {
	std::vector<Point2> sites;
	std::vector<Point2> verts;
	std::vector<Vector2> vectors;
	//replace each site with its cell's centroid:
	//    subdivide the cell into adjacent triangles
	//    find those triangles' centroids (by averaging corners) 
	//    and areas (by computing vector cross product magnitude)
	//    combine the triangles' centroids through weighted average
	//	  to get the whole cell's centroid
	for (Cell* c : diagram->cells) {
		size_t edgeCount = c->halfEdges.size();
		verts.resize(edgeCount);
		vectors.resize(edgeCount);

		for (size_t i = 0; i < edgeCount; ++i) {
			verts[i] = *c->halfEdges[i]->startPoint();
			vectors[i] = *c->halfEdges[i]->startPoint() - verts[0];
		}

		Point2 centroid(0.0, 0.0);
		double totalArea = 0.0;
		for (size_t i = 1; i < edgeCount-1; ++i) {
			double area = (vectors[i+1].x*vectors[i].y - vectors[i+1].y*vectors[i].x)/2;
			totalArea += area;
			centroid.x += area*(verts[0].x + verts[i].x + verts[i + 1].x) / 3;
			centroid.y += area*(verts[0].y + verts[i].y + verts[i + 1].y) / 3;
		}
		centroid.x /= totalArea;
		centroid.y /= totalArea;
		sites.push_back(centroid);
	}

	//then recompute the diagram using the cells' centroids
	Diagram* n_digram = compute(sites, boundingBox);
	delete diagram;

	return n_digram;
}



void VoronoiDiagramGenerator::SetLand(int seed, double radius, Diagram* diagram) {

	if (diagram) {

		Point2 center = Point2((boundingBox.xR - boundingBox.xL) / 2, (boundingBox.yB - boundingBox.yT) / 2);

		FastNoiseLite noise;
		noise.SetSeed(seed);
		noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);


		for (Cell* c : diagram->cells) {

			double scale = 3000;
			double dist = radius / (calcDistance(c->site.p, center) + 1);
			double value = (1 + (noise.GetNoise(round(c->site.p.x / scale), round(c->site.p.y / scale)))) / 2;

			if (value * pow(dist, 2) >= 1) {
				c->detail.color = Color(0.6, 0.4, 0, 1);
				c->detail.terrain = Terrain::LAND;
			}
			else {
				c->detail.is_peak = false;
				for (HalfEdge* he : c->halfEdges) {
					if (!he->edge->rSite) {
						c->detail.color = Color(0.1, 0, 0.3, 1);
						c->detail.is_edge = true;
						break;
					}
				}
			}
		}

		for (Edge* e : diagram->edges) {
			if (e->lSite && e->rSite) {
				Cell* l_cell = e->lSite->cell;
				Cell* r_cell = e->rSite->cell;
				if (l_cell->detail.terrain == Terrain::OCEAN && r_cell->detail.terrain == Terrain::OCEAN) {
					l_cell->detail.setUnionCell(r_cell);
				}
			}
		}

		std::queue<Cell*> landQueue;

		for (Edge* e : diagram->edges) {
			if (e->lSite && e->rSite) {
				Cell* l_cell = e->lSite->cell;
				Cell* r_cell = e->rSite->cell;
				//auto& l_detail = l_cell->detail;
				//auto& r_detail = r_cell->detail;

				Cell* landCell, *oceanCell;
				if (l_cell->detail.terrain == Terrain::OCEAN) {
					landCell = r_cell;
					oceanCell = l_cell;
				}
				else {
					landCell = l_cell;
					oceanCell = r_cell;
				}

				if (landCell->detail.terrain == Terrain::LAND && oceanCell->detail.terrain == Terrain::OCEAN) {
					if (oceanCell->detail.unionFindCell()->detail.is_edge) {
						oceanCell->detail.color *= 2;
						oceanCell->detail.terrain = Terrain::COAST;
						oceanCell->detail.elevation = 1;
						landQueue.push(oceanCell);
					}
				}
			}
		}

		while (!landQueue.empty() ) {
			Cell* c = landQueue.front();
			landQueue.pop();

			unsigned int land_cnt = 0, low_cnt = 0;
			for (HalfEdge* he : c->halfEdges) {
				Edge* e = he->edge;
				Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
				//if (targetCell->detail.elevation < c->detail.elevation) {
				if (targetCell->detail.terrain == Terrain::LAND) {
					land_cnt++;
					if (targetCell->detail.elevation == 0) {
						targetCell->detail.elevation = c->detail.elevation + 1;
						landQueue.push(targetCell);
						c->detail.is_peak = false;
						c->detail.unionFindCell()->detail.is_peak = false;
					}
					else if (c->detail.elevation > 2){
						if (targetCell->detail.elevation <= c->detail.elevation) {
							low_cnt++;
						}
						else {
							c->detail.is_peak = false;
							c->detail.unionFindCell()->detail.is_peak = false;
						}
						
					}


					if (c->detail.terrain == Terrain::LAND && c->detail.elevation > 2 && c->detail.elevation == targetCell->detail.elevation) {
						targetCell->detail.setUnionCell(c);
					}
				}


			}
			if (land_cnt != 0 && land_cnt == low_cnt) { // ÆòÁö
				//c->detail.color += Color(0,0.1,0);
				c->detail.is_flat = true;
			}
			else {
				c->detail.unionFindCell()->detail.is_peak = false;
			}
		}
		for (Cell* c : diagram->cells) {
			if (c->detail.elevation != 0) {
				c->detail.color.r += ((float)pow(c->detail.elevation, 1) - 1) / (80 * 0.2);
				c->detail.color.g += ((float)pow(c->detail.elevation, 1) - 1) / (80 * 0.2);
				c->detail.color.b += ((float)pow(c->detail.elevation, 1) - 1) / (50 * 0.2);
			}
			if (c->detail.terrain == Terrain::OCEAN && !c->detail.unionFindCell()->detail.is_edge) {
				c->detail.terrain = Terrain::LAKE;
				c->detail.color = Color(0.2, 0.4, 0.6);
			}
			if (c->detail.unionFindCell()->detail.is_peak) {
				//c->detail.color = Color(0.1, 0.1, 0.1);
				//c->detail.unionFindCell()->detail.color = Color(0.5, 0.5, 0.5);
			}
		}
	}
	
}


Diagram* VoronoiDiagramGenerator::relaxLoop(int num, Diagram* diagram) {
	for (int i = 0; i < num; i++) {
		diagram = relax(diagram);
	}

	return diagram;
}

