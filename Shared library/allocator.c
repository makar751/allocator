#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

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
	size_t freeSpace;
	struct memoryPageTmpl *next;
} memoryPage;

memoryUsedList *firstUsed, *notUsedElmOfUsed;
memoryFreeList *notUsedElmOfFree;
memoryFreeSizeList *firstFree;
memoryPage *firstPage;


/* void *getPage(size_t size)
{
	if (firstSys == NULL)
	{
		firstSys = mmap(NULL, 100, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		firstSys->next == NULL;
		firstSys->ptr = mmap(NULL, sizeof(memoryPage), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	}
} */

void *getmemory(size_t size)
{
    return mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

memoryFreeSizeList *createElmSL()
{
	return mmap(NULL, sizeof(memoryFreeSizeList), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

memoryFreeList *createElmL()
{
	if (notUsedElmOfFree == NULL)
		return mmap(NULL, sizeof(memoryFreeList), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
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
		return mmap(NULL, sizeof(memoryUsedList), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
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

/*void deleteFromFree(memoryFreeList *tof, size_t size)
{
	memoryFreeSizeList *q = searchELmSL(size);
	memoryFreeList *q1 = q->first;
	if (q1 == tof)
		q->first = q->first->next;
	else
	{
		while ((q1->next != NULL) && (q1->next != tof))
			q1 = q1->next;
		q1 = q1->next->next;
	}
}*/

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
	//deleteFromFree(ptr, size);
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

void *mymalloc(size_t size)
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

void myfree(void *in)
{
	void *test = mmap(NULL, 10000000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	printf ("Page addr: %p \n",test);
	if (in != NULL)
	{
		void *ptr = (void*)in;//- 0xFFFF800000000000;
		if (toFree(ptr) != 0)
			printf ("Can't free memory on addr: %p \n",ptr);
		else
			printf ("Fried addr: %p \n",ptr);
	}
	in = NULL;
	

}

void *mycalloc(size_t nmemb, size_t size)
{
	if ((nmemb == 0) || (size == 0))
		return NULL;
	void* ptr = mymalloc(nmemb * size);
	memset(ptr, 0, size*nmemb);
	return ptr;
}

void *myrealloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return mymalloc(size);
	if(size == 0)
	{
		myfree(ptr);
		return NULL;
	}
	else
	{

	}
}