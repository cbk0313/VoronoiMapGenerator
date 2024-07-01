#pragma once
#include "CellDetail.h"
#include "../Cell.h"
#include "../Edge.h"


bool CellDetail::IsEdge() {
	return GetUnionFind().UnionFindCell(Terrain::OCEAN)->GetDetail().GetEdge();
}
bool CellDetail::GetEdge() {
	return b_edge;
}

bool CellDetail::IsFlat() {
	return b_flat;
}

bool CellDetail::IsPeak() {
	return unionfind.UnionFindCell(Terrain::PEAK)->GetDetail().b_peak;
}

bool CellDetail::GetPeak() {
	return b_peak;
}


void CellDetail::SetFlat(bool b) {
	b_flat = b;
}

void CellDetail::SetPeak(bool b) {
	b_peak = b;
}

bool CellDetail::IsHighestPeak() {
	return unionfind.UnionFindCell(Terrain::HIGHEST_PEAK)->GetDetail().b_highest_peak;
}

void CellDetail::SetHighestPeak(bool b) {
	b_highest_peak = b;
}

bool CellDetail::GetHighestPeak() {
	return b_highest_peak;
}

Terrain CellDetail::GetTerrain() {
	return terrain;
}

Color& CellDetail::GetColor() {
	return color;
}

int CellDetail::GetElevation() {
	if (b_flat) {
		return elevation;
	}
	else {
		return elevation;
	}
}
void CellDetail::SetElevation(int num) {
	elevation = num;
}
void CellDetail::AddElevation(int num) {
	elevation += num;
}

unsigned int CellDetail::GetMoisture() {
	//return GetAreaMoisture() / 36 + GetLocalMoisture();
	return moisture;
}

void CellDetail::SetMoisture(unsigned int num) {
	moisture = num;
}

void CellDetail::AddMoisture(unsigned int num) {
	moisture += num;
}


unsigned int CellDetail::GetBiome() {
	return biome;
}
void CellDetail::SetBiome(unsigned int num) {
	biome = num;
}
void CellDetail::AddBiome(unsigned int num) {
	biome += num;
}

UnionFind<Cell, TERRAIN_CNT>& CellDetail::GetUnionFind() {
	return unionfind;
}

Cell* CellDetail::UnionFindCell(Terrain t) {
	return unionfind.UnionFindCell(t);
}

CellDetail& CellDetail::UnionFindCellDetail(Terrain t) {
	return unionfind.UnionFindCell(t)->GetDetail();
}

void CellDetail::SetUnionCell(Terrain t, Cell* c) {
	unionfind.SetUnionCell(t, c);
}

void CellDetail::SetTerrain(Terrain t) {
	terrain = t;
}

void CellDetail::SetEdge(bool b) {
	b_edge = b;
}

void CellDetail::Reset(bool reset_edge, bool reset_terrain, bool reset_elev) {
	if (reset_edge) {
		b_edge = false;
	}
	b_peak = false;
	b_highest_peak = false;
	if (reset_elev) {
		elevation = 0;
	}
	if (reset_terrain) {
		SetTerrain(Terrain::OCEAN);
	}
	unionfind.Reset(cell);
}



bool CellDetail::CheckSurroundRising() {
	CellDetail& cd = cell->GetDetail();
	for (HalfEdge* he : cell->halfEdges) {
		Edge* e = he->edge;
		Cell* targetCell = (e->lSite->cell == cell && e->rSite) ? e->rSite->cell : e->lSite->cell;
		CellDetail& tcd = targetCell->GetDetail();

		if (cd.GetTerrain() == Terrain::LAND && tcd.GetTerrain() == Terrain::LAND) {
			if (cd.GetElevation() < tcd.GetElevation()) {
				return true;
			}
		}
	}
	return false;
}