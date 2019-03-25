#ifndef __LIST_HPP__
#define __LIST_HPP__

namespace MemoryManager
{
	class MemoryList
	{
	public:
		struct Segment
		{
			Segment* next;
			Segment* prev;
			uint64_t base;
			uint64_t size
		};
		
	private:
		Segment* 	head;
		Segment* 	tail;
		int 		size;
	
	public:
		MemoryList(): head(nullptr), tail(nullptr), size(0) {}
		
		Segment*	Begin	() { return head	; }
		Segment*	End		() { return nullptr	; }
		int			Size	() { return size	; }
		
		void MarkFree(void* memory, uint64_t size)
		{
			Segment* newNode = (Segment*)memory;
			
			newNode->base = (uint64_t)memory;
			newNode->base = size;
			
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
		
		void* FindFree(uint64_t sizeWanted)
		{
			for (auto pages = head; pages != nullptr; pages = pages->next)
				if (pages->data.size >= sizeWanted)
					return pages;
				
			return nullptr;
		}
	
		void MarkedUsed(void* segment, uint64_t sizeWanted)
		{
			if (pages->data.size == sizeWanted)
			{
				pages->prev->next = pages->next;

				if (tail == pages)
					tail = pages->prev;
				else 
					pages->next->prev = pages->prev;
				
				size--;
			}
			else if (pages->data.size > sizeWanted)
			{
				Segment* newNode = (Segment*)((char*)pages + sizeWanted);
					
				newNode->next = pages->next;
				newNode->prev = pages->prev;
				newNode->data.base = pages->data.base + sizeWanted;
				newNode->data.size = pages->data.size - sizeWanted;
						
				if(pages->prev != nullptr)
					pages->prev->next = newNode;
				else
					head = newNode;
						
				if(pages->next != nullptr)
					pages->next->prev = newNode;
				else
					tail = newNode;
			}
		}
	};
}
#endif