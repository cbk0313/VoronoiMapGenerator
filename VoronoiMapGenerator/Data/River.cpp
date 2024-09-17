#include "River.h"
#include "Setting.h"
#include <queue>
#include "../Edge.h"
#include "../Diagram.h"
#include "Utility.h"
#include "../VoronoiDiagramGenerator.h"

//RiverCrossingMap RiverCrossing::RIVER_CROSSING_MAP = RiverCrossingMap();

#define PI_ 3.1415926
#define CALC_RIVER_ELEVE(x) (std::max((double)x - 1, 0.0))
#define CALC_RIVER_ELEVE_CELL(c) CALC_RIVER_ELEVE(c->GetDetail().GetElevation())



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

void RiverEdge::Initialize(Diagram* l_diagram, RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	is_end = false;
	power = 0;
	diagram = l_diagram;
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

void RiverEdge::RefreshDistAndOwner(int num, Cell* new_owner) {
	SetDist(num);
	for (RiverEdge* re : nexts) {
		re->SetOnwer(new_owner);
		re->RefreshDistAndOwner(num + 1, new_owner);
	}
}

void RiverEdge::SetDist(int num) {
	dist = num;
}


void RiverEdge::DeleteLine(/*std::vector<bool>& buf*/) {

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
			pre_e->DeleteLine(/*buf*/);
		}
		//if(pre_e != nullptr) pre_e->DeleteLine(map, buf);
		//buf[end->GetUnique()] = false;
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
			if (pre_e->IsEnd() && pre_e->GetNexts().size() <= 1) {
				pre_e->SetRiverEnd(false);
				//std::cout << "test\n";
			}
		}

		river_lines->RIVER_EDGES_MAP.erase(RiverEdge::GetPos(start, end));
		diagram->RIVER_DELETE_QUEUE.push(this);
		//delete this;
	}

}

RiverEdge* RiverEdge::Create(Diagram* l_diagram, RiverLines* lines, Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance) {
	RiverEdge* e;
	if (l_diagram->RIVER_DELETE_QUEUE.empty()) {
		e = new RiverEdge(l_diagram, lines, startCell, endCell, river_owner, pre_edge, next_edge, distance);
		//e->unique = (unsigned int)(l_diagram->RIVER_EDGES.size());
		l_diagram->RIVER_EDGES.push_back(e);
	}
	else {
		e = l_diagram->RIVER_DELETE_QUEUE.front();
		l_diagram->RIVER_DELETE_QUEUE.pop();
		e->Initialize(l_diagram, lines, startCell, endCell, river_owner, pre_edge, next_edge, distance);
	}
	return e;
}

RiverEdge* RiverEdge::CreateStartPoint(Diagram* diagram, RiverLines* lines, Cell* c) {
	RiverEdge* e = Create(diagram, lines, c, c, c, nullptr, nullptr, 0);
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

void RiverLines::Initialize(Diagram* diagram) {
	mDiagram = diagram;
	CurveChance = diagram->GetSetting().GetRiverAdditionalCurveChance();
	CurveDistance = diagram->GetSetting().GetRiverAdditionalCurveDistance();
}

Diagram* RiverLines::GetDiagram() {
	return mDiagram;
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
			//result[i][0] += round(100 * matrix_2[i][j] * point[j + start].point.x) / 100;
			//result[i][1] += round(100 * matrix_2[i][j] * point[j + start].point.y) / 100;
			result[i][0] += matrix_2[i][j] * point[j + start].point.x;
			result[i][1] += matrix_2[i][j] * point[j + start].point.y;
		}
	}
}

