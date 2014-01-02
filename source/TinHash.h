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

#include "integration.h"
namespace TinScript {

// ------------------------------------------------------------------------------------------------
// implemented in TinScript.cpp
uint32 Hash(const char *s, int32 length = -1);
uint32 HashAppend(uint32 h, const char *string, int32 length = -1);
const char* UnHash(uint32 hash);

template <class T>
class CHashTable {

	public:

	class CHashTableEntry {
		public:
			CHashTableEntry(T& _item, uint32 _hash) {
				item = &_item;
				hash = _hash;
				nextbucket = NULL;
                next = NULL;
                prev = NULL;
			}
			T* item;
			uint32 hash;
			CHashTableEntry* nextbucket;
			CHashTableEntry* next;
			CHashTableEntry* prev;
	};

	// -- constructor / destructor
	CHashTable(int32 _size = 0) {
		size = _size;
		table = new CHashTableEntry*[size];
		for(int32 i = 0; i < size; ++i)
			table[i] = NULL;
		bucketiter = NULL;
        iter = NULL;
        used = 0;
        head = NULL;
        tail = NULL;
        iter_was_removed = false;
	}

	virtual ~CHashTable() {
		for(int32 i = 0; i < size; ++i) {
			CHashTableEntry* entry = table[i];
			while (entry) {
				CHashTableEntry* nextentry = entry->nextbucket;
				delete entry;
				entry = nextentry;
			}
		}
        delete table;
	}

	void AddItem(T& _item, uint32 _hash)
	{
		CHashTableEntry* hte = new CHashTableEntry(_item, _hash);
		int32 bucket = _hash % size;
		hte->nextbucket = table[bucket];
		table[bucket] = hte;
		bucketiter = NULL;
        iter = NULL;
        iter_was_removed = false;
        ++used;

        // -- add to the double-linked list
        hte->next = NULL;
        hte->prev = tail;
        if(!head) {
            head = hte;
            tail = hte;
        }
        else {
            tail->next = hte;
            tail = hte;
        }
	}

	T* FindItem(uint32 _hash) const {
		int32 bucket = _hash % size;
		CHashTableEntry* hte = table[bucket];
		while (hte) {
			if (hte->hash == _hash)
				return hte->item;
			hte = hte->nextbucket;
		}

		// -- not found
		return NULL;
	}

	void RemoveItem(uint32 _hash) {
		uint32 bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry) {
			if (curentry->hash == _hash) {
				*prevptr = curentry->nextbucket;

                // -- update the iterators
                // $$$TZA This is reliable if'f the table is being iterated by a single
                // -- loop - not attempting to share iterators.
                if(curentry == bucketiter) {
                    bucketiter = curentry->next;
                    iter_was_removed = true;
                }
                if(curentry == iter) {
                    iter = curentry->next;
                    iter_was_removed = true;
                }

                // -- remove from the double-linked list
                if(curentry->prev)
                    curentry->prev->next = curentry->next;
                if(curentry->next)
                    curentry->next->prev = curentry->prev;
                if(curentry == head)
                    head = curentry->next;
                if(curentry == tail)
                    tail = curentry->prev;

				delete curentry;
                --used;
				return;
			}
			else {
				prevptr = &curentry->nextbucket;
				curentry = curentry->nextbucket;
			}
		}
	}

	void RemoveItem(T* _item, uint32 _hash) {
        if(!_item)
            return;

		int32 bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry) {
			if (curentry->hash == _hash && curentry->item == _item) {
				*prevptr = curentry->nextbucket;

                // -- update the iterators
                // $$$TZA This is reliable if'f the table is being iterated by a single
                // -- loop - not attempting to share iterators.  Should convert to CTable<>
                if(curentry == bucketiter) {
                    bucketiter = curentry->next;
                    iter_was_removed = true;
                }
                if(curentry == iter) {
                    iter = curentry->next;
                    iter_was_removed = true;
                }

                // -- remove from the double-linked list
                if(curentry->prev)
                    curentry->prev->next = curentry->next;
                if(curentry->next)
                    curentry->next->prev = curentry->prev;
                if(curentry == head)
                    head = curentry->next;
                if(curentry == tail)
                    tail = curentry->prev;

				delete curentry;
                --used;
				return;
			}
			else {
				prevptr = &curentry->nextbucket;
				curentry = curentry->nextbucket;
			}
		}
	}

    T* First() const {
        iter = head;
        iter_was_removed = false;
        if(head)
            return (head->item);
        else
            return (NULL);
    }

    T* Next() const {
        if(iter && !iter_was_removed) {
            iter = iter->next;
        }
        iter_was_removed = false;
        if(iter)
            return (iter->item);
        else
            return NULL;
    }

    T* Last() const {
        return (tail);
    }

	T* FindItemByBucket(int32 bucket) const {
		if(bucket >= size)
			return NULL;
		bucketiter = table[bucket];
        iter_was_removed = false;
		if(table[bucket])
			return table[bucket]->item;
		else
			return NULL;
	}

	T* GetNextItemInBucket(int32 bucket) const {
		// -- ensure it's the same bucket
		if(bucket >= size || !table[bucket]) {
			bucketiter = NULL;
            iter_was_removed = false;
			return NULL;
		}
		if(!bucketiter) {
			bucketiter = table[bucket];
        }
		else if (!iter_was_removed) {
			bucketiter = bucketiter->nextbucket;
        }
        iter_was_removed = false;
		return (bucketiter ? bucketiter->item : NULL);
	}

	CHashTableEntry* FindRawEntryByBucket(int32 bucket) const {
		if(bucket >= size)
			return NULL;
		bucketiter = table[bucket];
        iter_was_removed = false;
		if(table[bucket])
			return table[bucket];
		else
			return NULL;
	}

	CHashTableEntry* GetNextRawEntryInBucket(int32 bucket) const {
		// -- ensure it's the same bucket
		if(bucket >= size || !table[bucket]) {
			bucketiter = NULL;
			return NULL;
		}
		if(!bucketiter) {
			bucketiter = table[bucket];
        }
		else if(!iter_was_removed)
			bucketiter = bucketiter->nextbucket;
        iter_was_removed = false;
		return bucketiter;
	}

	int32 Size() const {
		return size;
	}

    int32 Used() const {
        return used;
    }

    bool8 IsEmpty() const {
        return (used == 0);
    }

    void RemoveAll() {
        // -- reset any iterators
        iter = NULL;
        bucketiter = NULL;
        iter_was_removed = false;

		// -- delete all the entries
		for(int32 i = 0; i < Size(); ++i) {
			CHashTableEntry* entry = FindRawEntryByBucket(i);
			while (entry != NULL) {
				int32 hash = entry->hash;
				RemoveItem(hash);
				entry = FindRawEntryByBucket(i);
			}
		}
        used = 0;
    }

    // -- This method doesn't just remove all entries from the
    // -- hash table, but it deletes the actual items stored
    void DestroyAll() {
        // -- reset any iterators
        iter = NULL;
        bucketiter = NULL;
        iter_was_removed = false;

        for(int32 i = 0; i < Size(); ++i) {
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
		int32 size;
        int32 used;

		mutable CHashTableEntry* iter;
        CHashTableEntry* head;
        CHashTableEntry* tail;

        mutable bool8 iter_was_removed;
 };

}  // TinScript

#endif // __TINHASH

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------

