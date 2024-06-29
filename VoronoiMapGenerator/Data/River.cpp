#include "River.h"
#include "Setting.h"
#include <queue>
#include "../Edge.h"


//RiverCrossingMap RiverCrossing::RIVER_CROSSING_MAP = RiverCrossingMap();


std::vector< RiverEdge*> RiverEdge::RIVER_EDGES = std::vector< RiverEdge*>();
std::queue<RiverEdge*> RiverEdge::RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();

void RiverEdge::Clear() {

	for (RiverEdge* e : RIVER_EDGES) {
		delete e;
	}
	RIVER_EDGES.clear();
	RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();
	//while (!RIVER_DELETE_QUEUE.empty()) RIVER_DELETE_QUEUE.pop();

}

void RiverLines::Clear() {

	RIVER_EDGES_MAP.clear();
	RIVER_OUT_MAP.clear();
	LINKED_RIVER_EDGES.clear();
	LINKED_RIVERS.clear();
	RIVER_CNT.clear();
	//RIVER_DELETE_QUEUE = std::queue<RiverEdge*>();

}

RiverEdgeMap& RiverLines::GetRiverEdges() {
	return RIVER_EDGES_MAP;
}

RiverOutMap& RiverLines::GetRiverOutEdges() {
	return RIVER_OUT_MAP;
}

RiverEdge* RiverLines::GetRiverEdge(RiverPos pos) {
	return RIVER_EDGES_MAP[pos];
}

std::vector<RiverEdge*>& RiverLines::GetRiverOutEdge(unsigned int num) {
	return RIVER_OUT_MAP[num];
}

std::vector<Cell*>& RiverLines::GetLinkedRiverEdges(unsigned int num) {
	return LINKED_RIVER_EDGES[num];
}

void RiverLines::AddLinkRiverEdge(Cell* lakeA, Cell* lakeB) {
	lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
	lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
	LINKED_RIVER_EDGES[lakeA->GetUnique()].push_back(lakeB);
	LINKED_RIVER_EDGES[lakeB->GetUnique()].push_back(lakeA);
}

bool RiverLines::CheckRiverEdgeLinked(Cell* lakeA, Cell* lakeB) {
	lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
	lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
	if (lakeA != lakeB) {
		for (auto c : LINKED_RIVER_EDGES[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
	}
	
	return false;
}


std::vector<Cell*>& RiverLines::GetLinkedRiver(unsigned int num) {
	return LINKED_RIVERS[num];
}

void RiverLines::AddLinkRiver(Cell* lakeA, Cell* lakeB) {
	lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
	lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
	LINKED_RIVERS[lakeA->GetUnique()].push_back(lakeB);
	LINKED_RIVERS[lakeB->GetUnique()].push_back(lakeA);
}

bool RiverLines::CheckRiverLinked(Cell* lakeA, Cell* lakeB) {
	lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
	lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
	for (auto c : LINKED_RIVERS[lakeA->GetUnique()]) {
		if (c == lakeB) {
			return true;
		}
	}
	return false;
}

void RiverEdge::Initialize(RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	prevs.clear();
	nexts.clear();
	links.clear();
	is_start = false;
	start = startCell;
	end = endCell;
	owner = river_owner;
	dist = distance;
	river_lines = lines;
	if (pre_edge != nullptr) {
		prevs.push_back(pre_edge);
		pre_edge->nexts.push_back(this);
		//RIVER_OUT_MAP[start->GetUnique()].push_back(this);
	}

	river_lines->RIVER_OUT_MAP[start->GetUnique()].push_back(this);
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
		if (river_lines->RIVER_OUT_MAP.find(start->GetUnique()) != river_lines->RIVER_OUT_MAP.end()) {
			auto& arr = river_lines->RIVER_OUT_MAP[start->GetUnique()];
			if (arr.size() == 1) {
				river_lines->RIVER_OUT_MAP.erase(start->GetUnique());
			}
			else {
				std::vector<RiverEdge*> tmp;
				for (auto e : arr) {
					if (e != this) {
						tmp.push_back(e);
					}
				}
				river_lines->RIVER_OUT_MAP[start->GetUnique()] = tmp;

			}
		}
		for (auto pre_e : prevs) {
			if (pre_e->GetRiverEnd() && pre_e->GetNexts().size() <= 1) {
				pre_e->SetRiverEnd(false);
				//std::cout << "test\n";
			}
		}

		river_lines->RIVER_EDGES_MAP.erase(RiverEdge::GetPos(start, end));
		RIVER_DELETE_QUEUE.push(this);
		//delete this;
	}

}

RiverEdge* RiverEdge::Create(RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	RiverEdge* e;
	if (RIVER_DELETE_QUEUE.empty()) {
		e = new RiverEdge(lines, startCell, endCell, river_owner, pre_edge, next_edge, distance);
		e->unique = (unsigned int)(RIVER_EDGES.size());
		RIVER_EDGES.push_back(e);
	}
	else {
		e = RIVER_DELETE_QUEUE.front();
		RIVER_DELETE_QUEUE.pop();
		e->Initialize(lines, startCell, endCell, river_owner, pre_edge, next_edge, distance);
	}
	return e;
}

RiverEdge* RiverEdge::CreateStartPoint(RiverLines* lines, Cell* c) {
	RiverEdge* e = Create(lines, c, c, c, nullptr, nullptr, 0);
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
	return river_lines->RIVER_EDGES_MAP[RiverEdge::GetPos(GetOnwer(), GetOnwer())];
}


void RiverLines::AddOceanConnect(Cell* c) {
	int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
	if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
		RIVER_CNT[num] = 1;
	}
	else {
		RIVER_CNT[num]++;
	}
};
int RiverLines::GetOceanConnect(Cell* c) {
	int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
	if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
		return 0;
	}
	else {
		return RIVER_CNT[num];
	}
}