void RiverTriangle::CalcCardinalEdge(RiverPointVector& point, RiverPointVector& left_point, RiverPointVector& right_point, Diagram* diagram, double main_result[][2], double left_result[][2], double right_result[][2], int start) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double radius = main_setting.GetRiverRadius();
	const double river_scale = main_setting.GetRiverPowerScale();
	for (int i = 0; i < 3; i++) {
		double dx = main_result[1][0] + (main_result[0][0]) * i;
		double dy = main_result[1][1] + (main_result[0][1]) * i;
		int cul_point = start + i;
		AddPoint(left_point, right_point, point[cul_point], diagram, Point2(dx, dy));

	}

	CalcCardinal(left_point, left_result, 0);
	CalcCardinal(right_point, right_result, 0);
}
//
//double FindFirstAnomaly(Cell* c, const Point2& pos, const Point2& norm, double start, double search_step) {
//	double left = 0;
//	double right = start;
//	double result = -1;
//
//	while (left <= right) {
//		double mid = left + (right - left) / 2;
//
//		if (!c->IsInside(pos + norm * mid)) {
//			right = mid - search_step;  
//		}
//		else {
//			left = mid + search_step; 
//			result = mid;
//		}
//	}
//
//	return result; 
//}
void RiverTriangle::AddPoint(RiverPointVector& left_points, RiverPointVector& right_points, RiverPoint& point, Diagram* diagram, Point2 dir) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	double length = point.GetRiverWidth(diagram);

	Point2 cross_norm = Point2(-dir.y, dir.x).Normalize();
	Point2 new_dot = point.point + cross_norm * length;
	/*if (point.cell && !point.cell->IsInside(new_dot)) {
		double find_dist = FindFirstAnomaly(point.cell, point.point, cross_norm, radius * scale, 10);
		new_dot = point.point + cross_norm * find_dist;
	}*/
	left_points.push_back(RiverPoint(point.power, point.elevation, nullptr, new_dot));
	
	cross_norm = Point2(-cross_norm.x, -cross_norm.y);
	new_dot = point.point + cross_norm * length;
	/*if (point.cell && !point.cell->IsInside(new_dot)) {
		double find_dist = FindFirstAnomaly(point.cell, point.point, cross_norm, radius * scale, 10);
		new_dot = point.point + cross_norm * find_dist;
	}*/
	right_points.push_back(RiverPoint(point.power, point.elevation, nullptr, new_dot));
}

void RiverTriangle::CalcSplineEdge(RiverPointVector& points, RiverPointVector& left_points, RiverPointVector& right_points, Diagram* diagram, double main_result[][2], double start_cardinal_result[][2], double end_cardinal_result[][2]) {
	size_t SIZE = points.size();
	CalcCardinal(points, start_cardinal_result, 0);
	AddPoint(left_points, right_points, points[0], diagram, GetCardinalDirection(start_cardinal_result, 0));

	for (int cubic_case = 0; cubic_case < SIZE - 3; cubic_case++) {
		CalcSpline(points, main_result, cubic_case);
		int cul_point = cubic_case + 1;
		AddPoint(left_points, right_points, points[cul_point], diagram, GetSplineDirection(main_result, 0));
	}

	CalcCardinal(points, end_cardinal_result, (int)SIZE - 3);
	AddPoint(left_points, right_points, points[SIZE - 2], diagram, GetCardinalDirection(end_cardinal_result, 0.5));
	AddPoint(left_points, right_points, points[SIZE - 1], diagram, GetCardinalDirection(end_cardinal_result, 1));
}

void RiverTriangle::CalcSpline(RiverPointVector& point, double result[][2], int cubic_case) {
	memset(result, 0, sizeof(double) * 4 * 2);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i][0] += matrix_3[i][j] * point[j + cubic_case].point.x;
			result[i][1] += matrix_3[i][j] * point[j + cubic_case].point.y;
		}
	}
}

Point2 RiverTriangle::GetCardinalPoint(double result[][2], double t) {
	return Point2(result[2][0] + t * (result[1][0] + result[0][0] * t), result[2][1] + t * (result[1][1] + result[0][1] * t));
}

Point2 RiverTriangle::GetCardinalDirection(double result[][2], double t) {
	return Point2(result[1][0] + 2 * (result[0][0]) * t, result[1][1] + 2 * (result[0][1]) * t).Normalize();
}

Point2 RiverTriangle::GetSplinePoint(double result[][2], double t) {
	return Point2((result[3][0] + t * (result[2][0] + t * (result[1][0] + result[0][0] * t))) * 0.5f, (result[3][1] + t * (result[2][1] + t * (result[1][1] + result[0][1] * t))) * 0.5f);
}

Point2 RiverTriangle::GetSplineDirection(double result[][2], double t) {
	double dx = 0.5 * (result[2][0] + t * (2 * result[1][0] + 3 * result[0][0] * t));
	double dy = 0.5 * (result[2][1] + t * (2 * result[1][1] + 3 * result[0][1] * t));
	return Point2(dx, dy).Normalize();
}

