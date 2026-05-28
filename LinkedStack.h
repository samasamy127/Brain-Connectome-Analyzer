#pragma once
#include<iostream>
#include"Node.h"
using namespace std;

template<typename T>

class LinkedStack
{
private:
	Node<T>* Head;	   
	
public :
	LinkedStack()
	{
		Head = nullptr;
	}	
	
	bool isEmpty() const
	{
		return (Head == nullptr);
	}

	bool push(const T& newEntry)
	{
		Node<T>* newNode = new Node <T>(newEntry);
		newNode->setNext(Head);
		Head = newNode;
		return true;
	}

	bool pop(T& TopEntry)
	{
		if (isEmpty())
			return false;

		Node<T>* nodetodelete = Head;
		TopEntry = Head->getItem();
		Head = Head->getNext();

		// to free memory 
		delete nodetodelete;

		return true;
	}

	T peek() const
	{
		return Head->getItem();
	}

	// Destructor 
	~LinkedStack()
	{
		T temp;
		while (pop(temp)) {}
	}

};