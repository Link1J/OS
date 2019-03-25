#ifndef __LIST_HPP__
#define __LIST_HPP__

namespace MemoryManager
{	
	

	template<typename T>
	class List
	{
	private:
		
		
	public:
		List(): head(nullptr), tail(nullptr) {}
		
		Node<T>*	begin	() { return head	; }
		Node<T>*	end		() { return nullptr	; }
		Node<T>*	rbegin	() { return tail	; }
		Node<T>*	rend	() { return nullptr	; }
		int			Size	() { return size	; }
		
		void AddNode(Node<T>* newNode)
		{
			newNode->next = nullptr;
			newNode->prev = nullptr;
			
			if (head == nullptr)
			{
				head = newNode;
				tail = newNode;
			}
			else
			{
				tail->next = newNode;
				newNode->prev = tail;
				tail = newNode;
			}
			
			size++;
		}
	
		void UpdateHead(Node<T>* newNode)
		{
			head = newNode;
		}
		
		void UpdateTail(Node<T>* newNode)
		{
			tail = newNode;
		}
		
		void RemoveNode(Node<T>* newNode)
		{
			newNode->prev->next = newNode->next;
			
			if (tail == newNode)
				tail = newNode->prev;
			else 
				newNode->next->prev = newNode->prev;
			
			size--;
		}
	};
}
#endif