void RiverTriangle::AddCurvedTri(Triangles& tris, RiverPointVector& point, RiverDrawBox& drawBox, double& pre_angle_left, double& pre_angle_right, double t, double radius, double spacing, bool is_end) {
	if (drawBox.mPreNorm != Point2(0, 0)) {
		//drawBox.mPreNorm = drawBox.mPreNorm.Normalize();
		//drawBox.mNextNorm = drawBox.mNextNorm.Normalize();


		double range = 2;
		double pow_lange = std::pow(range, 2);

		double between;
		double angle;

		double angle_left;
		double angle_right;

		if (!is_end) {
			int pow_size = 3;
			double weight = 2;
			between = Point2::AngleBetween(drawBox.mNextNorm, drawBox.mPreNorm);
			angle = std::clamp(between * weight, -range, range);


			angle_left = range - angle;
			angle_right = range + angle;
			//angle_left /= pow_lange;
			//angle_right /= pow_lange
			angle_left /= 2;
			angle_right /= 2;
			
			//angle_left = std::max(angle_left, 0.2);
			//angle_right = std::max(angle_right, 0.2);
		}
		else {
			between = 0;
			angle = 0;

			angle_left = 1;
			angle_right = 1;
		}


		double curv_add = std::pow(std::abs(t - 0.5), 2) * spacing + std::clamp((std::abs(between) / 40), 0.0, spacing);

		double curv_weight = spacing + curv_add;
		curv_weight = std::clamp(curv_weight, 0.0, 1.0);

		if (is_end) {
			double end_t = (t - 0.5) * 2;
			angle_left = angle_left * end_t + pre_angle_left * (1 - end_t);
			angle_right = angle_right * end_t + pre_angle_right * (1 - end_t);
		}
		else {
			angle_left = angle_left * curv_weight + pre_angle_left * (1 - curv_weight);
			angle_right = angle_right * curv_weight + pre_angle_right * (1 - curv_weight);
		}

		angle_left = std::clamp(angle_left, 0.2, 1.0);
		angle_right = std::clamp(angle_right, 0.2, 1.0);

		VertexColor pre_trans = drawBox.mPreColor;
		pre_trans.rgb.a = 0;

		VertexColor next_trans = drawBox.mNextColor;
		next_trans.rgb.a = 0;
		drawBox.mNextLeft = (drawBox.mNextLeft - drawBox.mNextMain) * angle_left + drawBox.mNextMain;
		drawBox.mNextRight = (drawBox.mNextRight - drawBox.mNextMain) * angle_right + drawBox.mNextMain;
		
		tris.push_back(Triangle({ drawBox.mNextMain, drawBox.mPreMain, drawBox.mPreLeft, drawBox.mNextColor, drawBox.mPreColor, pre_trans }));
		tris.push_back(Triangle({ drawBox.mNextMain, drawBox.mPreMain, drawBox.mPreRight, drawBox.mNextColor, drawBox.mPreColor, pre_trans }));
		tris.push_back(Triangle({ drawBox.mNextMain, drawBox.mPreLeft, drawBox.mNextLeft, drawBox.mNextColor, pre_trans, next_trans }));
		tris.push_back(Triangle({ drawBox.mNextMain, drawBox.mPreRight, drawBox.mNextRight, drawBox.mNextColor, pre_trans, next_trans }));


		pre_angle_left = angle_left;
		pre_angle_right = angle_right;
		drawBox.UpdatePrePoint();

	}
}

VertexColor RiverTriangle::CalcColor(Diagram* diagram, VertexColor& pre_c, RiverPoint& pre_p, RiverPoint& next_p, const double& color_rate, double t) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double sea_level = main_setting.GetSeaLevel();
	VertexColor next_c;
	if (diagram->GetImageFlag() == ALL_IMAGE) {
		pre_c = Color::lake;
		next_c = Color::lake;
	}
	else {
		VertexColor pre_color = VoronoiDiagramGenerator::CalcIslandColor(CALC_RIVER_ELEVE(pre_p.elevation), sea_level, color_rate);
		next_c = VoronoiDiagramGenerator::CalcIslandColor(CALC_RIVER_ELEVE(next_p.elevation), sea_level, color_rate);
		double reverse_t = 1 - t;
		next_c.rgb = pre_color.rgb * reverse_t + next_c.rgb * t;
		next_c.gray = (uint16_t)(pre_color.gray * reverse_t + next_c.gray * t);
		if (next_c.gray >= pre_c.gray) {
			return pre_c;
		}
	}
	return next_c;
}