const double RiverTriangle::matrix_2[3][3] = { {2.0f, -4.0f, 2.0f}, {-3.0f, 4.0f, -1.0f}, {1.0f, 0.0f, 0.0f} };
const double RiverTriangle::matrix_3[4][4] = { {-1.0f, 3.0f, -3.0f, 1.0f},
						  {2.0f, -5.0f, 4.0f, -1.0f},
						  {-1.0f, 0.0f, 1.0f, 0.0f},
						  {0.0f, 2.0f, 0.0f, 0.0f} };


void RiverTriangle::CalcCardinal(RiverPointVector& point, double result[][2], int start) {
	memset(result, 0, sizeof(double) * 3 * 2);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result[i][0] += matrix_2[i][j] * point[j + start].point.x;
			result[i][1] += matrix_2[i][j] * point[j + start].point.y;
		}
	}

}

Point2 RiverTriangle::GetCardinalPoint(double result[][2], double t) {
	return Point2(result[2][0] + t * (result[1][0] + result[0][0] * t), result[2][1] + t * (result[1][1] + result[0][1] * t));
}


Point2 RiverTriangle::GetCardinalDirection(double result[][2], double prev, double next) {

	return (GetCardinalPoint(result, next) - GetCardinalPoint(result, prev)).Normailize();
}


void RiverTriangle::CreateLineTri(Triangles& tris, RiverPointVector& point, Color color, double radius, double river_scale, double spacing, bool fade_in, bool fade_out) {


	RiverPoint& pre_c = point[0];

	Point2 p1 = pre_c.point;
	RiverPoint& c = point[1];
	Point2 p2 = c.point;
	color.a = 1;
	Color c_trans = Color(color.r, color.g, color.b, 0);



	Point2 norm = (p2 - p1).Normailize();


	double sacle1 = (pre_c.power) * river_scale + 1;
	double sacle2 = (c.power) * river_scale + 1;

	Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
	Point2 PerpB = Point2(-norm.y, norm.x) * radius * sacle2;
	tris.push_back(Triangle({ p2, p1, (p1 - PerpA), color, color, c_trans }));
	tris.push_back(Triangle({ p2, p1, (p1 + PerpA), color, color, c_trans }));
	tris.push_back(Triangle({ p2, (p1 + PerpA), (p2 + PerpB), color, c_trans, c_trans }));
	tris.push_back(Triangle({ p2, (p1 - PerpA), (p2 - PerpB), color, c_trans, c_trans }));


	if (fade_in) {
		RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, color);
	}
	if (fade_out) {
		RiverTriangle::DrawCircle(tris, point[1].point, 100, norm.GetAngle() - 90, 50, radius, river_scale, point[1].power, color);
	}

}

