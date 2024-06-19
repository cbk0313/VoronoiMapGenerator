#pragma once
#include <vector>
#include "Data/Buffer.h"
#include "../Cell.h"
#include "Triangle.h"

class RiverEdge;
struct Cell;
template<template<typename> class Q, typename T, typename ...args>
class UniqueBuffer;

namespace River {
	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator() (const std::pair<T1, T2>& p) const {
			auto hash1 = std::hash<T1>{}(p.first);
			auto hash2 = std::hash<T2>{}(p.second);
			return hash1 ^ hash2; // 해시 값의 XOR를 사용하여 결합
		}
	};

	// 사용자 정의 비교 함수 (필요한 경우)
	struct pair_equal {
		template <class T1, class T2>
		bool operator() (const std::pair<T1, T2>& p1, const std::pair<T1, T2>& p2) const {
			return p1.first == p2.first && p1.second == p2.second;
		}
	};

	struct RiverPriorityComp {
		bool operator() (Cell* A, Cell* B) {
			if (A->GetDetail().GetElevation() < B->GetDetail().GetElevation()) {
				return true;
			}
			else {
				if (A->GetDetail().GetMoisture() < B->GetDetail().GetMoisture()) {
					return true;
				}
				return false;
			}
		}
	};


}


using RiverContainer = std::pair<Cell*, Cell*>;


using RiverPos = std::pair<int, int>;
using RiverEdgeMap = std::unordered_map<RiverPos, RiverEdge*, River::pair_hash, River::pair_equal>;
using RiverOutMap = std::unordered_map<unsigned int, std::vector<RiverEdge*>>;
using RiverLinkMap = std::unordered_map<unsigned int, std::vector<Cell*>>;
using RiverCntMap = std::unordered_map<unsigned int, unsigned int>;



class RiverEdge {
	std::vector<RiverEdge*> prevs;
	std::vector<RiverEdge*> nexts;
	std::vector<Cell*> links;
	bool is_start;
	bool is_end;
	Cell* start; // 나중에 유니크 번호로 변경?
	Cell* end;
	Cell* owner;
	int dist;
	int branch_dist;
	int power;

	static RiverEdgeMap RIVER_EDGES_MAP;
	static RiverOutMap RIVER_OUT_MAP;
	static RiverLinkMap LINKED_RIVER_EDGES;
	static RiverLinkMap LINKED_RIVERS;
	static RiverCntMap RIVER_CNT;
	static std::vector< RiverEdge*> RIVER_EDGES;
	static std::queue<RiverEdge*> RIVER_DELETE_QUEUE;
	unsigned int unique;
public:

	static void Clear();

	static RiverEdgeMap& GetRiverEdges() {
		return RIVER_EDGES_MAP;
	}

	static RiverOutMap& GetRiverOutEdges() {
		return RIVER_OUT_MAP;
	}

	static RiverEdge* GetRiverEdge(RiverPos pos) {
		return RIVER_EDGES_MAP[pos];
	}

	static std::vector<RiverEdge*>& GetRiverOutEdge(unsigned int num) {
		return RIVER_OUT_MAP[num];
	}


	static std::vector<Cell*>& GetLinkedRiverEdges(unsigned int num) {
		return LINKED_RIVER_EDGES[num];
	}