void RiverTriangle::CreateLineTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in, bool fade_out) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double sea_level = main_setting.GetSeaLevel();
	const double radius = main_setting.GetRiverRadius();
	const double river_scale = main_setting.GetRiverPowerScale();

	const RiverPoint& pre_c = point[0];

	Point2 p1 = pre_c.point;
	const RiverPoint& c = point[1];
	Point2 p2 = c.point;



	Point2 norm = (p2 - p1).Normalize();


	double scale1 = (pre_c.power) * river_scale + 1;
	double scale2 = (c.power) * river_scale + 1;

	VertexColor pre_color = VoronoiDiagramGenerator::CalcIslandColor((double)point[0].GetCell()->GetDetail().GetElevation() - 1, sea_level, color_rate);
	VertexColor next_color = VoronoiDiagramGenerator::CalcIslandColor((double)point[1].GetCell()->GetDetail().GetElevation() - 1, sea_level, color_rate);
	VertexColor color_trans = pre_color;
	color_trans.rgb.a = 0;

	Point2 PerpA = Point2(-norm.y, norm.x) * radius * scale1;
	Point2 PerpB = Point2(-norm.y, norm.x) * radius * scale2;
	tris.push_back(Triangle({ p2, p1, (p1 - PerpA), next_color, pre_color, color_trans }));
	tris.push_back(Triangle({ p2, p1, (p1 + PerpA), next_color, pre_color, color_trans }));
	tris.push_back(Triangle({ p2, (p1 + PerpA), (p2 + PerpB), next_color, color_trans, color_trans }));
	tris.push_back(Triangle({ p2, (p1 - PerpA), (p2 - PerpB), next_color, color_trans, color_trans }));


	if (fade_in) {
		double length = point[0].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, length, pre_color);
	}
	if (fade_out) {
		double length = point[1].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[1].point, 100, norm.GetAngle() - 90, 50, length, next_color);
	}

}

void RiverTriangle::CreateCardinalTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in, bool fade_out) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double sea_level = main_setting.GetSeaLevel();
	const double radius = main_setting.GetRiverRadius();
	const double river_scale = main_setting.GetRiverPowerScale();
	const double spacing = main_setting.GetRiverCurvSpacing();
	
	double result[3][2];
	double left_result[3][2];
	double right_result[3][2];

	CalcCardinal(point, result, 0);

	double t = spacing;
	Point2 pre_p = point[0].point;
	Point2 main_dot = GetCardinalPoint(result, t);

	VertexColor pre_c;
	if (diagram->GetImageFlag() == ALL_IMAGE) {
		pre_c = Color::lake;
	}
	else {
		pre_c = VoronoiDiagramGenerator::CalcIslandColor(CALC_RIVER_ELEVE(point[0].elevation), sea_level, color_rate);
	}

	double pre_angle_left = 1;
	double pre_angle_right = 1;

	RiverPointVector left_point;
	RiverPointVector right_point;
	CalcCardinalEdge(point, left_point, right_point, diagram, result, left_result, right_result);

	Point2 pre_left_p = left_point[0].point;
	Point2 pre_right_p = right_point[0].point;
	Point2 pre_norm = GetCardinalDirection(result, 0);

	if (fade_in) {
		double length = point[0].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[0].point, 100, pre_norm.GetAngle() + 90, 50, length, pre_c);
	}

	while (t < 1) {
		int cul_point = t < 0.5 ? 0 : 1;

		Point2 new_p = GetCardinalPoint(result, t);
		Point2 left_p = GetCardinalPoint(left_result, t);
		Point2 right_p = GetCardinalPoint(right_result, t);
		Point2 next_norm = GetCardinalDirection(result, t);

		RiverPoint& pre_rp = point[cul_point].GetCell() == nullptr ? point[cul_point - 1] : point[cul_point];
		RiverPoint& next_rp = point[cul_point + 1].GetCell() == nullptr ? point[cul_point + 2] : point[cul_point + 1];
		
		VertexColor next_c = CalcColor(diagram, pre_c, pre_rp, next_rp, color_rate, t);

		RiverDrawBox Box = RiverDrawBox(pre_p, new_p, pre_left_p, left_p, pre_right_p, right_p, pre_norm, next_norm, pre_c, next_c);
		AddCurvedTri(tris, point, Box, pre_angle_left, pre_angle_right, t, radius, spacing, t > 0.5);

		t += spacing;
	}

	if (fade_out) {
		double length = point[2].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[2].point, 100, pre_norm.GetAngle() - 90, 50, length, pre_c);
	}
}

