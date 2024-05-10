#pragma once
#include<queue>
#include<vector>
#include<stack>

template<typename T>
struct QueueBuffer {
protected:
	std::queue<T> buffer;
public:
	inline virtual std::queue<T>& GetBuffer() {
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
	inline virtual T front() {
		return buffer.front();
	}
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


struct Cell;
struct CellBuffer : QueueBuffer<Cell*> {
private:
	size_t cell_cnt;
	std::vector<bool> isCalculating;
	bool deduplication;
public:

	CellBuffer(size_t cnt, bool dedupl = true);
	void SetPopDeduplication(bool b);

	virtual void push(Cell* c) override;
	//virtual Cell* top() override;
	virtual void pop() override;
	
};


struct CellVector : VectorBuffer<Cell*> {
private:
	size_t cell_cnt;
	std::vector<bool> isCalculating;
public:

	CellVector(size_t cnt);

	virtual void push_back(Cell* c) override;

};