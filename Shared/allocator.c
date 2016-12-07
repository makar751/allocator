#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h> 

typedef struct memoryFreeListTmpl{
	void *ptr;
	struct memoryFreeListTmpl *next;
} memoryFreeList;

typedef struct memoryUsedListTmpl{
	void *ptr;
	size_t size;
	struct memoryUsedListTmpl *next;
} memoryUsedList;

typedef struct memoryFreeSizeListTmpl{
	memoryFreeList *first;
	size_t size;
	struct memoryFreeSizeListTmpl *next;
} memoryFreeSizeList;

typedef struct memoryPageTmpl{
	memoryUsedList *first;
	void *page;
	size_t freeSpace;
	struct memoryPageTmpl *next;
} memoryPage;

memoryUsedList *firstUsed, *notUsedElmOfUsed;
memoryFreeList *notUsedElmOfFree;
memoryFreeSizeList *firstFree;
memoryPage *firstPage;
int page_size;

void *getSpace(size_t size, memoryPage *page)
{
	if (page->first == NULL)
	{
		page->first = page->page; 
		page->freeSpace = page->freeSpace - size - sizeof(memoryUsedList);
		page->first->ptr = page->page + sizeof(memoryUsedList);
		page->first->size = size;
		page->first->next = NULL;
		return page->first->ptr;
	}
	else
	{
		memoryUsedList *q = page->first;
		while (q->next != NULL)
			q = q->next;
		q->next = q->ptr + q->size;
		q->next->ptr = q->ptr + q->size + sizeof(memoryUsedList);
		q = q->next;
		q->size = size;
		q->next = NULL;
		page->freeSpace = page->freeSpace - size - sizeof(memoryUsedList);
		return q->ptr;
	}
}

void *getPage(size_t size)
{
	if (firstPage == NULL)
	{
		page_size = sysconf(_SC_PAGESIZE);
		firstPage = mmap(NULL, 1, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); //очевидный костыль
		firstPage->page = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); 
		firstPage->first = NULL;
		firstPage->freeSpace = page_size;
		firstPage->next = getSpace(sizeof(memoryPage), firstPage); // заготовка под следующюю страницу
		firstPage->next->freeSpace = 0;
		return firstPage;
	}
	memoryPage *q = firstPage;
	while ((q != NULL) && (q->freeSpace < (size + sizeof(memoryUsedList))))
		q = q->next;
	if (q == NULL)
	{
		q = firstPage;
		while (q->next != NULL)
			q = q->next;
		q->page = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); 
		q->freeSpace = page_size;
		q->first = NULL;
		q->next = getSpace(sizeof(memoryPage),q); // заготовка под следующюю страницу
		q->next->freeSpace = 0;
		return q;
	}
	if (q->freeSpace == 0)
	{
		q = firstPage;
		while (q->next != NULL)
			q = q->next;
		q->page = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); 
		q->freeSpace = page_size;
		q->first = NULL;
		q->next = getSpace(sizeof(memoryPage),q); // заготовка под следующюю страницу
		q->next->freeSpace = -1;
		return q;
	}
	else
		return q;
}

void *getPtr(size_t size)
{
	return getSpace(size, getPage(size));
}

void *getmemory(size_t size)
{
    return getPtr(size);
}

memoryFreeSizeList *createElmSL()
{
	return getPtr(sizeof(memoryFreeSizeList));
}

memoryFreeList *createElmL()
{
	if (notUsedElmOfFree == NULL)
		return getPtr(sizeof(memoryFreeList));
	else
	{
		memoryFreeList *q1 = notUsedElmOfFree;
		notUsedElmOfFree = notUsedElmOfFree->next;
		q1->next = NULL;
		return q1;
	}
}

memoryUsedList *createUsedElmL()
{
	if (notUsedElmOfUsed == NULL)
		return getPtr(sizeof(memoryUsedList));
	else
	{
		memoryUsedList *q1 = notUsedElmOfUsed;
		notUsedElmOfUsed = notUsedElmOfUsed->next;
		q1->next = NULL;
		return q1;
	}
}

int getSize(size_t a)
{
	int i = 2;
	while (a > i)
			i = i * 2;
	return i;
}