void RiverTriangle::CreateCardinalTri(Triangles& tris, RiverPointVector& point, Color color, double radius, double river_scale, double spacing, bool fade_in, bool fade_out) {
	double result[3][2];
	//memset(result, 0, sizeof(result));
	CalcCardinal(point, result, 0);

	double t = 0.0f;
	double x, y;


	t = spacing;
	Point2 pre_p = point[0].point;
	Point2 pre_norm = point[0].point;

	//glBegin(GL_TRIANGLES);
	color.a = 1;
	Color c_trans = Color(color.r, color.g, color.b, 0);
	while (t < 1) {

		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);

		double x2 = result[2][0] + (t + spacing) * (result[1][0] + result[0][0] * (t + spacing));
		double y2 = result[2][1] + (t + spacing) * (result[1][1] + result[0][1] * (t + spacing));
		/*Color c;
		if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
			c = Color(0, 1, 0);
		}
		else {
			c = Color(1, 0, 0);
		}*/

		/*	std::cout << point[0].power << "\n";
			std::cout << point[2].power << "\n";*/
		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = (new_p - pre_p);
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[0].power * (1 - t) + point[2].power * t) * river_scale + 1;
		double sacle2 = (point[0].power * (1 - t + spacing) + point[2].power * t + spacing) * river_scale + 1;
		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

			tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), color, c_trans, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), color, c_trans, c_trans }));

		}

		if (t == spacing) {
			if (fade_in) {
				RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, color);
			}
		}
		pre_p = new_p;
		pre_norm = norm2;
		t += spacing;
	}
	//glEnd();


	//if (fade_in) {
	//	//RiverTriangle::DrawCircle(tris, point[point.size() - 1].point, 100, pre_norm.GetAngle() - 90, 50, radius, river_scale, point[2].power);

	//	double scale = radius * (point[0].power * river_scale + 1);
	//	Point2 norm = GetCardinalDirection(result, 0, spacing);
	//	Point2 PerpA = Point2(-norm.y, norm.x) * scale;

	//	Point2 temp1 = point[0].point + PerpA;
	//	Point2 temp2 = point[0].point - PerpA;
	//	Point2 temp3 = (temp1 + temp2) / 2;
	//	tris.push_back(Triangle({ temp3, temp1, point[0].GetCell()->site.p, c_half_red, c_trans, c_trans }));
	//	tris.push_back(Triangle({ temp3, temp2, point[0].GetCell()->site.p, c_half_red, c_trans, c_trans }));
	//
	//}


	if (fade_out) {
		RiverTriangle::DrawCircle(tris, point[2].point, 100, pre_norm.GetAngle() - 90, 50, radius, river_scale, point[2].power, color);

		/*	double scale = radius * (point[point.size() - 1].power * river_scale + 1);
			Point2 norm = GetCardinalDirection(result, 1, 1 + spacing);
			Point2 PerpA = Point2(-norm.y, norm.x) * scale;

			Point2 temp1 = point[point.size() - 1].point + PerpA;
			Point2 temp2 = point[point.size() - 1].point - PerpA;
			Point2 temp3 = (temp1 + temp2) / 2;
			tris.push_back(Triangle({ temp3, temp1, point[point.size() - 1].GetCell()->site.p, c_half_red, c_trans, c_trans }));
			tris.push_back(Triangle({ temp3, temp2, point[point.size() - 1].GetCell()->site.p, c_half_red, c_trans, c_trans }));
		*/
	}
}

