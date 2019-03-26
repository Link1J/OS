#ifndef __STACK_HPP__
#define __STACK_HPP__

template<typename T>
class Stack
{
private:
	struct Node
    {
        Node* next;
        Node* prev;
        T data;
    };

    uint64_t size;
    Node* head;
    Node* tail;
	
public:
	Stack() : size(0), head(nullptr), tail(nullptr) {}
	
    ~Stack()
    {
        while (size > 0);
			Pop();
    }
	
	uint64_t Size() const { return size; }
	
	void Push(const T& t)
    {
        Node* node = new Node();
		
		node->next = nullptr;
	
		if (tail == nullptr)
		{
			node->prev = nullptr;
			head = node;
		}
		else
		{
			node->prev = tail;
			tail->next = node;
		}
		
		tail = node;
		size++;
    }
    
    T Pop()
    {
        size--;
		T temp = head->data;
		head = head->next;
		delete head->prev;
		head->prev = nullptr;
		return temp;
    }
};

#endif