void RiverTriangle::CreateSplineTri(Triangles& tris, RiverPointVector& point, Diagram* diagram, const double& color_rate, bool fade_in, bool fade_out) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double sea_level = main_setting.GetSeaLevel();
	const double radius = main_setting.GetRiverRadius();
	const double river_scale = main_setting.GetRiverPowerScale();
	const double spacing = main_setting.GetRiverCurvSpacing();

	double start_cardinal_result[3][2];
	double main_result[4][2];
	double end_cardinal_result[3][2];
	double left_result[4][2];
	double right_result[4][2];

	const size_t SIZE = point.size();
	RiverPointVector left_points;
	RiverPointVector right_points;
	CalcSplineEdge(point, left_points, right_points, diagram, main_result, start_cardinal_result, end_cardinal_result);


	Point2 pre_left_p = left_points[0].point;
	Point2 pre_right_p = right_points[0].point;
	Point2 pre_p = point[0].point;

	VertexColor pre_c;
	if (diagram->GetImageFlag() == ALL_IMAGE) {
		pre_c = Color::lake;
	}
	else {
		pre_c = VoronoiDiagramGenerator::CalcIslandColor(CALC_RIVER_ELEVE(point[0].elevation), sea_level, color_rate);
	}

	double pre_angle_left = 1;
	double pre_angle_right = 1;

	Point2 pre_norm = GetCardinalDirection(start_cardinal_result, 0);
	if (fade_in) {
		double length = point[0].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[0].point, 100, pre_norm.GetAngle() + 90, 50, length, pre_c);
	}

	double t = spacing;

	CalcCardinal(left_points, left_result, 0);
	CalcCardinal(right_points, right_result, 0);

	while (t < 0.5f) {

		Point2 new_p = GetCardinalPoint(start_cardinal_result, t);
		Point2 left_p = GetCardinalPoint(left_result, t);
		Point2 right_p = GetCardinalPoint(right_result, t);
		Point2 next_norm = GetCardinalDirection(start_cardinal_result, t);

		double t_scale = t * 2;
		VertexColor next_c = CalcColor(diagram, pre_c, point[0], point[1], color_rate, t_scale);

		RiverDrawBox Box = RiverDrawBox(pre_p, new_p, pre_left_p, left_p, pre_right_p, right_p, pre_norm, next_norm, pre_c, next_c);
		AddCurvedTri(tris, point, Box, pre_angle_left, pre_angle_right, t, radius, spacing, t > 0.5);
		t += spacing;
	}

	for (int cubic_case = 0; cubic_case < SIZE - 3; cubic_case++) {
		CalcSpline(point, main_result, cubic_case);
		CalcSpline(left_points, left_result, cubic_case);
		CalcSpline(right_points, right_result, cubic_case);
		int cul_point = cubic_case + 1;

		//CalcSplineEdge(point, left_point, right_point, diagram, main_result, left_result, right_result, cul_point, cul_point);

		t = spacing;
		while (t < 1.0f) {
			Point2 new_p = GetSplinePoint(main_result, t);
			Point2 left_p = GetSplinePoint(left_result, t);
			Point2 right_p = GetSplinePoint(right_result, t);
			Point2 next_norm = GetSplineDirection(main_result, t);

			VertexColor next_c = CalcColor(diagram, pre_c, point[cul_point], point[cul_point + 1], color_rate, t);
			RiverDrawBox Box = RiverDrawBox(pre_p, new_p, pre_left_p, left_p, pre_right_p, right_p, pre_norm, next_norm, pre_c, next_c);
			AddCurvedTri(tris, point, Box, pre_angle_left, pre_angle_right, t, radius, spacing);

			t += spacing;
		}
	}

	CalcCardinal(left_points, left_result, (int)SIZE - 3);
	CalcCardinal(right_points, right_result, (int)SIZE - 3);
	t = 0.5f + spacing;
	while (t < 1.0f) {
		Point2 new_p = GetCardinalPoint(end_cardinal_result, t);
		Point2 left_p = GetCardinalPoint(left_result, t);
		Point2 right_p = GetCardinalPoint(right_result, t);
		Point2 next_norm = GetCardinalDirection(end_cardinal_result, t);

		double t_scale = (t - 0.5) * 2;
		VertexColor next_c = CalcColor(diagram, pre_c, point[SIZE - 2], point[SIZE - 1], color_rate, t_scale);
		RiverDrawBox Box = RiverDrawBox(pre_p, new_p, pre_left_p, left_p, pre_right_p, right_p, pre_norm, next_norm, pre_c, next_c);
		AddCurvedTri(tris, point, Box, pre_angle_left, pre_angle_right, t, radius, spacing, t > 0.5);
		t += spacing;
	}
	if (fade_out) {
		double length = point[SIZE - 1].GetRiverWidth(diagram);
		RiverTriangle::DrawCircle(tris, point[SIZE - 1].point, 100, pre_norm.GetAngle() - 90, 50, length, pre_c);
	}
};

