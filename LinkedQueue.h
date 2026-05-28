
#pragma once
#include <iostream>
#include "Node.h"
#include <vector>
using namespace std;

template <typename T>
class LinkedQueue
{
private:
	Node<T>* rearPtr;
	Node<T>* frontPtr;

public:
	LinkedQueue()
	{
		rearPtr = nullptr;
		frontPtr = nullptr;
	}

	bool isEmpty() const
	{
		return (frontPtr == nullptr);
	}

	bool enqueue(const T& newEntry)
	{
		Node<T>* newNodePtr = new Node<T>(newEntry);
		if (isEmpty())	               
			frontPtr = rearPtr = newNodePtr;     // The queue is empty
		else
			rearPtr->setNext(newNodePtr); // The queue was not empty
		rearPtr = newNodePtr; 
		return true;
	}

	bool dequeue(T& frontEntry)
	{
		if (isEmpty())
			return false;

		Node<T>* nodeToDeletePtr = frontPtr;

		frontEntry = frontPtr->getItem();
		frontPtr = frontPtr->getNext();

		// Queue is not empty; remove front
		if (nodeToDeletePtr == rearPtr)	 
			rearPtr = nullptr;

		//remeber to Free memory 
		delete nodeToDeletePtr;
		return true;
	}

	bool peek(T& frntEntry)  const
	{
		if (isEmpty())
			return false;

		frntEntry = frontPtr->getItem();
		return true;
	}
	 // always make sure that destractor is presnet 
	~LinkedQueue()
	{
		T temp;
		while (dequeue(temp));
	}

	//copy constructor
	LinkedQueue(const LinkedQueue<T> & LQ)
	{
		Node<T>* NodePtr = LQ.frontPtr;
		if (!NodePtr)   //LQ is empty
		{
			frontPtr = rearPtr = nullptr;
			return;
		}

		//insert the first node
		Node<T>* ptr = new Node<T>(NodePtr->getItem());
		frontPtr = rearPtr = ptr;
		NodePtr = NodePtr->getNext();

		//insert the remained nodes
		while (NodePtr)
		{
			Node<T>* ptr = new Node<T>(NodePtr->getItem());
			rearPtr->setNext(ptr);
			rearPtr = ptr;
			NodePtr = NodePtr->getNext();
		}
	}
};