void RiverTriangle::CreateSplineTri(Triangles& tris, RiverPointVector& point, Color color, double radius, double river_scale, double spacing, bool fade_in, bool fade_out) {

	double result[4][2];


	double t = 0.0f;
	double x, y;
	const size_t SIZE = point.size();
	// Section 1.
	// quadratic function

	CalcCardinal(point, result, 0);

	/*	if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
			glColor4f(0, 1, 0, 1);
		}
		else {
			glColor4f(1, 0, 0, 1);
		}*/


	color.a = 1;
	Color c_trans = Color(color.r, color.g, color.b, 0);
	t = spacing;
	Point2 pre_p = point[0].point;
	Point2 pre_norm = point[0].point;
	//glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
	//glBegin(GL_TRIANGLES);
	while (t < 0.5f) {

		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);

		double x2 = result[2][0] + (t + spacing) * (result[1][0] + result[0][0] * (t + spacing));
		double y2 = result[2][1] + (t + spacing) * (result[1][1] + result[0][1] * (t + spacing));


		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = (new_p - pre_p);
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[0].power * ((0.5 - t) * 2) + point[1].power * (t * 2)) * river_scale + 1;
		double sacle2 = (point[0].power * ((0.5 - (t + spacing)) * 2) + point[1].power * ((t + spacing) * 2)) * river_scale + 1;

		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

			tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), color, c_trans, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), color, c_trans, c_trans }));

		}

		if (t == spacing) {
			if (fade_in) {
				RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, color);
			}
		}

		pre_norm = norm2;
		pre_p = new_p;
		t += spacing;
	}

	/*	if (fade_in) {
			double scale = radius * (point[0].power * river_scale + 1);
			Point2 norm = GetCardinalDirection(result, 0, spacing);
			Point2 PerpA = Point2(-norm.y, norm.x) * scale;

			Point2 temp1 = point[0].point + PerpA;
			Point2 temp2 = point[0].point - PerpA;
			Point2 temp3 = (temp1 + temp2) / 2;
			tris.push_back(Triangle({ temp3, temp1, point[0].GetCell()->site.p, c_half_red, c_trans, c_trans }));
			tris.push_back(Triangle({ temp3, temp2, point[0].GetCell()->site.p, c_half_red, c_trans, c_trans }));
		}*/


		//glEnd();

		// Section 2.
		// cubic spline
	for (int cubic_case = 0; cubic_case < SIZE - 3; cubic_case++)
	{
		memset(result, 0, sizeof(result));
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result[i][0] += matrix_3[i][j] * point[j + cubic_case].point.x;
				result[i][1] += matrix_3[i][j] * point[j + cubic_case].point.y;
			}
		}

		//if (point[cubic_case + 1].GetCell()->GetDetail().GetElevation() == point[cubic_case + 2].GetCell()->GetDetail().GetElevation()) {
		//	glColor4f(0, 1, 0, 1);
		//}
		//else {
		//	glColor4f(1, 0, 0, 1);
		//}

		//t = 0.0f;
		////glBegin(GL_LINE_STRIP);
		//while (t < 1.0f) {
		//	x = (result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f;
		//	y = (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f;
		//	glVertex2f((GLfloat)normalize(x, dimension), -(GLfloat)normalize(y, dimension));
		//	t += 0.01f;
		//}
		////glEnd();





		t = 0.0f;
		//Point2 pre_p = point[0]->site.p;
		//Point2 pre_norm = point[0]->site.p;

		//glBegin(GL_TRIANGLES);
		while (t < 1.0f) {

			x = (result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f;
			y = (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f;

			double x2 = (result[3][0] + (t + spacing) * (result[2][0] + (t + spacing) * (result[1][0] + result[0][0] * (t + spacing)))) * 0.5f;
			double y2 = (result[3][1] + (t + spacing) * (result[2][1] + (t + spacing) * (result[1][1] + result[0][1] * (t + spacing)))) * 0.5f;


			Point2 new_p = Point2(x, y);
			Point2 next_p = Point2(x2, y2);
			Point2 norm = pre_norm;
			Point2 norm2 = (next_p - new_p);
			double sacle1 = (point[cubic_case + 1].power * (1 - t) + point[cubic_case + 2].power * t) * river_scale + 1;
			double sacle2 = (point[cubic_case + 1].power * (1 - (t + spacing)) + point[cubic_case + 2].power * (t + spacing)) * river_scale + 1;
			if (norm != Point2(0, 0)) {
				norm = norm.Normailize();
				norm2 = norm2.Normailize();
				Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
				Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), color, color, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), color, color, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), color, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), color, c_trans, c_trans }));
			}
			pre_norm = norm2;
			pre_p = new_p;
			t += spacing;
		}
		//glEnd();




	}

	// Section 3.
	// quadratic function
	CalcCardinal(point, result, (int)SIZE - 3);



	t = 0.5f + spacing;
	//glBegin(GL_TRIANGLES);
	while (t < 1.0f) {

		x = result[2][0] + t * (result[1][0] + result[0][0] * t);
		y = result[2][1] + t * (result[1][1] + result[0][1] * t);

		double x2 = result[2][0] + (t + spacing) * (result[1][0] + result[0][0] * (t + spacing));
		double y2 = result[2][1] + (t + spacing) * (result[1][1] + result[0][1] * (t + spacing));


		Point2 new_p = Point2(x, y);
		Point2 next_p = Point2(x2, y2);
		Point2 norm = pre_norm;
		Point2 norm2 = (next_p - new_p);
		double sacle1 = (point[SIZE - 2].power * (1 - t) + point[SIZE - 1].power * t) * river_scale + 1;
		double sacle2 = (point[SIZE - 2].power * (1 - (t + spacing)) + point[SIZE - 1].power * (t + spacing)) * river_scale + 1;
		if (norm != Point2(0, 0)) {
			norm = norm.Normailize();
			norm2 = norm2.Normailize();
			Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
			Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

			tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), color, color, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), color, c_trans, c_trans }));
			tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), color, c_trans, c_trans }));
		}
		pre_norm = norm2;
		pre_p = new_p;
		t += spacing;
	}

	if (fade_out) {
		RiverTriangle::DrawCircle(tris, point[SIZE - 1].point, 100, pre_norm.GetAngle() - 90, 50, radius, river_scale, point[SIZE - 1].power, color);
	}

	//if (fade_out) {
	//	double scale = radius * (point[point.size() - 1].power * river_scale + 1);
	//	Point2 norm = GetCardinalDirection(result, 1, 1 + spacing);
	//	Point2 PerpA = Point2(-norm.y, norm.x) * scale;

	//	Point2 temp1 = point[point.size() - 1].point + PerpA;
	//	Point2 temp2 = point[point.size() - 1].point - PerpA;
	//	Point2 temp3 = (temp1 + temp2) / 2;
	//	tris.push_back(Triangle({ temp3, temp1, point[point.size() - 1].GetCell()->site.p, c_half_red, c_trans, c_trans }));
	//	tris.push_back(Triangle({ temp3, temp2, point[point.size() - 1].GetCell()->site.p, c_half_red, c_trans, c_trans }));

	//}
	//glEnd();
};

