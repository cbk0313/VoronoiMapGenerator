#pragma once
#include "MapData.h"

template<typename T, std::size_t N>
T* UnionFind<T, N>::unionFindCell(std::size_t t) {
	if (cell[t] == cell[t]->detail.unionfind[t]) {
		return cell[t];
	}
	else {
		return cell[t] = cell[t]->detail.unionfind.unionFindCell(t);
	}
}

template<typename T, std::size_t N>
void UnionFind<T, N>::setUnionCell(std::size_t t, T* target) {
	auto& vim_d = unionFindCell(t)->detail;
	target = target->detail.unionfind.unionFindCell(t);
	vim_d.unionfind[t] = target;
	//if (vim_d.is_edge) target->detail.unionfind[t] = vim_d.unionfind[t];
	//else {
	//	vim_d.unionfind[t] = target;
	//}
}

//
//
//Cell* CellDetail::unionFindCell() {
//	if (unionCell == unionCell->detail.unionCell) {
//		return unionCell;
//	}
//	else {
//		return unionCell = unionCell->detail.unionFindCell();
//	}
//}
//
//void CellDetail::setUnionCell(Cell* target) {
//	auto vim_d = unionFindCell()->detail;
//	target = target->detail.unionFindCell();
//	if (vim_d.is_edge) target->detail.unionCell = vim_d.unionCell;
//	else {
//		unionFindCell()->detail.unionCell = target;
//	}
//	
//}
