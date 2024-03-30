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

	//siteEventQueue = new std::vector<Point2*>();
	std::vector<Point2*> siteEventQueue = std::vector<Point2*>();
	siteEventQueue.reserve(sites.size());
	boundingBox = bbox;

	for (size_t i = 0; i < sites.size(); ++i) {
		//sanitize sites by quantizing to integer multiple of epsilon
		sites[i].x = round(sites[i].x / EPSILON)*EPSILON;
		sites[i].y = round(sites[i].y / EPSILON)*EPSILON;

		siteEventQueue.push_back(&(sites[i]));
	}

	Diagram* diagram = new Diagram();
	circleEventQueue = new CircleEventQueue();
	beachLine = new RBTree<BeachSection>();
	
	std::sort(siteEventQueue.begin(), siteEventQueue.end(), pointComparator);
	// Initialize site event queue
	

	// process queue
	Point2* site = siteEventQueue.empty() ? nullptr : siteEventQueue.back();
	if (!siteEventQueue.empty()) siteEventQueue.pop_back();
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
			site = siteEventQueue.empty() ? nullptr : siteEventQueue.back();
			if (!siteEventQueue.empty()) siteEventQueue.pop_back();
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
	
	//diagram->finalize();

	delete circleEventQueue;
	circleEventQueue = nullptr;

	//delete siteEventQueue;
	//siteEventQueue = nullptr;

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


Diagram* VoronoiDiagramGenerator::relaxLoop(int num, Diagram* diagram) {
	for (int i = 0; i < num; i++) {
		diagram = relax(diagram);
	}

	return diagram;
}



void VoronoiDiagramGenerator::createWorld(int seed, double radius, Diagram* diagram) {
	if (diagram) {
		srand(seed);
		initWorld(seed, radius, 10 + (rand() % 21 - 5), radius / 3, radius / 5, diagram);
		createRiver(diagram);
	}
}

std::pair<double, double> VoronoiDiagramGenerator::getMinDist(std::vector<std::pair<Point2, double>>& points, Point2& c_p, double radius) {
	double min_dist = radius;
	double min_r = radius;
	for (std::pair<Point2, double>& p : points) {
		double dist = calcDistance(p.first, c_p);
		if (min_dist - min_r > dist - p.second) {
			min_dist = dist;
			min_r = p.second;
		}
	}

	return std::make_pair(min_dist, min_r);
}