void RiverTriangle::DrawCircle(Triangles& tris, Point2 center, const int num_segments, const double start, const int end, double radius, double river_scale, double power, Color color) {
	//const int num_segments = 100;
	//return;
	Color t_c = Color(color.r, color.g, color.b, 0);
	//glVertex2f(cx, cy); // ¿øÀÇ Áß½É
	for (int i = 0; i < end; i++) {
		double theta = 2.0f * 3.1415926f * (double(i) / double(num_segments) + start / 360); // ÇöÀç °¢µµ
		double x_ = radius * (river_scale * power + 1) * cosf((float)theta); // x ÁÂÇ¥
		double y_ = radius * (river_scale * power + 1) * sinf((float)theta); // y ÁÂÇ¥
		theta = 2.0f * 3.1415926f * (double(i + 1) / double(num_segments) + start / 360); // ÇöÀç °¢µµ
		double x_2 = radius * (river_scale * power + 1) * cosf((float)theta); // x ÁÂÇ¥
		double y_2 = radius * (river_scale * power + 1) * sinf((float)theta); // y ÁÂÇ¥
		tris.push_back(Triangle({
			Point2(x_ + center.x, y_ + center.y),
			Point2(x_2 + center.x, y_2 + center.y),
			Point2(center.x, center.y),
			t_c,
			t_c,
			color }));


	}
}
std::vector<RiverLine*> RiverLine::RIVER_LINE_ARR = std::vector<RiverLine*>();
unsigned int RiverLine::ADDED_COUNT = 0;



