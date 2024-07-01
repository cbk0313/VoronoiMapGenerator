#pragma once
#include <iostream>

using CellUniuq = unsigned int;

enum class Terrain : std::size_t;

template<typename T, std::size_t N>
struct UnionFind {

private:
	T* cell[N];
public:
	T* UnionFindCell(Terrain t);
	void SetUnionCell(Terrain t, T* target);
	void Reset(T* c) {
		for (unsigned int i = 0; i < N; i++) {
			cell[i] = c;
		}
	}

	UnionFind(T* c) {
		Reset(c);
	}

	T*& operator[](std::size_t index) {
		return cell[index];
	}

	const T*& operator[](std::size_t index) const {
		return cell[index];
	}
};


template<typename T, std::size_t N>
T* UnionFind<T, N>::UnionFindCell(Terrain t) {
	unsigned int num = static_cast<unsigned int>(t);
	if (cell[num] == cell[num]->GetDetail().unionfind[num]) {
		return cell[num];
	}
	else {
		return cell[num] = cell[num]->GetDetail().unionfind.UnionFindCell(t);
	}
}

template<typename T, std::size_t N>
void UnionFind<T, N>::SetUnionCell(Terrain t, T* target) {
	unsigned int num = static_cast<unsigned int>(t);
	auto& vim_d = UnionFindCell(t)->GetDetail();
	target = target->GetDetail().unionfind.UnionFindCell(t);
	vim_d.unionfind[num] = target;
	//if (vim_d.b_edge) target->GetDetail().unionfind[t] = vim_d.unionfind[t];
	//else {
	//	vim_d.unionfind[t] = target;
	//}
}


template<typename T>
struct UnionArray {
	// First is Unique number of representative cell, Second is the list of cells.
	std::unordered_map<CellUniuq, T> unions;

	UnionArray() {}
	T* findUnion(unsigned int uniuqe) {
		auto item = unions.find(uniuqe);
		if (item == unions.end()) {
			return nullptr;
		}
		return &item->second;
	}
	T* insert(unsigned int uniuqe) {
		T* uni = findUnion(uniuqe);
		if (uni == nullptr) {
			unions.insert(std::make_pair(uniuqe, T()));
			uni = findUnion(uniuqe);
		}
		return uni;
	}
};