void VoronoiDiagramGenerator::initWorld(int seed, double radius, unsigned int p_cnt, double p_max_r, double p_min_r, Diagram* diagram) {

	
	Point2 center = Point2((boundingBox.xR - boundingBox.xL) / 2, (boundingBox.yB - boundingBox.yT) / 2); // Center of the circular island.

	FastNoiseLite noise;
	noise.SetSeed(seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);

	FastNoiseLite p_noise;
	p_noise.SetSeed(seed);
	p_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	unsigned int max_elevation = 0;
	double p_r_step = p_max_r - p_min_r;
	


	double continentSize = radius * 0.6;
	double islandRange = radius * 0.2;	
	std::vector<std::pair<Point2, double>> points;
	for (int i = 0; i < p_cnt; i++) {
		Point2 p = Point2((getRandom() - 0.5) * islandRange, (getRandom() - 0.5) * islandRange);
		double length = calcDistance(Point2(0, 0), p);
		Point2 dir = Point2((p.x / length) * continentSize + center.x, (p.y / length) * continentSize + center.y);
		//std::cout << p_x << "\n";
		//std::cout << p_y << "\n\n";
		points.push_back(std::make_pair(Point2(p.x + dir.x, p.y + dir.y), (p_max_r - (getRandom() * p_r_step))));
	}


	for (Cell* c : diagram->cells) {

		double scale = 2000;
		double p_scale = 500;
		double dist = calcDistance(c->site.p, center);
		std::pair<double, double> p_dist = getMinDist(points, c->site.p, radius);
		//std::cout << p_dist << endl;

		double dist_scale = 1 - pow(dist / radius, 1);
		double p_dist_scale = 1 - pow(p_dist.first / p_dist.second, 2);

		double value = (1 + (noise.GetNoise(round(c->site.p.x / scale), round(c->site.p.y / scale)))); // Add noise to shape the circular island.
		double p_value = (1 + (p_noise.GetNoise(round(c->site.p.x / p_scale), round(c->site.p.y / p_scale)))); // Add noise to shape the circular island.

		//if (value * dist_scale >= 0.4) { // Set Land
		//if (p_value * p_dist_scale >= 0.4) { // Set Land
		//if (p_value > 1) { // Set Land
		if (std::max(value * dist_scale, p_value *p_dist_scale) >= 0.4) { // Set Land
			c->detail.color = Color(0.6, 0.4, 0, 1);
			c->detail.terrain = Terrain::LAND; // detail.terrain default value is Terrain::OCEAN
		}
		else {
			c->detail.b_peak = false;
			for (HalfEdge* he : c->halfEdges) {
				if (!he->edge->rSite) { // Check whether it is the outermost border.
					c->detail.color = Color(0.1, 0, 0.3, 1);
					c->detail.b_edge = true;
					break;
				}
			}
		}
	}

	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;

			if (l_cell->detail.terrain == Terrain::OCEAN && r_cell->detail.terrain == Terrain::OCEAN) { // Set 'union' between connected seas
				if (l_cell->detail.unionfind.unionFindCell(Terrain::OCEAN)->detail.b_edge) {
					r_cell->detail.unionfind.setUnionCell(Terrain::OCEAN, l_cell);
				}
				else {
					l_cell->detail.unionfind.setUnionCell(Terrain::OCEAN, r_cell);
				}
			}
		}
	}

	for (Cell* c : diagram->cells) { // Set Lake
		CellDetail& cd = c->detail;
		Terrain ct = cd.terrain;
		if (ct == Terrain::OCEAN) {

			auto unique = cd.unionfind.unionFindCell(Terrain::OCEAN)->getUnique();
			if (!cd.unionfind.unionFindCell(Terrain::OCEAN)->detail.b_edge) {
				cd.terrain = Terrain::LAKE;
				cd.color = Color(0.2, 0.4, 0.6);
				//diagram->lakeUnion.insertCell(unique, c);

			}
			else {
				auto unique = cd.unionfind.unionFindCell(Terrain::OCEAN)->getUnique();
				diagram->oceanUnion.insert(unique)->push_back(c);
			}

		}
		else if (ct == Terrain::COAST) {
			auto unique = cd.unionfind.unionFindCell(Terrain::OCEAN)->getUnique();
			diagram->oceanUnion.insert(unique)->push_back(c);
		}
	}

	std::queue<Cell*> landQueue;
	std::queue<Cell*> lakeQueue;

	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;
			//auto& l_detail = l_cell->detail;
			//auto& r_detail = r_cell->detail;

			Cell* landCell, *oceanCell;
			(l_cell->detail.terrain == Terrain::OCEAN || l_cell->detail.terrain == Terrain::LAKE) ? (landCell = r_cell, oceanCell = l_cell) : (landCell = l_cell, oceanCell = r_cell);

			if (landCell->detail.terrain == Terrain::LAND && oceanCell->detail.terrain == Terrain::OCEAN) {
				CellDetail& cd = oceanCell->detail;
				if (cd.unionfind.unionFindCell(Terrain::OCEAN)->detail.b_edge) { // check if it's lake or not
					cd.color *= 2;
					cd.terrain = Terrain::COAST;
					cd.elevation = 1;
					landQueue.push(oceanCell);
				}
				
			}

			else if (landCell->detail.terrain == Terrain::LAND && oceanCell->detail.terrain == Terrain::LAKE) {
				//landCell->detail.color = Color(1, 0, 0);
				lakeQueue.push(oceanCell);
			}
		}
	}

	while (!landQueue.empty() || !lakeQueue.empty()) {
		//Cell* c = landQueue.front();

		bool is_lq = false;
		Cell* c;
		if (landQueue.empty()) {
			c = lakeQueue.front();
			lakeQueue.pop();
			is_lq = true;
		}
		else {
			c = landQueue.front();
			landQueue.pop();
		}
		

		CellDetail& cd = c->detail;
		unsigned int land_cnt = 0, low_cnt = 0;

		for (HalfEdge* he : c->halfEdges) {
			Edge* e = he->edge;
			Cell* targetCell = (e->lSite->cell == c && e->rSite) ? e->rSite->cell : e->lSite->cell;
			//if (targetCell->detail.elevation < c->detail.elevation) {
			CellDetail& tcd = targetCell->detail;
			if (cd.terrain == Terrain::LAKE && tcd.terrain == Terrain::LAND) {
				if (tcd.elevation == 0) {
					tcd.elevation = cd.unionfind.unionFindCell(Terrain::OCEAN)->detail.elevation;
					lakeQueue.push(targetCell);
					continue;
				}
				
			}
			else if (tcd.terrain == Terrain::LAND) {
				land_cnt++;
				if (tcd.elevation == 0) {
					//tcd.elevation = cd.elevation + 1;
					tcd.elevation = cd.elevation + 1;
					max_elevation = std::max(max_elevation, tcd.elevation);
					if (is_lq) {
						lakeQueue.push(targetCell);
					}
					else {
						landQueue.push(targetCell);
					}
					
					cd.b_peak = false;
					cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak = false;
				}
				else if (cd.elevation > 2){
					if (tcd.elevation <= cd.elevation) {
						low_cnt++;
					}
					else {
						cd.b_peak = false;
						cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak = false;
					}
				}

				if (cd.terrain == Terrain::LAND && cd.elevation > 2 && cd.elevation == tcd.elevation) {
					tcd.b_peak = cd.b_peak && tcd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak && cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak; // Peak only if both are peaks
					cd.b_peak = tcd.b_peak; // Peak only if both are peaks
					cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak = tcd.b_peak; // Peak only if both are peaks
					tcd.unionfind.setUnionCell(Terrain::PEAK, c);
				}
			}
			else if (tcd.terrain == Terrain::LAKE) {
				//low_cnt++;
				if (!is_lq) {
					auto t_union = tcd.unionfind.unionFindCell(Terrain::OCEAN);
					if (t_union->detail.elevation == 0 || t_union->detail.elevation > cd.elevation) {
						t_union->detail.elevation = cd.elevation;
					}
				}
			}
		}

		if (land_cnt != 0 && land_cnt == low_cnt) { // ÆòÁö
			//c->detail.color += Color(0,0.1,0);
			cd.b_flat = true;
		}
		else {
			cd.b_peak = false;
			cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak = false;
		}
	}

	

	for (Edge* e : diagram->edges) {
		if (e->lSite && e->rSite) {
			Cell* l_cell = e->lSite->cell;
			Cell* r_cell = e->rSite->cell;
			Terrain l_t = l_cell->detail.terrain;
			Terrain r_t = r_cell->detail.terrain;
			if ((l_t == Terrain::LAND || l_t == Terrain::LAKE) && (r_t == Terrain::LAND || r_t == Terrain::LAKE)) { // land ¶¥ °øÀ¯
				l_cell->detail.unionfind.setUnionCell(Terrain::LAND, r_cell);
			}
		}
	}




	for (Cell* c : diagram->cells) {
		CellDetail& cd = c->detail;
		if (cd.elevation != 0) {
			double elev_scale = (double)cd.elevation / max_elevation;
			elev_scale = pow(elev_scale, 1);
			double scale = 1.2;
			cd.color.r += elev_scale * scale;
			cd.color.g += elev_scale * scale;
			cd.color.b += elev_scale * 0.7 * scale;
		}

		Terrain ct = cd.terrain;
		if (ct == Terrain::LAND) {
			auto arr = diagram->islandUnion.insert(cd.unionfind.unionFindCell(Terrain::LAND)->getUnique());
			arr->land.push_back(c);

			if (cd.unionfind.unionFindCell(Terrain::PEAK)->detail.b_peak) {
				auto unique = cd.unionfind.unionFindCell(Terrain::PEAK)->getUnique();
				cd.terrain = Terrain::PEAK;
				arr->peakUnion.insert(unique)->push_back(c);
				/*if (c->getUnique() > unique) {
					std::cout << "cell: " << c->getUnique() << "\n";
					std::cout << "uni: " << unique << "\n";
				}*/
				cd.color = Color(0.3, 0.3, 0.3);
				cd.unionfind.unionFindCell(Terrain::PEAK)->detail.color = Color(0, 0, 0);
			}
		}
		else if (ct == Terrain::LAKE) {
			auto arr = diagram->islandUnion.insert(cd.unionfind.unionFindCell(Terrain::LAND)->getUnique());
			arr->land.push_back(c);
			auto unique = cd.unionfind.unionFindCell(Terrain::OCEAN)->getUnique();
			arr->lakeUnion.insert(unique)->push_back(c);

		}
		/*if (cd.b_flat) {
			c->detail.color = Color(0, 1, 0);
		}*/
	}
	
	
}


void VoronoiDiagramGenerator::createRiver(Diagram* diagram) {
	for (auto item : diagram->islandUnion.unions) {
		auto island = item.second;
		island.peakUnion.unions;
	}
}