void RiverLine::Clear() {
	for (RiverLine* item : RIVER_LINE_ARR) {
		delete item;
	}
	RIVER_LINE_ARR.clear();
	ADDED_COUNT = 0;
}

void RiverLine::ClearJunk() {
	unsigned int cnt = 0;
	std::vector<RiverLine*> temp(ADDED_COUNT);
	for (RiverLine* item : RIVER_LINE_ARR) {
		if (item->used) {
			//std::cout << cnt << "\n";
			temp[cnt++] = item;
		}
		else {
			delete item;
		}
	}
	ADDED_COUNT = 0;
	RIVER_LINE_ARR = temp;
}

RiverLine* RiverLine::Create(GenerateSetting& setting) {
	RiverLine* item = new RiverLine(setting);
	RIVER_LINE_ARR.push_back(item);
	return item;
}

RiverLine* RiverLine::Create(const RiverLine& other) {
	RiverLine* item = new RiverLine(other);
	RIVER_LINE_ARR.push_back(item);
	return item;
}

void RiverLine::SetUsed() {
	if (!used) {
		used = true;
		ADDED_COUNT++;
	}
}

Triangles& RiverLine::GetTriangle() {
	return tris;
}

void RiverLine::AddPoint(RiverPoint p) {
	points.push_back(p);

}

RiverPointVector& RiverLine::GetPointArray() {
	return points;
}
RiverPoint& RiverLine::GetPoint(unsigned int index) {
	return points[index];
}

RiverPoint& RiverLine::GetFirstPoint() {
	return points[0];
}

RiverPoint& RiverLine::GetEndPoint() {
	return points[points.size() - 1];
}



void RiverLine::AdjustPoint() {
	RiverPoint& f_p = GetFirstPoint();
	RiverPoint& e_p = GetEndPoint();
	double f_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * f_p.power + 1) / 2;
	double e_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * e_p.power + 1) / 2;
	if (points.size() > 2) {

		double start_result[3][2];
		double end_result[3][2];
		RiverTriangle::CalcCardinal(points, start_result, 0);
		RiverTriangle::CalcCardinal(points, end_result, (int)points.size() - 3);

		f_p.point = f_p.point + RiverTriangle::GetCardinalDirection(start_result, 0, main_setting.GetRiverCurvSpacing()) * f_scale;

		e_p.point = e_p.point + RiverTriangle::GetCardinalDirection(end_result, 1, 1 - main_setting.GetRiverCurvSpacing()) * e_scale;
	}
	else {
		Point2 dir = (e_p.point - f_p.point).Normailize();

		f_p.point = f_p.point + dir * f_scale;
		e_p.point = e_p.point - dir * e_scale;
	}

}
void RiverLine::CreateTriangle(Color c) {
	tris.clear();
	if (points.size() > 3) {
		/*	double start_result[3][2];
			double end_result[3][2];
			memset(start_result, 0, sizeof(start_result));
			memset(end_result, 0, sizeof(end_result));
			CalcCardinal(start_result, 0);
			CalcCardinal(end_result, (int)points.size() - 3);*/

		RiverTriangle::CreateSplineTri(tris, points, c, main_setting.GetRiverRadius(), main_setting.GetRiverPowerScale(), main_setting.GetRiverCurvSpacing(), true, true);
	}
	else if (points.size() == 3) {
		RiverTriangle::CreateCardinalTri(tris, points, c, main_setting.GetRiverRadius(), main_setting.GetRiverPowerScale(), main_setting.GetRiverCurvSpacing(), true, true);
	}
	else {
		RiverTriangle::CreateLineTri(tris, points, c, main_setting.GetRiverRadius(), main_setting.GetRiverPowerScale(), main_setting.GetRiverCurvSpacing(), true, true);
	}

}