void RiverTriangle::DrawCircle(Triangles& tris, Point2 center, const int num_segments, const double start, const int end, double radius, VertexColor color) {

	//const int num_segments = 100;
	//return;
	VertexColor t_c = VertexColor(Color(color.rgb.r, color.rgb.g, color.rgb.b, 0), color.gray);
	//glVertex2f(cx, cy); 
	for (int i = 0; i < end; i++) {
		double theta = 2.0f * 3.1415926f * (double(i) / double(num_segments) + start / 360);
		double x_ = radius * cosf((float)theta);
		double y_ = radius * sinf((float)theta);
		theta = 2.0f * 3.1415926f * (double(i + 1) / double(num_segments) + start / 360);
		double x_2 = radius * cosf((float)theta);
		double y_2 = radius * sinf((float)theta);
		tris.push_back(Triangle({
			Point2(x_ + center.x, y_ + center.y),
			Point2(x_2 + center.x, y_2 + center.y),
			Point2(center.x, center.y),
			t_c,
			t_c,
			color }));


	}
}


//
//RiverLine::RiverLine(double radius, double powerScale, double curvSpacing)
//	: used(false)
//	, Radius(radius)
//	, PowerScale(powerScale)
//	, CurvSpacing(curvSpacing)
//	//, radius(_radius)
//	//, power_sacle(_power_sacle)
//	//, curv_spacing(0.02f)
//
//{};

Diagram* RiverLine::GetDiagram() {
	return diagram;
}

RiverLine* RiverLine::Create(Diagram* l_diagram) {
	RiverLine* item = new RiverLine(l_diagram);
	l_diagram->RIVER_LINE_ARR.push_back(item);
	return item;
}

RiverLine* RiverLine::Create(const RiverLine& other) {
	RiverLine* item = new RiverLine(other);
	other.diagram->RIVER_LINE_ARR.push_back(item);
	return item;
}


void RiverLine::SetUsed() {
	if (!used) {
		used = true;
		diagram->ADDED_COUNT++;
	}
}
bool RiverLine::GetUsed() {
	return used;
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

	//double f_scale = round(100 * main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * f_p.power + 1) / 2) / 100;
	//double e_scale = round(100 * main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * e_p.power + 1) / 2) / 100;
	const GenerateSetting& main_setting = GetDiagram()->GetSetting();
	double f_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * f_p.power + 1) / 2;
	double e_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * e_p.power + 1) / 2;

	if (points.size() > 2) {

		double start_result[3][2];
		double end_result[3][2];
		RiverTriangle::CalcCardinal(points, start_result, 0);
		RiverTriangle::CalcCardinal(points, end_result, (int)points.size() - 3);

		f_p.point = f_p.point + RiverTriangle::GetCardinalDirection(start_result, 0) * f_scale;
		if (points[points.size() - 1].GetCell()->GetDetail().GetTerrain() != Terrain::OCEAN) {
			e_p.point = e_p.point - RiverTriangle::GetCardinalDirection(end_result, 1) * e_scale;
		}
		

		}
	else {
		Point2 dir = (e_p.point - f_p.point).Normalize();

		f_p.point = f_p.point + dir * f_scale;
		if (points[points.size() - 1].GetCell()->GetDetail().GetTerrain() != Terrain::OCEAN) {
			e_p.point = e_p.point - dir * e_scale;
		}
	}

}


void RiverLine::RepairPoint() {

	RiverPoint& f_p = GetFirstPoint();
	RiverPoint& e_p = GetEndPoint();

	//double f_scale = round(100 * main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * f_p.power + 1) / 2) / 100;
	//double e_scale = round(100 * main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * e_p.power + 1) / 2) / 100;
	const GenerateSetting& main_setting = GetDiagram()->GetSetting();
	double f_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * f_p.power + 1) / 2;
	double e_scale = main_setting.GetRiverRadius() * (main_setting.GetRiverPowerScale() * e_p.power + 1) / 2;

	if (points.size() > 2) {

		double start_result[3][2];
		double end_result[3][2];
		RiverTriangle::CalcCardinal(points, start_result, 0);
		RiverTriangle::CalcCardinal(points, end_result, (int)points.size() - 3);

		f_p.point = f_p.point - RiverTriangle::GetCardinalDirection(start_result, 0) * f_scale;
		if (points[points.size() - 1].GetCell()->GetDetail().GetTerrain() != Terrain::OCEAN) {
			e_p.point = e_p.point + RiverTriangle::GetCardinalDirection(end_result, 1) * e_scale;
		}


	}
	else {
		Point2 dir = (e_p.point - f_p.point).Normalize();

		f_p.point = f_p.point - dir * f_scale;
		if (points[points.size() - 1].GetCell()->GetDetail().GetTerrain() != Terrain::OCEAN) {
			e_p.point = e_p.point + dir * e_scale;
		}
	}

}

