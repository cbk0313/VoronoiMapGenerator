#pragma once
#include <vector>
#include "Data/Buffer.h"
#include "../Cell.h"
#include "Triangle.h"

class RiverEdge;
struct Cell;
template<template<typename> class Q, typename T, typename ...args>
class UniqueBuffer;

class RiverPoint;

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
				if (A->GetDetail().GetElevation() == B->GetDetail().GetElevation()) {
					if (A->GetDetail().GetMoisture() < B->GetDetail().GetMoisture()) {
						return true;
					}
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

using RiverPointVector = std::vector<RiverPoint>;


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



class RiverTriangle {
private:

	static const double matrix_2[3][3];
	static const double matrix_3[4][4];

public:

	// https://m.blog.naver.com/mykepzzang/220578028540
	// https://wtg-study.tistory.com/101
	// https://blog.naver.com/mykepzzang/220578097319
	// https://wtg-study.tistory.com/102

	static void CalcCardinal(RiverPointVector& point, double result[][2], int start = 0) {
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

	static Point2 GetCardinalPoint(double result[][2], double t) {
		return Point2(result[2][0] + t * (result[1][0] + result[0][0] * t), result[2][1] + t * (result[1][1] + result[0][1] * t));
	}


	static Point2 GetCardinalDirection(double result[][2], double prev, double next) {

		return (GetCardinalPoint(result, next) - GetCardinalPoint(result, prev)).Normailize();
	}


	static void CreateLineTri(Triangles& tris, RiverPointVector& point, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false) {


		RiverPoint& pre_c = point[0];

		Point2 p1 = pre_c.point;
		RiverPoint& c = point[1];
		Point2 p2 = c.point;
		Color c_red = Color(1, 0, 0, 1);
		Color c_trans = Color(1, 0, 0, 0);



		Point2 norm = (p2 - p1).Normailize();


		double sacle1 = (pre_c.power) * river_scale + 1;
		double sacle2 = (c.power) * river_scale + 1;

		Point2 PerpA = Point2(-norm.y, norm.x) * radius * sacle1;
		Point2 PerpB = Point2(-norm.y, norm.x) * radius * sacle2;
		tris.push_back(Triangle({ p2, p1, (p1 - PerpA), c_red, c_red, c_trans }));
		tris.push_back(Triangle({ p2, p1, (p1 + PerpA), c_red, c_red, c_trans }));
		tris.push_back(Triangle({ p2, (p1 + PerpA), (p2 + PerpB), c_red, c_trans, c_trans }));
		tris.push_back(Triangle({ p2, (p1 - PerpA), (p2 - PerpB), c_red, c_trans, c_trans }));


		if (fade_in) {
			RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, c_red);
		}
		if (fade_out) {
			RiverTriangle::DrawCircle(tris, point[1].point, 100, norm.GetAngle() - 90, 50, radius, river_scale, point[1].power, c_red);
		}

	}

	static void CreateCardinalTri(Triangles& tris, RiverPointVector& point, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false) {
		double result[3][2];
		//memset(result, 0, sizeof(result));
		CalcCardinal(point, result, 0);

		double t = 0.0f;
		double x, y;


		t = spacing;
		Point2 pre_p = point[0].point;
		Point2 pre_norm = point[0].point;

		//glBegin(GL_TRIANGLES);
		Color c_red = Color(1, 0, 0, 1);
		Color c_half_red = Color(1, 0, 0, 1);
		Color c_trans = Color(1, 0, 0, 0);
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

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));

			}

			if (t == spacing) {
				if (fade_in) {
					RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, c_half_red);
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
			RiverTriangle::DrawCircle(tris, point[point.size() - 1].point, 100, pre_norm.GetAngle() - 90, 50, radius, river_scale, point[2].power, c_half_red);
			
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

	static void CreateSplineTri(Triangles& tris, RiverPointVector& point, double radius, double river_scale, double spacing, bool fade_in = false, bool fade_out = false) {

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


		Color c_red = Color(1, 0, 0, 1);
		Color c_half_red = Color(1, 0, 0, 1);
		Color c_trans = Color(1, 0, 0, 0);
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

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));

			}

			if (t == spacing) {
				if (fade_in) {
					RiverTriangle::DrawCircle(tris, point[0].point, 100, norm.GetAngle() + 90, 50, radius, river_scale, point[0].power, c_half_red);
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

					tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
					tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
					tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
					tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));
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

				tris.push_back(Triangle({ new_p, pre_p, (pre_p - PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, pre_p, (pre_p + PerpA), c_red, c_red, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p + PerpA), (new_p + PerpB), c_red, c_trans, c_trans }));
				tris.push_back(Triangle({ new_p, (pre_p - PerpA), (new_p - PerpB), c_red, c_trans, c_trans }));
			}
			pre_norm = norm2;
			pre_p = new_p;
			t += spacing;
		}

		if (fade_out) {
			RiverTriangle::DrawCircle(tris, point[SIZE - 1].point, 100, pre_norm.GetAngle() - 90, 50, radius, river_scale, point[SIZE - 1].power, c_half_red);
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

	static void DrawCircle(Triangles& tris, Point2 center, const int num_segments, const double start, const int end, double radius, double river_scale, double power, Color color) {
		//const int num_segments = 100;
		//return;
		Color t_c = Color(color.r, color.g, color.b, 0);
		//glVertex2f(cx, cy); // 원의 중심
		for (int i = 0; i < end; i++) {
			double theta = 2.0f * 3.1415926f * (double(i) / double(num_segments) + start / 360); // 현재 각도
			double x_ = radius * (river_scale * power + 1) * cosf((float)theta); // x 좌표
			double y_ = radius * (river_scale * power + 1) * sinf((float)theta); // y 좌표
			theta = 2.0f * 3.1415926f * (double(i + 1) / double(num_segments) + start / 360); // 현재 각도
			double x_2 = radius * (river_scale * power + 1) * cosf((float)theta); // x 좌표
			double y_2 = radius * (river_scale * power + 1) * sinf((float)theta); // y 좌표
			tris.push_back(Triangle({
				Point2(x_ + center.x, y_ + center.y),
				Point2(x_2 + center.x, y_2 + center.y),
				Point2(center.x, center.y),
				t_c,
				t_c,
				color }));


		}
	}

};