void RiverCrossing::Clear() {
	for (auto iter : RIVER_CROSSING_MAP) {
		iter.second.GetTriangle().clear();
	}
	RIVER_CROSSING_MAP.clear();
}

Cell* RiverCrossing::GetCell() {
	return cell;
}

RiverCrossing* RiverCrossing::Get(unsigned int cell_unique) {
	if (RIVER_CROSSING_MAP.find(cell_unique) == RIVER_CROSSING_MAP.end()) {
		return nullptr;
	}
	else {
		return &RIVER_CROSSING_MAP[cell_unique];
	}
}

void RiverCrossing::AddRiver(RiverLine* river) {
	Cell* f_c = river->GetFirstPoint().GetCell();
	Cell* e_c = river->GetEndPoint().GetCell();
	auto first_unique = f_c->GetUnique();
	auto end_unique = e_c->GetUnique();

	if (RIVER_CROSSING_MAP.find(first_unique) == RIVER_CROSSING_MAP.end()) {
		RIVER_CROSSING_MAP[first_unique] = RiverCrossing(f_c);
	}


	if (RIVER_CROSSING_MAP.find(end_unique) == RIVER_CROSSING_MAP.end()) {
		RIVER_CROSSING_MAP[end_unique] = RiverCrossing(e_c);
	}

	RiverCrossing& first_crossing = RIVER_CROSSING_MAP[first_unique];
	RiverCrossing& end_crossing = RIVER_CROSSING_MAP[end_unique];

	first_crossing.outputs.push_back(river);
	end_crossing.inputs.push_back(river);


}


void RiverCrossing::CreateCrossingPointTriagle(Color color, double radius, double river_scale, double spacing) {
	//return;
	//radius *= std::sqrt(2);

	for (auto& iter : RIVER_CROSSING_MAP) {
		RiverCrossing& rc = iter.second;
		rc.tris.clear();
		if (rc.inputs.size() > 0 && rc.outputs.size() > 0) {
			RiverPointVector points;
			RiverPoint& input_point = rc.inputs[0]->GetEndPoint();
			RiverPoint& ouput_point = rc.outputs[0]->GetFirstPoint();
			double power = 0;
			for (auto& p : rc.inputs) {
				power += p->GetEndPoint().power;
			}
			for (auto& p : rc.outputs) {
				power += p->GetFirstPoint().power;
			}
			power /= (rc.inputs.size() + rc.outputs.size());

			RiverPoint middle_point = RiverPoint((input_point.power + ouput_point.power) / 2, rc.GetCell());
			points.push_back(input_point);
			points.push_back(middle_point);
			points.push_back(ouput_point);

			//RiverTriangle::CreateCardinalTri(rc.tris, points, radius, river_scale, spacing);
			//continue;
			const int num_segments = 100;


			//glVertex2f(cx, cy); // ¿øÀÇ Áß½É
			color.a = 1;
			Color trans_c = Color(color.r, color.g, color.b, 0);
			for (int i = 0; i < num_segments; i++) {
				double theta = 2.0f * 3.1415926f * double(i) / double(num_segments); // ÇöÀç °¢µµ
				double x_ = radius * (river_scale * power + 1) * cosf((float)theta); // x ÁÂÇ¥
				double y_ = radius * (river_scale * power + 1) * sinf((float)theta); // y ÁÂÇ¥
				theta = 2.0f * 3.1415926f * double(i - 1) / double(num_segments); // ÇöÀç °¢µµ
				double x_2 = radius * (river_scale * power + 1) * cosf((float)theta); // x ÁÂÇ¥
				double y_2 = radius * (river_scale * power + 1) * sinf((float)theta); // y ÁÂÇ¥
				rc.tris.push_back(Triangle({
					Point2(x_ + rc.GetCell()->site.p.x, y_ + rc.GetCell()->site.p.y),
					Point2(x_2 + rc.GetCell()->site.p.x, y_2 + rc.GetCell()->site.p.y),
					Point2(rc.GetCell()->site.p.x, rc.GetCell()->site.p.y),
					trans_c,
					trans_c,
					color }));


			}

		}
	}
}

