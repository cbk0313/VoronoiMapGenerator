#include "VoronoiDiagramGenerator.h"
#include <iostream>
#include <Windows.h>
#include "Data/Heightmap.h"


Diagram* VoronoiDiagramGenerator::GetDiagram() {
	return diagram;
}

//void VoronoiDiagramGenerator::printBeachLine() {
//	treeNode<BeachSection>* section = beachLine->getFirst(beachLine->getRoot());
//
//	while (section) {
//		cout << section->data.site->p << endl;
//		section = section->next;
//	}
//	if (section) cout << section->data.site->p << endl;
//	cout << endl << endl;
//}

bool pointComparator(Point2* a, Point2* b) {
	double r = b->y - a->y;
	if (r < 0) return true;
	else if (r == 0) {
		if (b->x - a->x < 0) return true;
		else return false;
	}
	else return false;
}


void VoronoiDiagramGenerator::CreateSite(unsigned int dimension, unsigned int numSites) {

	image_dim = dimension;
	sites.clear();

	//bbox = BoundingBox(0, dimension, dimension, 0);
	boundingBox = BoundingBox(0, dimension, dimension, 0);
	//std::vector<Point2> tmpSites;
	numSites = (unsigned int)sqrt(numSites);
	unsigned int pow_site = (unsigned int)pow(numSites, 2);
	//tmpSites.reserve(pow_site);
	sites.reserve(pow_site);

	Point2 s;

	double step = dimension / numSites;
	int half_step = (int)(step * setting.GetSiteRange());
	setting.Srand();
	for (unsigned int i = 0; i < numSites; ++i) {
		for (unsigned int j = 0; j < numSites; ++j) {

			//s.x = (i * step) + (rand() / ((double)RAND_MAX)) * (half_step);
			s.x = (i * step) + (rand() % (int)half_step);
			s.y = (j * step) + (rand() % (int)half_step);
			//s.Epsilon();
			//std::cout << "rand: " << (rand() / ((double)RAND_MAX)) * (half_step) << "\n";
			//std::cout << "x: " << s.x << "\n";
			//std::cout << "y: " << s.y << "\n";
			sites.push_back(s);
		}
	}

	//remove any duplicates that exist
	// 
	if (setting.GetSiteRange() >= 1) {
		std::sort(sites.begin(), sites.end(), Point2::SitesOrdered);
	}
	

	/*sites.push_back(tmpSites[0]);
	for (Point2& s : tmpSites) {
		if (s != sites.back()) sites.push_back(s);
	}*/
	
}


void VoronoiDiagramGenerator::Compute() {
	if (diagram != nullptr) {
		delete diagram;
		diagram = nullptr;
	}

	//siteEventQueue = new std::vector<Point2*>();
	std::vector<Point2*> siteEventQueue = std::vector<Point2*>();
	siteEventQueue.reserve(sites.size());
	
	//boundingBox = bbox;

	for (size_t i = 0; i < sites.size(); ++i) {
		//sanitize sites by quantizing to integer multiple of epsilon
		//sites[i].x = round(sites[i].x / EPSILON) * EPSILON;
		//sites[i].y = round(sites[i].y / EPSILON) * EPSILON;

		siteEventQueue.push_back(&(sites[i]));
		siteEventQueue.back()->Epsilon();
	}

	diagram = new Diagram();
	diagram->Initialize(setting);
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
			addBeachSection(&cell->site);
			site = siteEventQueue.empty() ? nullptr : siteEventQueue.back();
			if (!siteEventQueue.empty()) siteEventQueue.pop_back();
		}

		// remove beach section
		else if (circle)
			removeBeachSection(circle->data.beachSection);

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

}

bool halfEdgesCW(HalfEdge* e1, HalfEdge* e2) {
	return e1->angle < e2->angle;
}

void  VoronoiDiagramGenerator::Relax() {
	//std::vector<Point2> sites;
	sites.clear();
	sites.reserve(diagram->cells.size());
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
			verts[i] = c->halfEdges[i]->startPoint()->point;
			vectors[i] = c->halfEdges[i]->startPoint()->point - verts[0];
		}

		Point2 centroid(0.0, 0.0);
		double totalArea = 0.0;
		for (size_t i = 1; i < edgeCount - 1; ++i) {
			double area = (vectors[i + 1].x * vectors[i].y - vectors[i + 1].y * vectors[i].x) / 2;
			totalArea += area;
			centroid.x += area * (verts[0].x + verts[i].x + verts[i + 1].x) / 3;
			centroid.y += area * (verts[0].y + verts[i].y + verts[i + 1].y) / 3;
		}
		centroid.x /= totalArea;
		centroid.y /= totalArea;
		//centroid.Epsilon();
		sites.push_back(centroid);
	}

	//then recompute the diagram using the cells' centroids

	delete diagram;
	diagram = nullptr;
	Compute();
}


void  VoronoiDiagramGenerator::RepeatRelax(int num) {
	for (int i = 0; i < num; i++) {
		Relax();
	}
}

