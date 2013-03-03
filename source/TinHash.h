// ------------------------------------------------------------------------------------------------
//  The MIT License
//  
//  Copyright (c) 2013 Tim Andersen
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// TinHash.h
// an implementation of the DJB hashing algoritm
// ref:  https://blogs.oracle.com/ali/entry/gnu_hash_elf_sections
// ------------------------------------------------------------------------------------------------

#ifndef __TINHASH_H
#define __TINHASH_H

#include "assert.h"
#include "stdio.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// implemented in TinScript.cpp
unsigned int Hash(const char *s, int length = -1);
unsigned int HashAppend(unsigned int h, const char *string, int length = -1);
const char* UnHash(unsigned int hash);

template <class T>
class CHashTable {

	public:

	class CHashTableEntry {
		public:
			CHashTableEntry(T& _item, unsigned int _hash) {
				item = &_item;
				hash = _hash;
				next = NULL;
			}
			T* item;
			unsigned int hash;
			CHashTableEntry* next;
	};

	// -- constructor / destructor
	CHashTable(int _size = 0) {
		size = _size;
		table = new CHashTableEntry*[size];
		for(unsigned int i = 0; i < size; ++i)
			table[i] = NULL;
		bucketiter = NULL;
        used = 0;
	}

	virtual ~CHashTable() {
		for(unsigned int i = 0; i < size; ++i) {
			CHashTableEntry* entry = table[i];
			while (entry) {
				CHashTableEntry* nextentry = entry->next;
				delete entry;
				entry = nextentry;
			}
		}
        delete table;
	}

	void AddItem(T& _item, unsigned int _hash)
	{
		CHashTableEntry* hte = new CHashTableEntry(_item, _hash);
		unsigned int bucket = _hash % size;
		hte->next = table[bucket];
		table[bucket] = hte;
		bucketiter = NULL;
        ++used;
	}

	T* FindItem(unsigned int _hash) const {
		unsigned int bucket = _hash % size;
		CHashTableEntry* hte = table[bucket];
		while (hte) {
			if (hte->hash == _hash)
				return hte->item;
			hte = hte->next;
		}

		// -- not found
		return NULL;
	}

	void RemoveItem(unsigned int _hash) {
		bucketiter = NULL;
		unsigned int bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry) {
			if (curentry->hash == _hash) {
				*prevptr = curentry->next;
				delete curentry;
                --used;
				return;
			}
			else {
				prevptr = &curentry->next;
				curentry = curentry->next;
			}
		}
	}

	void RemoveItem(T* _item, unsigned int _hash) {
        if(!_item)
            return;
		bucketiter = NULL;
		unsigned int bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry) {
			if (curentry->hash == _hash && curentry->item == _item) {
				*prevptr = curentry->next;
				delete curentry;
                --used;
				return;
			}
			else {
				prevptr = &curentry->next;
				curentry = curentry->next;
			}
		}
	}

	T* FindItemByBucket(unsigned int bucket) const {
		if(bucket >= size)
			return NULL;
		bucketiter = table[bucket];
		if(table[bucket])
			return table[bucket]->item;
		else
			return NULL;
	}

	T* GetNextItemInBucket(unsigned int bucket) const {
		// -- ensure it's the same bucket
		if(bucket >= size || !table[bucket]) {
			bucketiter = NULL;
			return NULL;
		}
		if(!bucketiter)
			bucketiter = table[bucket];
		else
			bucketiter = bucketiter->next;
		return (bucketiter ? bucketiter->item : NULL);
	}

	CHashTableEntry* FindRawEntryByBucket(unsigned int bucket) const {
		if(bucket >= size)
			return NULL;
		bucketiter = table[bucket];
		if(table[bucket])
			return table[bucket];
		else
			return NULL;
	}

	CHashTableEntry* GetNextRawEntryInBucket(unsigned int bucket) const {
		// -- ensure it's the same bucket
		if(bucket >= size || !table[bucket]) {
			bucketiter = NULL;
			return NULL;
		}
		if(!bucketiter)
			bucketiter = table[bucket];
		else
			bucketiter = bucketiter->next;
		return bucketiter;
	}

	unsigned int Size() const {
		return size;
	}

    unsigned int Used() const {
        return used;
    }

    bool IsEmpty() const {
        return (used == 0);
    }

    void RemoveAll() {
		// -- delete all the entries
		for(unsigned int i = 0; i < Size(); ++i) {
			CHashTableEntry* entry = FindRawEntryByBucket(i);
			while (entry != NULL) {
				unsigned int hash = entry->hash;
				RemoveItem(hash);
				entry = FindRawEntryByBucket(i);
			}
		}
        used = 0;
    }

    // -- This method doesn't just remove all entries from the
    // -- hash table, but it deletes the actual items stored
    void DestroyAll() {
        for(unsigned int i = 0; i < Size(); ++i) {
			CHashTableEntry* entry = FindRawEntryByBucket(i);
		    T* object = FindItemByBucket(i);
		    while(object != NULL) {
                RemoveItem(entry->hash);
                delete object;
			    entry = FindRawEntryByBucket(i);
		        object = FindItemByBucket(i);
		    }
        }
        used = 0;
    }

	private:
		CHashTableEntry** table;
		mutable CHashTableEntry* bucketiter;
		unsigned int size;
        unsigned int used;
};

}  // TinScript

#endif // __TINHASH

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------