memoryFreeSizeList *searchELmSL(size_t size)
{
	memoryFreeSizeList *q1 = firstFree;
	if (q1 == NULL)
	{
		int i = getSize(size);
		q1 =	createElmSL();
		q1->size = i;
		q1->next = NULL;
		firstFree = q1;
		return q1;
	}
	else
	{
		memoryFreeSizeList *q2;
		if (q1->next == NULL)
		{
			q2 = firstFree;
			int i = getSize(size);
			if (q1->size == i)
				return q1;
			else
			{
				q2 = createElmSL();
				if (q1->size < i)
				{
					q1->next = q2;
					q2->next = NULL;
					q2->size = i;
				}
				else
				{
					q2->next = q1;
					q2->size = i;
					firstFree->next = q2;
				}
			}
		}
		else
		{
			int i = getSize(size);
			q2 = firstFree->next;
			while ((q2 != NULL) && (q2->size < i))
			{
				q1 = q1->next;
				q2 = q2->next;
			}
			if (q2 == NULL)
			{
				if (q1->size != i)
					q1->next=createElmSL();
				q2 = q1;
			}
			else
			{
				if (q2->size == i)
					q2 = q1;
				else 
				{
					if (q1 != firstFree)
					{
						q1->next = createElmSL();
						q1->size = size;
						q1->next->next = q2;
						q2 = q1;	
					}
					else
					{
						memoryFreeSizeList *q3;
						q3 = createElmSL();
						q3->size = size;
						q3->next = firstFree;
						firstFree = q3;
						q2 = q3;
					}
				}
			}
		}
		return q2;
	}
}

memoryFreeList *searchELmL(memoryFreeSizeList *list)
{
	memoryFreeList *ptr = list->first;
	if (ptr==NULL)
	{
		ptr = createElmL();
		ptr->next = NULL;
		return ptr;
	}
	else
		list->first = ptr->next;
	return ptr;

}

void* toUsed(memoryFreeList *ptr, size_t size)
{
	if (ptr->ptr == NULL)
	{
		ptr->ptr = getmemory(size);
		printf ("No free memory. Get memory from system: %p \n",ptr->ptr);
	}
	if (firstUsed == NULL)
	{
		firstUsed = createUsedElmL();
		firstUsed->next = NULL;
	}
	else
	{
		memoryUsedList *q = createUsedElmL();
		q->next = firstUsed;
		firstUsed = q;
	}
	printf ("Found free memory at addr: %p \n",ptr->ptr);
	firstUsed->size = size;
	firstUsed->ptr = ptr->ptr;
	ptr->ptr = NULL;
	if (notUsedElmOfFree == NULL)
		notUsedElmOfFree = ptr;
	else
	{
		ptr->next = notUsedElmOfFree;
		notUsedElmOfFree = ptr;
	}
	return firstUsed->ptr;
}

void deleteFromUsed(memoryUsedList *tof)
{
	memoryUsedList *q = firstUsed;
	if (firstUsed == tof)
		firstUsed = firstUsed->next;
	else
	{
		while ((q->next != NULL) && (q->next != tof))
			q = q->next;
		q = q->next->next;
	}
}

int toFree(void *ptr)
{
	memoryUsedList *q = firstUsed;
	printf ("Need to free addr: %p \n",ptr);
	while ((q != NULL) && (q->ptr != ptr))
		q = q->next;
	if (q == NULL)
		return 1;
	else
	{
		memoryFreeSizeList *sl = searchELmSL(q->size);
		memoryFreeList *q1 = createElmL();
		q1->ptr = ptr;
		q1->next = sl->first;
		sl->first = q1;
		q->ptr = NULL;
		if (notUsedElmOfUsed == NULL)
			notUsedElmOfUsed = q;
		else
		{
			q->next = notUsedElmOfUsed;
			notUsedElmOfUsed = q;
		}
		deleteFromUsed(q);
		return 0;
	}	
}

void *malloc(size_t size)
{
	if (size != 0)
	{
		memoryFreeList *ptr;
		memoryFreeSizeList *q;
		q = searchELmSL(size);
		ptr = searchELmL(q);
		void *result = toUsed(ptr, q->size);
		printf ("Malloc addr: %p \n",result);		
		return result;
	}
	else
		return NULL;
}

void free(void *in)
{
	if (in != NULL)
	{
		void *ptr = (void*)in;
		if (toFree(ptr) != 0)
			printf ("Can't free memory on addr: %p \n",ptr);
		else
			printf ("Fried addr: %p \n",ptr);
	}
	in = NULL;
	

}

void *calloc(size_t nmemb, size_t size)
{
	if ((nmemb == 0) || (size == 0))
		return NULL;
	void* ptr = malloc(nmemb * size);
	memset(ptr, 0, size*nmemb);
	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return malloc(size);
	if(size == 0)
	{
		free(ptr);
		return NULL;
	}
	else
	{
		memoryUsedList *q = firstUsed;
		while ((q != NULL) && (q->ptr != ptr))
			q = q->next;
		if (q == NULL)
			return NULL;
		else
		{
			void *res = malloc(size);
			memcpy(res, ptr, q->size);
			free(ptr);
			return res;
		}	
	}
}