	static void AddLinkRiverEdge(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		LINKED_RIVER_EDGES[lakeA->GetUnique()].push_back(lakeB);
		LINKED_RIVER_EDGES[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverEdgeLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : LINKED_RIVER_EDGES[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
		return false;
	}


	static std::vector<Cell*>& GetLinkedRiver(unsigned int num) {
		return LINKED_RIVERS[num];
	}

	static void AddLinkRiver(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		LINKED_RIVERS[lakeA->GetUnique()].push_back(lakeB);
		LINKED_RIVERS[lakeB->GetUnique()].push_back(lakeA);
	}

	static bool CheckRiverLinked(Cell* lakeA, Cell* lakeB) {
		lakeA = lakeA->GetDetail().UnionFindCell(Terrain::OCEAN);
		lakeB = lakeB->GetDetail().UnionFindCell(Terrain::OCEAN);
		for (auto c : LINKED_RIVERS[lakeA->GetUnique()]) {
			if (c == lakeB) {
				return true;
			}
		}
		return false;
	}

	//RiverEdge() : is_start(false), start(nullptr), end(nullptr), dist(0) {};
	RiverEdge(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance)
	{ 
		Initialize(startCell, endCell, river_owner, pre_edge, next_edge, distance);
	};

	void Initialize(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
	int GetDistance() { return dist; };
	Cell* GetOnwer();
	void SetOnwer(Cell* c);
	std::vector<RiverEdge*>& GetPrevs();
	void AddPrev(RiverEdge* e);
	std::vector<RiverEdge*>& GetNexts();
	void AddNext(RiverEdge* e);


	int GetPower() { return power; };
	void SetPower(int num) { power = num; };
	bool IsStart() { return is_start; };
	void SetRiverEnd(bool b) { is_end = b; };
	bool GetRiverEnd() { return is_end; };
	Cell* GetStart() { return start; };
	Cell* GetEnd() { return end; };
	void SetStart(Cell* c) { start = c; };
	void SetEnd(Cell* c) { end = c; };
	void ChangeDist(int num);
	void SetDistAndNextAll(int num, Cell* river_owner);
	void SetDist(int num);
	void DeleteLine(std::vector<bool>& buf);
	static RiverEdge* Create(Cell* startCell, Cell* endCell, Cell* river_owner, RiverEdge* pre_edge, RiverEdge* next_edge, int distance);
	static RiverEdge* CreateStartPoint(Cell* c);
	static RiverPos GetPos(Cell* start, Cell* end);

	void ConnectPrev(RiverEdge* e);

	RiverEdge* GetOwnerEdge();

	static void AddOceanConnect(Cell* c) {
		int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
		if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
			RIVER_CNT[num] = 1;
		}
		else {
			RIVER_CNT[num]++;
		}
	};
	static int GetOceanConnect(Cell* c) {
		int num = c->GetDetail().UnionFindCell(Terrain::OCEAN)->GetUnique();
		if (RIVER_CNT.find(num) == RIVER_CNT.end()) {
			return 0;
		}
		else {
			return RIVER_CNT[num];
		}
	}
};


class RiverPoint {

	Cell* cell;
public:
	unsigned int power;
	Point2 point;
	RiverPoint(unsigned int pow, Cell* c, Point2 p = Point2(0, 0)) : power(pow), cell(c) {
		if (c != nullptr) {
			point = c->site.p;
		}
		else {
			point = p;
		}
	};

	Cell* GetCell() {
		return cell;
	}
};

using RiverPointVector = std::vector<RiverPoint>;

class RiverLine {

	static const double matrix_2[3][3];
	static const double matrix_3[4][4];


	RiverPointVector points;
	std::vector<Triangle> tris;
	double radius;
	double power_sacle;
	double line_step;
public:
	RiverLine(double _radius, double _power_sacle)
		: radius(_radius)
		, power_sacle(_power_sacle)
		, line_step(0.02f)
	{};

	std::vector<Triangle>& GetTriangle() {
		return tris;
	}

	void AddPoint(RiverPoint p) {
		points.push_back(p);
	}
	
	RiverPointVector& GetPointArray() {
		return points;
	}
	RiverPoint& GetPoint(unsigned int num) {
		return points[num];
	}
	
	RiverPoint& GetFirstPoint() {
		return points[0];
	}

	RiverPoint& GetEndPoint() {
		return points[points.size() - 1];
	}

	Point2 GetCardinalDirection(double result[][2], double prev, double next) {

		return (GetCardinalPoint(result, next) - GetCardinalPoint(result, prev)).Normailize();
	}


	void AdjustPoint() {
		RiverPoint& f_p = GetFirstPoint();
		RiverPoint& e_p = GetEndPoint();
		double f_scale = radius * (power_sacle * f_p.power + 1);
		double e_scale = radius * (power_sacle * e_p.power + 1);
		if (points.size() > 2) {

			double start_result[3][2];
			double end_result[3][2];
			CalcCardinal(start_result, 0);
			CalcCardinal(end_result, (int)points.size() - 3);

			f_p.point = f_p.point + GetCardinalDirection(start_result, 0, line_step) * f_scale;

			e_p.point = e_p.point + GetCardinalDirection(end_result, 1, 1 - line_step) * e_scale;
		}
		else {
			Point2 dir = (e_p.point - f_p.point).Normailize();

			f_p.point = f_p.point + dir * f_scale;
			e_p.point = e_p.point - dir * e_scale;
		}
		
	}
	void CrateTriangle() {

		if (points.size() > 2) {
			

			
			AdjustPoint();

			if (points.size() > 3) {
			/*	double start_result[3][2];
				double end_result[3][2];
				memset(start_result, 0, sizeof(start_result));
				memset(end_result, 0, sizeof(end_result));
				CalcCardinal(start_result, 0);
				CalcCardinal(end_result, (int)points.size() - 3);*/
				CreateSplineTri(radius, power_sacle);
			}
			else {

				CreateCardinalTri(radius, power_sacle);
			}


			
		}
		

	}

	



	// https://m.blog.naver.com/mykepzzang/220578028540
	// https://wtg-study.tistory.com/101
	// https://blog.naver.com/mykepzzang/220578097319
	// https://wtg-study.tistory.com/102

	void CalcCardinal(double result[][2], int start = 0) {
		memset(result, 0, sizeof(double) * 3 * 2);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				result[i][0] += matrix_2[i][j] * points[j + start].point.x;
				result[i][1] += matrix_2[i][j] * points[j + start].point.y;
			}
		}

	}

