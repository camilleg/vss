/*
class Symtab {
};
*/

const int cchszName = 32;

class Entry {
public:
	float hActor;
	char szName[cchszName];
};

class EntryNote {
public:
	float hActor;
	float hNote;
	char szName[cchszName];
};

class SymtabActor {
private:
	int celements;
	Entry* storage;
	int iIter;
public:
//	SymtabActor(int c);
	SymtabActor();
	~SymtabActor();
	void Reset(void);
	void Dump(void);
	int FAddSz(char* sz, float hactor);
		// fail iff oospace or sz already added.
		// add (hactor,sz) to storage[]. 
	int FDeleteSz(char* sz);
		// fail iff sz !found: "bad actor name"
	int FFoundSz(char* sz);
	float HactorFromSz(char* sz);

	float HFirst(void);
	float HNext(void); // return -1.0 when reach end.
};


class SymtabNote {
private:
	EntryNote* storage;
	int celements;
public:
//	SymtabNote(int c);
	SymtabNote();
	~SymtabNote();
	void Reset(void);
	int FAddSz(char* sz, float hactor, float hnote);
	int FDeleteSz(char* sz);
	int FFoundSz(char* sz);
	float HnoteFromSz(char* sz);
	float HactorFromHnote(float hnote);
};