void RiverLine::CreateTriangle(double color_rate) {
	tris.clear();
	if (points.size() > 3) {
		RiverTriangle::CreateSplineTri(tris, points, GetDiagram(), color_rate, true, true); //true, true
	}
	else if (points.size() == 3) {
		RiverTriangle::CreateCardinalTri(tris, points, GetDiagram(), color_rate, true, true);
	}
	else {
		RiverTriangle::CreateLineTri(tris, points, GetDiagram(), color_rate, true, true);
	}

}


Cell* RiverCrossing::GetCell() {
	return cell;
}

Triangles& RiverCrossing::GetTriangle() {
	return tris;
}

void RiverCrossingMap::Initialize(Diagram* diagram) {
	mDiagram = diagram;
}

void RiverCrossingMap::Clear() {
	for (auto iter : GetMap()) {
		iter.second.GetTriangle().clear();
	}
	GetMap().clear();
}

Diagram* RiverCrossingMap::GetDiagram() {
	return mDiagram;
}

RiverCrossing* RiverCrossingMap::Get(unsigned int cell_unique) {
	if (GetMap().find(cell_unique) == GetMap().end()) {
		return nullptr;
	}
	else {
		return &GetMap()[cell_unique];
	}
}

void RiverCrossingMap::AddRiver(RiverLine* river) {
	Cell* f_c = river->GetFirstPoint().GetCell();
	Cell* e_c = river->GetEndPoint().GetCell();
	auto first_unique = f_c->GetUnique();
	auto end_unique = e_c->GetUnique();

	if (GetMap().find(first_unique) == GetMap().end()) {
		GetMap()[first_unique] = RiverCrossing(f_c);
	}


	if (GetMap().find(end_unique) == GetMap().end()) {
		GetMap()[end_unique] = RiverCrossing(e_c);
	}

	RiverCrossing& first_crossing = GetMap()[first_unique];
	RiverCrossing& end_crossing = GetMap()[end_unique];

	first_crossing.GetOutputs().push_back(river);
	end_crossing.GetInputs().push_back(river);


}