	Point2 GetCardinalPoint(double result[][2], double t) {
		return Point2(result[2][0] + t * (result[1][0] + result[0][0] * t), result[2][1] + t * (result[1][1] + result[0][1] * t));
	}





	void CreateCardinalTri(double radius, double river_scale) {
		double result[3][2];
		//memset(result, 0, sizeof(result));
		CalcCardinal(result, 0);

		double t = 0.0f;
		double x, y;


		RiverPointVector& point = points;

		t = line_step;
		Point2 pre_p = point[0].point;
		Point2 pre_norm = point[0].point;

		//glBegin(GL_TRIANGLES);
		Color c_red = Color(1, 0, 0, 1);
		Color c_trans = Color(1, 0, 0, 0);
		while (t < 1) {

			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);

			double x2 = result[2][0] + (t + line_step) * (result[1][0] + result[0][0] * (t + line_step));
			double y2 = result[2][1] + (t + line_step) * (result[1][1] + result[0][1] * (t + line_step));
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

			t += line_step;
			double sacle2 = (point[0].power * (1 - t) + point[2].power * t) * river_scale + 1;
			if (norm != Point2(0, 0)) {
				norm = norm.Normailize();
				norm2 = norm2.Normailize();
				Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
				Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));

			}
			pre_p = new_p;
		}
		//glEnd();


		double scale = radius * (point[0].power * river_scale + 1);
		Point2 norm = GetCardinalDirection(result, 0, line_step);
		Point2 PerpA = Point2(-norm.y, norm.x) * scale;

		Point2 temp1 = point[0].point + PerpA;
		Point2 temp2 = point[0].point - PerpA;
		Point2 temp3 = (temp1 + temp2) / 2;
		tris.push_back(Triangle({ temp3, temp1, point[0].GetCell()->site.p, c_red, c_trans, c_trans }));
		tris.push_back(Triangle({ temp3, temp2, point[0].GetCell()->site.p, c_red, c_trans, c_trans }));


		

		scale = radius * (point[point.size() - 1].power * river_scale + 1);
		norm = GetCardinalDirection(result, 1, 1 + line_step);
		PerpA = Point2(-norm.y, norm.x) * scale;

		temp1 = point[point.size() - 1].point + PerpA;
		temp2 = point[point.size() - 1].point - PerpA;
		temp3 = (temp1 + temp2) / 2;
		tris.push_back(Triangle({ temp3, temp1, point[point.size() - 1].GetCell()->site.p, c_red, c_trans, c_trans }));
		tris.push_back(Triangle({ temp3, temp2, point[point.size() - 1].GetCell()->site.p, c_red, c_trans, c_trans}));
	}

	void CreateSplineTri(double radius, double river_scale) {
		
		RiverPointVector& point = points;
		double result[4][2];

		double t = 0.0f;
		double x, y;
		const size_t SIZE = point.size();
		// Section 1.
		// quadratic function

		CalcCardinal(result, 0);

	/*	if (point[0].GetCell()->GetDetail().GetElevation() == point[1].GetCell()->GetDetail().GetElevation()) {
			glColor4f(0, 1, 0, 1);
		}
		else {
			glColor4f(1, 0, 0, 1);
		}*/


		Color c_red = Color(1, 0, 0, 1);
		Color c_trans = Color(1, 0, 0, 0);
		t = 0.0f;
		Point2 pre_p = point[0].point;
		Point2 pre_norm = point[0].point;
		//glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
		//glBegin(GL_TRIANGLES);
		while (t < 0.5f) {

			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);

			double x2 = result[2][0] + (t + line_step) * (result[1][0] + result[0][0] * (t + line_step));
			double y2 = result[2][1] + (t + line_step) * (result[1][1] + result[0][1] * (t + line_step));


			Point2 new_p = Point2(x, y);
			Point2 next_p = Point2(x2, y2);
			Point2 norm = (new_p - pre_p);
			Point2 norm2 = (next_p - new_p);
			double sacle1 = (point[0].power * ((0.5 - t) * 2) + point[1].power * (t * 2)) * river_scale + 1;
			if (t == 0) {
				t += line_step;
				continue;
			}
			t += line_step;
			double sacle2 = (point[0].power * ((0.5 - t) * 2) + point[1].power * (t * 2)) * river_scale + 1;
			if (norm != Point2(0, 0)) {
				norm = norm.Normailize();
				norm2 = norm2.Normailize();
				Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
				Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));
				
			}
			pre_p = new_p;
		}

		double scale = radius * (point[0].power * river_scale + 1);
		Point2 norm = GetCardinalDirection(result, 0, line_step);
		Point2 PerpA = Point2(-norm.y, norm.x) * scale;

		Point2 temp1 = point[0].point + PerpA;
		Point2 temp2 = point[0].point - PerpA;
		Point2 temp3 = (temp1 + temp2) / 2;
		tris.push_back(Triangle({ temp3, temp1, point[0].GetCell()->site.p, c_red, c_trans, c_trans }));
		tris.push_back(Triangle({ temp3, temp2, point[0].GetCell()->site.p, c_red, c_trans, c_trans }));



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

				double x2 = (result[3][0] + (t + line_step) * (result[2][0] + (t + line_step) * (result[1][0] + result[0][0] * (t + line_step)))) * 0.5f;
				double y2 = (result[3][1] + (t + line_step) * (result[2][1] + (t + line_step) * (result[1][1] + result[0][1] * (t + line_step)))) * 0.5f;


				Point2 new_p = Point2(x, y);
				Point2 next_p = Point2(x2, y2);
				Point2 norm = (new_p - pre_p);
				Point2 norm2 = (next_p - new_p);
				double sacle1 = (point[cubic_case + 1].power * (1 - t) + point[cubic_case + 2].power * t) * river_scale + 1;
				if (t == 0) {
					t += line_step;
					continue;
				}
				t += line_step;
				double sacle2 = (point[cubic_case + 1].power * (1 - t) + point[cubic_case + 2].power * t) * river_scale + 1;
				if (norm != Point2(0, 0)) {
					norm = norm.Normailize();
					norm2 = norm2.Normailize();
					Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
					Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

					tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
					tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
					tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
					tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));
}
				pre_p = new_p;
			}
			//glEnd();




		}

		// Section 3.
		// quadratic function
		CalcCardinal(result, (int)SIZE - 3);
	


		t = 0.5f + line_step;
		//glBegin(GL_TRIANGLES);
		while (t < 1.0f) {

			x = result[2][0] + t * (result[1][0] + result[0][0] * t);
			y = result[2][1] + t * (result[1][1] + result[0][1] * t);

			double x2 = result[2][0] + (t + line_step) * (result[1][0] + result[0][0] * (t + line_step));
			double y2 = result[2][1] + (t + line_step) * (result[1][1] + result[0][1] * (t + line_step));


			Point2 new_p = Point2(x, y);
			Point2 next_p = Point2(x2, y2);
			Point2 norm = (new_p - pre_p);
			Point2 norm2 = (next_p - new_p);
			double sacle1 = (point[SIZE - 2].power * (1 - t) + point[SIZE - 1].power * t) * river_scale + 1;

			t += line_step;
			double sacle2 = (point[SIZE - 2].power * (1 - t) + point[SIZE - 1].power * t) * river_scale + 1;
			if (norm != Point2(0, 0)) {
				norm = norm.Normailize();
				norm2 = norm2.Normailize();
				Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
				Point2 PerpB = Point2(-norm2.y, norm2.x) * radius * sacle2;

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));
			}
			pre_p = new_p;
		}



		scale = radius * (point[point.size() - 1].power * river_scale + 1);
		norm = GetCardinalDirection(result, 1, 1 + line_step);
		PerpA = Point2(-norm.y, norm.x) * scale;

		temp1 = point[point.size() - 1].point + PerpA;
		temp2 = point[point.size() - 1].point - PerpA;
		temp3 = (temp1 + temp2) / 2;
		tris.push_back(Triangle({ temp3, temp1, point[point.size() - 1].GetCell()->site.p, c_red, c_trans, c_trans }));
		tris.push_back(Triangle({ temp3, temp2, point[point.size() - 1].GetCell()->site.p, c_red, c_trans, c_trans }));


		//glEnd();
	};

};

