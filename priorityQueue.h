#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

// we could use the normal array  but the took how t handle in the lab so, we are tring another implemenation with vector as
// it will be dynamic array without using the manual functions of the array as size, capcity ..etc

#include <vector> 
// from the functions that will be used from vector is 
// push_back , pop_back 
#include <utility>
using namespace std;

// we should know that we did not depend on the linked list in the priority queue so we will not use the node header file 
// we depened on the use of the vector  (falt array)

template<typename T>

class priorityQueue {

private:
	vector<T> dataarray;
	// data that will be stored in the queue on the dijkestra algorithium

	// to keep the order of the queue 2 functions needed to be implemeneted 

	// shift up --> just to check if the new inseted iteam is smaller than its parent or not 
	void shiftUP(int index) {
		// just to swap the new item if it is smaller than the existing parent to maintain the priority queue
		while (index > 0) {
			int parent = (index - 1) / 2; // to set the parent index
			if (dataarray[index] < dataarray[parent]) {
				swap(dataarray[index], dataarray[parent]);
				index = parent;
			}
			else {
				break;
			}
		}

	}

	// the second needed function to shift down the after the removal of the item 

	void shiftdown(int index) {
		int size = static_cast<int> (dataarray.size());
		while (true) {
			// initialieze the right and the left, and the smallest values 
			int leftdata = 2 * index + 1;
			int rightdata = 2 * index + 2;
			int smallest = index;

			//checking step for the left
			if (leftdata < size && dataarray[leftdata] < dataarray[smallest]) {
				smallest= leftdata;
			}

			// for the right 
			if (rightdata < size && dataarray[rightdata] < dataarray[smallest]) {
				smallest = rightdata;
			}

			// check the smallest withe the index to swap
			if (smallest != index) {
				swap(dataarray[index], dataarray[smallest]);
				index = smallest;
			}

			else {
				break;
			}

		}

	}


public:
	// normal functions of the queue push, pop , peek

	void push(const T& topentry) {
		dataarray.push_back(topentry);
		shiftUP(static_cast<int>(dataarray.size()) - 1);
	}

	void pop() {
		// check if the array is empty or not
		if (dataarray.empty()) {
			return;
		}
		dataarray[0] = dataarray.back();
		dataarray.pop_back();

		if (!dataarray.empty()) {
			shiftdown(0);
		}
	}

	// the type of the peek is T to return the top whatever is it 
	T peek() {
		return dataarray[0];
	}

	bool empty() const {
		return dataarray.empty();
	}
};
#endif // PRIORITY_QUEUE_H