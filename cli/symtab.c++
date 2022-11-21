//;; coalesce these two classes.

#include <cstdio>
#include <cstring>
#include "symtab.h"

// storage[] is just a linear unsorted array.
// If it becomes a hotspot, make it a hash table.

#define FEmpty(i) (storage[i].szName[0] == '\0')
#define Empty(i) storage[i].szName[0] = '\0'

float SymtabActor::HFirst(void)
{
	iIter = -1;
	return HNext();
}
float SymtabActor::HNext(void)
{
	iIter++;
	// skip empty storage entries
	while (FEmpty(iIter))
		{
		if (iIter >= celements-1)
			return -1.;
		iIter++;
		}
	float h = storage[iIter].hActor;
	return (h > -1e7 && h < 1e7) ? h : -1;
	// This test is a workaround for an off-by-1 in win32,
	// where the last HNext() returns 1.5e14 or so.
}

// cloned exactly, to produce SymtabNote::FAddSz
SymtabActor::SymtabActor(/*int c*/)
{
	storage = new Entry[celements = 5000/*c*/];
}
SymtabActor::~SymtabActor()
{
	delete [] storage;
}

int SymtabActor::FAddSz(char* sz, float hactor)
{
	if (strlen(sz) >= cchszName)
		fprintf(stderr, "warning: truncating \"%s\"\n", sz);
	int fAdded = 0;
	int iAdded = -1;
	int fFound = 0;
	// Scan entire array.
	for (int i=0; i<celements; i++)
		{
		if (FEmpty(i))
			{
			if (!fAdded)
				{
				fAdded = 1;
				iAdded = i;
				strncpy(storage[i].szName, sz, cchszName-1);
				storage[i].szName[cchszName-1] = '\0';
				storage[i].hActor = hactor;
				}
			}
		else
			if (!strcmp(sz, storage[i].szName))
				{
				if (fFound)
					fprintf(stderr, "FAddSz internal error\n");
				fFound = 1;
				// We shouldn't have added it, after all.
				Empty(iAdded);
				}
		}
	if (!fAdded)
		fprintf(stderr, "SymtabActor of %d elements filled up!\n", celements);
	// fprintf(stderr, "SymtabActor::FAddSz <%s> %d %d\n", sz, fAdded, fFound);;
	return fAdded && !fFound;
}

int SymtabActor::FDeleteSz(char* sz)
{
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			{
			Empty(i);
			return 1;
			}
		}
	return 0;
}

void SymtabActor::Reset(void)
{
	for (int i=0; i<celements; i++)
		Empty(i);
}

void SymtabActor::Dump(void)
{
	fprintf(stderr, "----  SymtabActor  ----\n");
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i))
			fprintf(stderr, "\t[%3d]  %f \"%s\"\n",
				i, storage[i].hActor, storage[i].szName);
		}
	fprintf(stderr, "-----------------------\n");
}

int SymtabActor::FFoundSz(char* sz)
{
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			return 1;
		}
	return 0;
}

float SymtabActor::HactorFromSz(char* sz)
{
//	printf("looking for %s\n", sz);;;;
	for (int i=0; i<celements; i++)
		{
//		if (!FEmpty(i))
//			printf("\t\"%s\"\t\"%s\"\t%d %d\n", 
//				sz, storage[i].szName, !strcmp(sz, storage[i].szName), storage[i].hActor);

		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			return storage[i].hActor;
		}
	return -1.;
}

SymtabNote::SymtabNote(/*int c*/)
{
	storage = new EntryNote[celements = 6000/*c*/];
}
SymtabNote::~SymtabNote()
{
	delete [] storage;
}

int SymtabNote::FAddSz(char* sz, float hactor, float hnote)
{
	if (strlen(sz) >= cchszName)
		fprintf(stderr, "warning: truncating \"%s\"\n", sz);
	int fAdded = 0;
	int iAdded = -1;
	int fFound = 0;
	// Scan entire array.
	for (int i=0; i<celements; i++)
		{
		if (FEmpty(i))
			{
			if (!fAdded)
				{
				fAdded = 1;
				iAdded = i;
				strncpy(storage[i].szName, sz, cchszName-1);
				storage[i].szName[cchszName-1] = '\0';
				storage[i].hActor = hactor;
				storage[i].hNote = hnote;
				}
			}
		else
			if (!strcmp(sz, storage[i].szName))
				{
				if (fFound)
					fprintf(stderr, "FAddSz internal error\n");
				fFound = 1;
				// We shouldn't have added it, after all.
				Empty(iAdded);
				}
		}
	if (!fAdded)
		fprintf(stderr, "SymtabNote of %d elements filled up!\n", celements);
	return fAdded && !fFound;
}

int SymtabNote::FDeleteSz(char* sz)
{
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			{
			Empty(i);
			return 1;
			}
		}
	return 0;
}

void SymtabNote::Reset(void)
{
	for (int i=0; i<celements; i++)
		Empty(i);
}

int SymtabNote::FFoundSz(char* sz)
{
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			return 1;
		}
	return 0;
}

float SymtabNote::HnoteFromSz(char* sz)
{
	for (int i=0; i<celements; i++)
		{
		if (!FEmpty(i) && !strcmp(sz, storage[i].szName))
			return storage[i].hNote;
		}
	return -1.;
}

float SymtabNote::HactorFromHnote(float hnote)
{
	for (int i=0; i<celements; i++)
		{
		if (hnote == storage[i].hNote)
			{
			return storage[i].hActor;
			}
		}
	return -1.;
}

// assert(no duplicate hnote's, hactor's in table);