class RiverLine {

	static std::vector<RiverLine*> RIVER_LINE_ARR;
	static unsigned int ADDED_COUNT;

	RiverPointVector points;
	Triangles tris;
	double radius;
	double power_sacle;
	double curv_spacing;

	bool added;

	RiverLine(double _radius, double _power_sacle)
		: radius(_radius)
		, power_sacle(_power_sacle)
		, curv_spacing(0.02f)
		, added(false)
	{};

	RiverLine(const RiverLine& other) 
		: points(other.points)
		, tris(other.tris)
		, radius(other.radius)
		, power_sacle(other.power_sacle)
		, curv_spacing(other.curv_spacing)
		, added(false)
	{
		//std::cout << "복사 생성자 호출 " << points.size() << std::endl;
	}


public:

	static void Clear() {
		for (RiverLine* item : RIVER_LINE_ARR) {
			delete item;
		}
		RIVER_LINE_ARR.clear();
		ADDED_COUNT = 0;
	}
	
	static void ClearJunk() {
		unsigned int cnt = 0;
		std::vector<RiverLine*> temp(ADDED_COUNT);
		for (RiverLine* item : RIVER_LINE_ARR) {
			if (item->added) {
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

	static RiverLine* Create(double _radius, double _power_sacle) {
		RiverLine* item = new RiverLine(_radius, _power_sacle);
		RIVER_LINE_ARR.push_back(item);
		return item;
	}

	static RiverLine* Create(const RiverLine& other) {
		RiverLine* item = new RiverLine(other);
		RIVER_LINE_ARR.push_back(item);
		return item;
	}

	void SetAdd() {
		if (!added) {
			added = true;
			ADDED_COUNT++;
		}
	}
	
	Triangles& GetTriangle() {
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



	void AdjustPoint() {
		RiverPoint& f_p = GetFirstPoint();
		RiverPoint& e_p = GetEndPoint();
		double f_scale = radius * (power_sacle * f_p.power + 1) / 2;
		double e_scale = radius * (power_sacle * e_p.power + 1) / 2;
		if (points.size() > 2) {

			double start_result[3][2];
			double end_result[3][2];
			RiverTriangle::CalcCardinal(points, start_result, 0);
			RiverTriangle::CalcCardinal(points, end_result, (int)points.size() - 3);

			f_p.point = f_p.point + RiverTriangle::GetCardinalDirection(start_result, 0, curv_spacing) * f_scale;

			e_p.point = e_p.point + RiverTriangle::GetCardinalDirection(end_result, 1, 1 - curv_spacing) * e_scale;
		}
		else {
			Point2 dir = (e_p.point - f_p.point).Normailize();

			f_p.point = f_p.point + dir * f_scale;
			e_p.point = e_p.point - dir * e_scale;
		}
		
	}
	void CrateTriangle() {
		AdjustPoint();
			
		if (points.size() > 3) {
		/*	double start_result[3][2];
			double end_result[3][2];
			memset(start_result, 0, sizeof(start_result));
			memset(end_result, 0, sizeof(end_result));
			CalcCardinal(start_result, 0);
			CalcCardinal(end_result, (int)points.size() - 3);*/
			RiverTriangle::CreateSplineTri(tris, points, radius, power_sacle, curv_spacing, true, true);
		}
		else if(points.size() == 3){
			RiverTriangle::CreateCardinalTri(tris, points, radius, power_sacle, curv_spacing, true, true);
		}
		else {
			RiverTriangle::CreateLineTri(tris, points, radius, power_sacle, curv_spacing, true, true);
		}

			
		
		

	}

	
};

class RiverCrossing;
using RiverCrossingMap = std::unordered_map<unsigned int, RiverCrossing>;

class RiverCrossing {

	static RiverCrossingMap RIVER_CROSSING_MAP;
	std::vector<RiverLine*> inputs;
	std::vector<RiverLine*> outputs;
	Triangles tris;
	Cell* cell;

public:
	RiverCrossing() {};
	RiverCrossing(Cell* c) : cell(c) {};
	static void Clear() {
		for (auto iter : RIVER_CROSSING_MAP) {
			iter.second.GetTriangle().clear();
		}
		RIVER_CROSSING_MAP.clear();
	}

	Cell* GetCell() {
		return cell;
	}

	static RiverCrossing* Get(unsigned int cell_unique) {
		if (RIVER_CROSSING_MAP.find(cell_unique) == RIVER_CROSSING_MAP.end()) {
			return nullptr;
		}
		else {
			return &RIVER_CROSSING_MAP[cell_unique];
		}
	}

	static void AddRiver(RiverLine* river) {
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

		river->CrateTriangle();
		
	}

	static void CreateCrossingPointTriagle(double radius, double river_scale, double spacing) {
		//return;
		//radius *= std::sqrt(2);
		
		for (auto& iter : RIVER_CROSSING_MAP) {
			RiverCrossing& rc = iter.second;
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
				

				//glVertex2f(cx, cy); // 원의 중심

				for (int i = 0; i < num_segments; i++) {
					double theta = 2.0f * 3.1415926f * double(i) / double(num_segments); // 현재 각도
					double x_ = radius * (river_scale * power + 1) * cosf((float)theta); // x 좌표
					double y_ = radius * (river_scale * power + 1) * sinf((float)theta); // y 좌표
					theta = 2.0f * 3.1415926f * double(i - 1) / double(num_segments); // 현재 각도
					double x_2 = radius * (river_scale * power + 1) * cosf((float)theta); // x 좌표
					double y_2 = radius * (river_scale * power + 1) * sinf((float)theta); // y 좌표
					rc.tris.push_back(Triangle({
						Point2(x_ + rc.GetCell()->site.p.x, y_ + rc.GetCell()->site.p.y),
						Point2(x_2 + rc.GetCell()->site.p.x, y_2 + rc.GetCell()->site.p.y),
						Point2(rc.GetCell()->site.p.x, rc.GetCell()->site.p.y),
						Color(1.0f, 0.0f, 0.0f, 0.0f),
						Color(1.0f, 0.0f, 0.0f, 0.0f),
						Color(1.0f, 0.0f, 0.0f, 1.0f) }));

				
				}
				
			}
		}
	}

	static Triangles GetTriangle() {
		Triangles trianlges;
		for (auto& iter : RIVER_CROSSING_MAP) {
			trianlges.insert(trianlges.end(), iter.second.tris.begin(), iter.second.tris.end());
		}
		return trianlges;
	}
};


class RiverLines {
	std::vector<RiverLine*> lines;
public:
	void AddLine(RiverLine* line) {
		//if (line.GetPointArray().size() > 0) {
		line->SetAdd();
		RiverCrossing::AddRiver(line);
		lines.push_back(line);
		//std::cout << (*(lines.end() - 1)).GetPointArray().size() << "ADF\n";
		//std::cout << &lines[lines.size() - 1] << "ADF\n";
		//}
	}

	std::vector<RiverLine*>& GetArray() {
		return lines;
	}

};