void RiverCrossingMap::CreateCrossingPointTriagle(Diagram* diagram, double color_rate) {
	//return;
	//radius *= std::sqrt(2);
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double spacing = 0.1;
	for (auto& iter : GetMap()) {
		RiverCrossing& rc = iter.second;
		rc.GetTriangle().clear();
		if (rc.GetInputs().size() > 0 && rc.GetOutputs().size() > 0) {
			RiverPointVector points;
			//RiverPoint& input_point = rc.GetInputs()[0]->GetEndPoint();
			//RiverPoint& ouput_point = rc.GetOutputs()[0]->GetFirstPoint();
			int elev = rc.GetCell()->GetDetail().GetElevation();
			double power = 0;
			unsigned int power_stack = 0;
			for (auto& p : rc.GetInputs()) {
				power_stack += p->GetEndPoint().power;
				elev = std::min(elev, p->GetEndPoint().elevation);
			}
			for (auto& p : rc.GetOutputs()) {
				power_stack += p->GetFirstPoint().power;
				elev = std::min(elev, p->GetFirstPoint().elevation);
			}
			//power = round(((double)power_stack / (rc.GetInputs().size() + rc.GetOutputs().size())) * 100) / 100;
			power = (double)power_stack / (rc.GetInputs().size() + rc.GetOutputs().size());

			//RiverPoint middle_point = RiverPoint((input_point.power + ouput_point.power) / 2, rc.GetCell());
			//points.push_back(input_point);
			//points.push_back(middle_point);
			//points.push_back(ouput_point);

			//RiverTriangle::CreateCardinalTri(rc.tris, points, radius, river_scale, spacing);
			//continue;
			const int num_segments = 100;

			
			//glVertex2f(cx, cy);
			VertexColor color;
			if (GetDiagram()->GetImageFlag() == ALL_IMAGE) {
				color = Color::lake;
			}
			else {
				color = VoronoiDiagramGenerator::CalcIslandColor(CALC_RIVER_ELEVE(elev), main_setting.GetSeaLevel(), color_rate);
			}

			const double radius = main_setting.GetRiverRadius();
			const double river_scale = main_setting.GetRiverPowerScale();
			double length = std::min(main_setting.GetCellSize(), radius * (power * (river_scale + 1)));

			
			VertexColor trans_c = VertexColor(Color(color.rgb.r, color.rgb.g, color.rgb.b, 0), color.gray);
			for (int i = 0; i < num_segments; i++) {
				double theta = 2.0f * PI_ * double(i) / double(num_segments);
				//theta = round(theta / 100) * 100;

				//double x_ = radius * (round(100 * (river_scale * power + 1)) / 100) * (round(100 * cosf((float)theta)) / 100);
				//double y_ = radius * (round(100 * (river_scale * power + 1)) / 100) * (round(100 * sinf((float)theta)) / 100);
				double x_ = length * cosf((float)theta);
				double y_ = length * sinf((float)theta);
				//x_ = round(x_ / 100) * 100;
				//y_ = round(y_ / 100) * 100;
				//3.1415926
				theta = 2.0f * PI_ * double(i - 1) / double(num_segments);
				//theta = round(theta / 100) * 100;

				//double x_2 = radius * (round(100 * (river_scale * power + 1)) / 100) * (round(100 * cosf((float)theta)) / 100);
				//double y_2 = radius * (round(100 * (river_scale * power + 1)) / 100) * (round(100 * sinf((float)theta)) / 100);
				double x_2 = length * cosf((float)theta);
				double y_2 = length * sinf((float)theta);
				//x_2 = round(x_2 / 100) * 100;
				//y_2 = round(y_2 / 100) * 100;

				rc.GetTriangle().push_back(Triangle({
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

Triangles RiverCrossingMap::GetTriangle() {
	Triangles trianlges;
	for (auto& iter : GetMap()) {
		trianlges.insert(trianlges.end(), iter.second.GetTriangle().begin(), iter.second.GetTriangle().end());
	}
	return trianlges;
}

void RiverLines::AddLine(RiverLine* line) {
	RiverCrossingMap& corssing = GetDiagram()->GetRiverCrossing();
	//if (line.GetPointArray().size() > 0) {
	if (line->GetPointArray().size() <= 1) return;
	line->SetUsed();
	RiverPointVector& vec = line->GetPointArray();

	//Add one random point
	auto cnt = vec.size();
	int elev = vec[0].elevation;
	if (cnt == 2) {
		double chance = 1 - CurveChance;
		//double dist = main_setting.GetRiverAdditionalCurveDistance();
		int chance_cnt = 0;
		std::vector<bool> list = std::vector<bool>(cnt - 1, false);
		for (unsigned int i = 0, loop_cnt = cnt - 1; i < loop_cnt; i++) {
			list[i] = GetDiagram()->GetSetting().GetRandom() >= chance;
			chance_cnt++;
		}

		RiverPointVector temp;
		temp.reserve(cnt + chance_cnt);
		temp.push_back(vec[0]);
		for (int i = 1; i < cnt; i++) {
			elev = std::min(elev, vec[0].elevation);
			if (list[i - 1]) {
				Point2 p = (vec[i].point + vec[i - 1].point) / 2;
				double power = (vec[i - 1].power + vec[i].power) / 2;
				Edge* e = nullptr;
				for (auto hf : vec[i - 1].GetCell()->halfEdges) {
					if (hf->edge->lSite->cell == vec[i].GetCell() ||
						(hf->edge->rSite && hf->edge->rSite->cell == vec[i].GetCell())) {
						e = hf->edge;
					}
				}
				double p_dist = e->vertA->point.DistanceTo(e->vertB->point);
				/*if (e == nullptr) {
					p_dist = vec[i - 1].point.DistanceTo(vec[i].point);
				}
				else {
					p_dist = e->vertA->point.DistanceTo(e->vertB->point);
				}*/
				double dist = p_dist * CurveDistance;
				double theta = 2.0f * PI_ * (GetDiagram()->GetSetting().GetRandom() / 360);
				double x = dist * cosf((float)theta);
				double y = dist * sinf((float)theta);
				//Point2 p = (vec[i - 1].point + vec[i].point) / 2 + Point2(main_setting.GetRandom(), main_setting.GetRandom()) * p_dist * main_setting.GetRiverAdditionalCurveDistance();
				RiverPoint rp = RiverPoint((unsigned int)power, elev, nullptr, p + Point2(x, y));
				temp.push_back(rp);
			}
			temp.push_back(vec[i]);
		}
		//std::cout << temp.size() << "\n";
		line->GetPointArray() = temp;
	}

	corssing.AddRiver(line);
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

void RiverLines::RepairPoint() {
	for (auto river : lines) {
		river->RepairPoint();
	}
}

void RiverLines::CreateTriagle(double color_rate) {
	for (auto river : lines) {
		river->CreateTriangle(color_rate);
	}

}


double RiverPoint::GetRiverWidth(Diagram* diagram) {
	const GenerateSetting& main_setting = diagram->GetSetting();
	const double radius = main_setting.GetRiverRadius();
	const double river_scale = main_setting.GetRiverPowerScale();
	return std::min(main_setting.GetCellSize(), radius * (power * (river_scale + 1)));
}

#undef PI_