void VoronoiDiagramGenerator::SaveAllImage(unsigned int w, unsigned int h) {
	SaveImage(ALL_IMAGE, "voronoi_map_all.bmp", w, h, false);
	SaveImage(ISLAND, "voronoi_map_islnad.bmp", w, h, false);
	SaveImage(LAKE, "voronoi_map_lake.bmp", w, h, false);
	SaveImage(RIVER, "voronoi_map_river.bmp", w, h);
}

void VoronoiDiagramGenerator::SaveImage(int flag, const char* filename, unsigned int w, unsigned int h, bool restore) {


	FILE* out = nullptr;
	errno_t err = fopen_s(&out, filename, "wb");
	if (err != 0) {
		std::cout << "File write error!!\n";
		return;
	}

	auto start = std::clock();
	unsigned char* pixel_data = GetImage(flag, w, h, restore);


	//char pixel_data[IMAGE_WIDTH * IMAGE_HEIGHT * 300];
	//glReadPixels(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);

	auto duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;

	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	
	//const char* filename = "voronoi_map.bmp";


	//char* data = pixel_data;
	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));
	bf.bfType = 'MB';
	bf.bfSize = sizeof(bf) + sizeof(bi) + w * h * 3;
	bf.bfOffBits = sizeof(bf) + sizeof(bi);
	bi.biSize = sizeof(bi);
	bi.biWidth = w;
	bi.biHeight = h;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biSizeImage = w * h * 3;
	fwrite(&bf, sizeof(bf), 1, out);
	fwrite(&bi, sizeof(bi), 1, out);
	fwrite(pixel_data, sizeof(unsigned char), w * h * 3, out);
	fclose(out);
	delete[] pixel_data;

	auto duration2 = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "file saved: " << filename << ", create time: " << duration << "ms, save time: " << duration2 - duration << ", result: " << duration2 << "ms \n";

	if (restore) {
		SetupRiverTriangle(Color::lake);
		SetupColor(ALL_IMAGE);
		CreateTriangle();
	}

}



unsigned char* VoronoiDiagramGenerator::GetImage(int flag, unsigned int w, unsigned int h, bool restore) {
	unsigned char* pixel_data = new unsigned char[w * h * 3];
	std::fill(pixel_data, pixel_data + w * h * 3, 0);

	SetupColor(flag);
	CreateTriangle();
	if (flag == RIVER) {
		SetupRiverTriangle(Color(1, 1, 1));
	}
	else {
		SetupRiverTriangle(Color::lake);
	}

	if (image_flag != RIVER) {
		for (Triangle tri : diagram->triangles) {
			tri.AdjustSize(w, h, image_dim);
			tri.Draw(pixel_data, w, h);
		}
	}


	if (image_flag & RIVER) {

		for (RiverLine* line : diagram->river_lines.GetArray()) {
			for (Triangle tri : line->GetTriangle()) {
				tri.AdjustSize(w, h, image_dim);
				tri.DrawTransparent(pixel_data, w, h);
			}
		}
		for (Triangle tri : diagram->river_cross.GetTriangle()) {
			tri.AdjustSize(w, h, image_dim);
			tri.DrawTransparent(pixel_data, w, h);
		}
	}

	if (restore) {
		SetupRiverTriangle(Color::lake);
		SetupColor(ALL_IMAGE);
		CreateTriangle();
	}
	return pixel_data;
}


void VoronoiDiagramGenerator::CreateHeightmap(Heightmap& out_map, bool clear_image, int flag, unsigned int w, unsigned int h, bool restore) {
	
	auto start = std::clock();

	if (clear_image) {
		out_map.ClearImage();
	}

	SetupColor(flag);
	CreateTriangle();
	if (flag == RIVER) {
		SetupRiverTriangle(Color(1, 1, 1));
	}
	else {
		SetupRiverTriangle(Color::lake);
	}

	//Heightmap* pixel_data = new Heightmap(w, h);
	if (image_flag != RIVER) {
		for (Triangle tri : diagram->triangles) {
			tri.AdjustSize(w, h, image_dim);
			tri.DrawGrayscale(out_map, w, h);
		}
	}


	if (image_flag & RIVER) {

		for (RiverLine* line : diagram->river_lines.GetArray()) {
			for (Triangle tri : line->GetTriangle()) {
				tri.AdjustSize(w, h, image_dim);
				tri.DrawTransparentGrayscale(out_map, w, h);
			}
		}
		for (Triangle tri : diagram->river_cross.GetTriangle()) {
			tri.AdjustSize(w, h, image_dim);
			tri.DrawTransparentGrayscale(out_map, w, h);
		}
	}

	if (restore) {
		SetupRiverTriangle(Color::lake);
		SetupColor(ALL_IMAGE);
		CreateTriangle();
	}
	
	auto duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "Heightmap has been created: " << duration << "ms\n";
	//return pixel_data;
}