#ifndef _NODE
#define _NODE

template < typename T>
class Node
{
private :
	T data;             // A data of thr node
	Node<T>* next;     // Pointer to next node

public :
	Node()
	{
	next = nullptr;
	}

	// constructor with item
	Node( const T & item) {
		data = item;
		next = nullptr;
	}

	// Constructor with item and next pointer
	Node( const T & item, Node<T>* nextPtr) {
		data = item;
		next = nextPtr;
	}

	// Set item
	void setItem( const T & item) {
		data = item;
	}

	// Set next pointer
	void setNext(Node<T>* nextPtr) {
		next = nextPtr;
	}

	// Get item
	T getItem() const {
		return data;
	}

	// Get next pointer
	Node<T>* getNext() const {
		return next;
	}

};
#endif