Triangles RiverCrossing::GetTriangle() {
	Triangles trianlges;
	for (auto& iter : RIVER_CROSSING_MAP) {
		trianlges.insert(trianlges.end(), iter.second.tris.begin(), iter.second.tris.end());
	}
	return trianlges;
}


void RiverLines::AddLine(RiverCrossing* corssing, RiverLine* line) {
	//if (line.GetPointArray().size() > 0) {
	if (line->GetPointArray().size() <= 1) return;
	line->SetUsed();
	RiverPointVector& vec = line->GetPointArray();
	auto cnt = vec.size();
	if (cnt == 2) {
		double chance = 1 - main_setting.GetRiverAdditionalCurveChance();
		double dist = main_setting.GetRiverAdditionalCurveDistance();
		int chance_cnt = 0;
		std::vector<bool> list = std::vector<bool>(cnt - 1, false);
		for (int i = 0, loop_cnt = cnt - 1; i < loop_cnt; i++) {
			list[i] = main_setting.GetRandom() >= chance;
			chance_cnt++;
		}

		RiverPointVector temp;
		temp.reserve(cnt + chance_cnt);
		temp.push_back(vec[0]);
		for (int i = 1; i < cnt; i++) {
			if (list[i - 1]) {
				Point2 p = (vec[i].point + vec[i - 1].point) / 2;
				//Point2 norm = (vec[i].point - vec[i - 1].point).Normailize();
				double power = (vec[i - 1].power + vec[i].power) / 2;
				Edge* e = nullptr;
				for (auto hf : vec[i - 1].GetCell()->halfEdges) {
					if (hf->edge->lSite->cell == vec[i].GetCell() ||
						(hf->edge->rSite && hf->edge->rSite->cell == vec[i].GetCell())) {
						e = hf->edge;
					}
				}
				double p_dist;
				if (e == nullptr) {
					p_dist = vec[i - 1].point.DistanceTo(vec[i].point);
				}
				else {
					p_dist = e->vertA->point.DistanceTo(e->vertB->point);
				}
				double dist = p_dist * main_setting.GetRiverAdditionalCurveDistance();
				double theta = 2.0f * 3.1415926f * (main_setting.GetRandom() * 360.0  / 360);
				double x = dist * cosf((float)theta);
				double y = dist * sinf((float)theta);
				//Point2 p = (vec[i - 1].point + vec[i].point) / 2 + Point2(main_setting.GetRandom(), main_setting.GetRandom()) * p_dist * main_setting.GetRiverAdditionalCurveDistance();
				RiverPoint rp = RiverPoint(power, nullptr, p + Point2(x, y));
				temp.push_back(rp);
			}
			temp.push_back(vec[i]);
		}
		//std::cout << temp.size() << "\n";
		line->GetPointArray() = temp;
	}

	corssing->AddRiver(line);
	lines.push_back(line);
	//std::cout << (*(lines.end() - 1)).GetPointArray().size() << "ADF\n";
	//std::cout << &lines[lines.size() - 1] << "ADF\n";
	//}
}

std::vector<RiverLine*>& RiverLines::GetArray() {
	return lines;
}

void RiverLines::AdjustPoint() {
	for (auto river : lines) {
		river->AdjustPoint();
	}
}

void RiverLines::CreateTriagle(Color c) {
	for (auto river : lines) {
		river->CreateTriangle(c);
	}

}