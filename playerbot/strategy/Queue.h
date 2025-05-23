#include "ActionBasket.h"

#pragma once
namespace ai
{
class Queue
{
public:
    Queue(void) {}
public:
    ~Queue(void) {}
public:
	void Push(ActionBasket *action);
	ActionNode* Pop(ActionBasket* action = nullptr);
    ActionBasket* Peek() const;
	int Size();
	void RemoveExpired();
private:
	std::list<ActionBasket*> actions;
};
}
