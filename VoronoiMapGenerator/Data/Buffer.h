#pragma once

#include<queue>
#include<vector>
#include<stack>

template<template<typename> class Q, typename T, typename ...args>
class UniqueBuffer {
protected:
	Q<args...> buffer;
	size_t cell_cnt;
	std::vector<bool> isCalculating;
	bool deduplication;

	inline virtual unsigned int GetUnique(T t) = 0;

	inline virtual void push_to_buffer(T t) {
		buffer.push(t);
	}

	inline virtual void pop_to_buffer() {
		buffer.pop();
	}
private:
	UniqueBuffer() {};
public:

	UniqueBuffer(size_t size, bool dedupl = true) : cell_cnt(size) {
		isCalculating = std::vector<bool>(cell_cnt, false);
		SetPopDeduplication(dedupl);
	}

	inline virtual T GetValue() = 0;

	void SetPopDeduplication(bool b) {
		deduplication = b;
	}


	inline virtual Q<args...>& GetBuffer() {
		return buffer;
	}
	inline virtual bool push(T t) {
		if (!isCalculating[GetUnique(t)]){
			isCalculating[GetUnique(t)] = true;
			push_to_buffer(t);
			return true;
		}
		return false;
		//buffer.push(t);
	}

	inline virtual void pop() {

		if (deduplication) isCalculating[GetUnique(GetValue())] = false;
		pop_to_buffer();

		//buffer.pop();
	}

	inline virtual bool empty() {
		return buffer.empty();
	}
	
	/*inline virtual T front() {
		return buffer.front();
	}*/
};

template<typename T>
struct VectorBuffer {
protected:
	std::vector<T> buffer;
public:
	inline virtual std::vector<T>& GetBuffer() {
		return buffer;
	}

	inline virtual void push_back(T t) {
		buffer.push_back(t);
	}

	inline virtual void pop_back() {
		buffer.pop_back();
	}

	inline virtual bool empty() {
		return buffer.empty();
	}
	inline virtual T front() {
		return buffer.front();
	}
};


template<typename T>
struct StackBuffer {
protected:
	std::stack<T> buffer;
public:
	inline virtual std::stack<T>& GetBuffer() {
		return buffer;
	}
	inline virtual void push(T t) {
		buffer.push(t);
	}

	inline virtual void pop() {
		buffer.pop();
	}

	inline virtual bool empty() {
		return buffer.empty();
	}
	inline virtual T top() {
		return buffer.top();
	}
};

template<template<typename> class ARR_TYPE, typename MAIN_T, typename UNIQUE_F, typename VALUE_F, typename ...SUB_T_ARGS>
class UniBuf : public UniqueBuffer<ARR_TYPE, MAIN_T, SUB_T_ARGS...> {
private:
	UNIQUE_F unique_f;
	VALUE_F value_f;
	inline virtual unsigned int GetUnique(MAIN_T c) override {
		return unique_f(c);
	}
public:
	

	UniBuf(UNIQUE_F u_f, VALUE_F v_f, size_t cnt, bool dedup = true) 
		: UniqueBuffer<ARR_TYPE, MAIN_T, SUB_T_ARGS...>(cnt, dedup)
		, unique_f(u_f)
		, value_f(v_f)
	{};

	inline virtual MAIN_T GetValue() override {
		return value_f(&this->buffer);
	}
};

struct Cell;
class CellQueue : public UniqueBuffer<std::queue, Cell*, Cell*> {
private:
	inline virtual unsigned int GetUnique(Cell* c) override;
public:
	CellQueue(size_t cnt, bool dedup = true);
	inline virtual Cell* GetValue() override;
};


struct PriorityCellComp {
	bool operator() (Cell* A, Cell* B);
};

class CellPriorityQueue : public UniqueBuffer<std::priority_queue, Cell*, Cell*, std::vector<Cell*>, PriorityCellComp>{
private:
	inline virtual unsigned int GetUnique(Cell* c) override;
public:
	CellPriorityQueue(size_t cnt, bool dedup = true);
	inline virtual Cell* GetValue() override;
};







struct CellVector : VectorBuffer<Cell*> {
private:
	size_t cell_cnt;
	std::vector<bool> isCalculating;
public:

	CellVector(size_t cnt);

	virtual void push_back(Cell* c) override;

};