class RiverCrossing;
using RiverCrossingMap = std::unordered_map<unsigned int, RiverCrossing>;

class RiverCrossing {


	static RiverCrossingMap RIVER_CROSSING_MAP;
	std::vector<RiverLine*> inputs;
	std::vector<RiverLine*> outputs;
public:
	RiverCrossing() {};
	static void Clear() {
		RIVER_CROSSING_MAP.clear();
	}

	static RiverCrossing* Get(unsigned int cell_unique) {
		if (RIVER_CROSSING_MAP.find(cell_unique) == RIVER_CROSSING_MAP.end()) {
			return nullptr;
		}
		else {
			return &RIVER_CROSSING_MAP[cell_unique];
		}
	}

	static void AddRiver(RiverLine& river) {
		auto first_unique = river.GetFirstPoint().GetCell()->GetUnique();
		auto end_unique = river.GetEndPoint().GetCell()->GetUnique();

		if (RIVER_CROSSING_MAP.find(first_unique) == RIVER_CROSSING_MAP.end()) {
			RIVER_CROSSING_MAP[first_unique] = RiverCrossing();
		}
		
		
		if (RIVER_CROSSING_MAP.find(end_unique) == RIVER_CROSSING_MAP.end()) {
			RIVER_CROSSING_MAP[end_unique] = RiverCrossing();
		}

		RiverCrossing& first_crossing = RIVER_CROSSING_MAP[first_unique];
		RiverCrossing& end_crossing = RIVER_CROSSING_MAP[end_unique];

		first_crossing.outputs.push_back(&river);
		end_crossing.inputs.push_back(&river);
		

		river.CrateTriangle();
		

	}
};


class RiverLines {
	std::vector<RiverLine> lines;
public:
	void AddLine(RiverLine& line) {
		RiverCrossing::AddRiver(line);
		lines.push_back(line);
	}

	std::vector<RiverLine>& GetArray() {
		return lines;
	}

};