#include "Buffer.h"
#include "..\Cell.h"
#include <algorithm>


CellQueue::CellQueue(size_t cnt, bool dedup) : UniqueBuffer(cnt, dedup) {
}

unsigned int CellQueue::GetUnique(Cell* c) {
	return c->GetUnique();
}

Cell* CellQueue::GetValue() {
	return buffer.front();
}


bool PriorityCellComp::operator() (Cell* A, Cell* B) {
	if (A->GetDetail().GetElevation() < B->GetDetail().GetElevation()) {
		return true;
	}
	else {
		return false;
	}
}

CellPriorityQueue::CellPriorityQueue(size_t cnt, bool dedup) : UniqueBuffer(cnt, dedup) {
}

unsigned int CellPriorityQueue::GetUnique(Cell* c) {
	return c->GetUnique();
}

Cell* CellPriorityQueue::GetValue() {
	return buffer.top();
}

CellVector::CellVector(size_t size) : cell_cnt(size) {
	isCalculating = std::vector<bool>(cell_cnt, false);
}

void CellVector::push_back(Cell* c) {
	if (!isCalculating[c->GetUnique()]) {
		isCalculating[c->GetUnique()] = true;
		VectorBuffer::push_back(c);
	}
}
