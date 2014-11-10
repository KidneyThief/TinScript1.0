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

// ====================================================================================================================
// TinHash.h
// an implementation of the DJB hashing algoritm
// ref:  https://blogs.oracle.com/ali/entry/gnu_hash_elf_sections
// ====================================================================================================================

#ifndef __TINHASH_H
#define __TINHASH_H

// -- includes

#include "assert.h"
#include "stdio.h"

#include "integration.h"

// == namespace Tinscript =============================================================================================

namespace TinScript
{

// ====================================================================================================================
// -- implemented in TinScript.cpp
uint32 Hash(const char *s, int32 length = -1, bool add_to_table = true);
uint32 HashAppend(uint32 h, const char *string, int32 length = -1);
const char* UnHash(uint32 hash);

// ====================================================================================================================
// class CHashTable:  This class is used for *all* TinScript hash tables, of any type.
// Regardless of the content type being stored, this hash table only allows pointers (to that type).
// This will allow hash table entries to be a fixed size, and can be pooled amongst all tables.
// ====================================================================================================================
template <class T>
class CHashTable
{
	public:

    // ====================================================================================================================
    // class CHashTableEntry:  As mentioned, all hash tables store pointers only, using this common entry class.
    // ====================================================================================================================
	class CHashTableEntry
    {
		public:
			CHashTableEntry(T& _item, uint32 _hash)
            {
				item = &_item;
				hash = _hash;
				nextbucket = NULL;
                next = NULL;
                prev = NULL;

                index = -1;
                index_next = NULL;
			}

			T* item;
			uint32 hash;
			CHashTableEntry* nextbucket;
			CHashTableEntry* next;
			CHashTableEntry* prev;

            int32 index;
			CHashTableEntry* index_next;
	    };

	// -- constructor / destructor
	CHashTable(int32 _size = 0)
    {
		size = _size;
		table = new CHashTableEntry*[size];
		index_table = new CHashTableEntry*[size];
		for (int32 i = 0; i < size; ++i)
        {
			table[i] = NULL;
			index_table[i] = NULL;
        }

		bucketiter = NULL;
        iter = NULL;
        used = 0;
        head = NULL;
        tail = NULL;
        iter_was_removed = false;
	}

	virtual ~CHashTable()
    {
		for (int32 i = 0; i < size; ++i)
        {
			CHashTableEntry* entry = table[i];
			while (entry)
            {
				CHashTableEntry* nextentry = entry->nextbucket;
				delete entry;
				entry = nextentry;
			}
		}
        delete table;
        delete index_table;
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
        if (!head)
        {
            // -- add to the index table
            hte->index = 0;
            index_table[0] = hte;

            head = hte;
            tail = hte;
        }
        else
        {
            // -- add to the index table (note:  used has already been incremented)
            hte->index = used - 1;
            hte->index_next = index_table[hte->index % size];
            index_table[hte->index % size] = hte;

            tail->next = hte;
            tail = hte;
        }
	}

	void InsertItem(T& _item, uint32 _hash, int32 _index)
	{
        // -- if the index is anywhere past the end, simply add
        if (_index >= used)
        {
            AddItem(_item, _hash);
            return;
        }

        // -- ensure the index is valid
        if (_index < 0)
            _index = 0;

        // -- create the entry, add it to the table as per the hash, and clear the iterators
		CHashTableEntry* hte = new CHashTableEntry(_item, _hash);
		int32 bucket = _hash % size;
		hte->nextbucket = table[bucket];
		table[bucket] = hte;
		bucketiter = NULL;
        iter = NULL;
        iter_was_removed = false;

        // -- we need to insert it into the double-linked list before the entry currently at the given index
        CHashTableEntry* prev_hte = NULL;
        CHashTableEntry* cur_entry = FindRawEntryByIndex(_index, prev_hte);
        assert(cur_entry);

        // -- insert before
        hte->prev = cur_entry->prev;
        cur_entry->prev = hte;
        hte->next = cur_entry;
        if (hte->prev)
            hte->prev->next = hte;
        else
            head = hte;

        // -- insert it into the index table - note, we need to bump up the indices of all entries after this
        // -- also note used has not yet been incremented
        // -- update all entries after cur_entry, by decrimenting and updating the index table
        for (int32 bump_index = used - 1; bump_index >= _index; --bump_index)
        {
            CHashTableEntry* prev_hte = NULL;
            CHashTableEntry* bump_hte = FindRawEntryByIndex(bump_index, prev_hte);

            // -- remove the hte from the linked list in the index_table bucket
            assert(bump_hte != 0);
            if (prev_hte)
                prev_hte->index_next = bump_hte->index_next;
            else
                index_table[bump_index % size] = bump_hte->index_next;

            // -- increment the index and add it to the previous index bucket
            ++bump_hte->index;
            bump_hte->index_next = index_table[bump_hte->index % size];
            index_table[bump_hte->index % size] = bump_hte;
        }

        // -- now add ourself into the index table
        hte->index = _index;
        hte->index_next = index_table[_index % size];
        index_table[_index % size] = hte;

        // -- finally, increment the used count
        ++used;
	}

	T* FindItem(uint32 _hash) const
    {
		int32 bucket = _hash % size;
		CHashTableEntry* hte = table[bucket];
		while (hte)
        {
			if (hte->hash == _hash)
				return hte->item;

			hte = hte->nextbucket;
		}

		// -- not found
		return (NULL);
	}

    T* FindItemByIndex(int32 _index) const
    {
        if (_index < 0 || _index >= used)
            return (NULL);

        CHashTableEntry* hte = index_table[_index % size];
        while (hte && hte->index != _index)
            hte = hte->index_next;

        return (hte ? hte->item : NULL);
    }

    void RemoveItemByIndex(int32 _index)
    {
        CHashTableEntry* hte = FindRawEntryByIndex(_index);
        if (hte)
        {
            RemoveItem(hte->item, hte->hash);
        }
    }

    CHashTableEntry* FindRawEntryByIndex(int32 _index, CHashTableEntry*& prev_entry) const
    {
        prev_entry = NULL;
        if (_index < 0 || _index >= used)
            return (NULL);

        CHashTableEntry* hte = index_table[_index % size];
        while (hte && hte->index != _index)
        {
            prev_entry = hte;
            hte = hte->index_next;
        }

        return (hte);
    }

    void RemoveRawEntryFromIndexTable(CHashTableEntry* cur_entry)
    {
        // -- remove the current entry from the index table (first)
        CHashTableEntry* prev_hte = NULL;
        CHashTableEntry* hte = FindRawEntryByIndex(cur_entry->index, prev_hte);

        assert(hte == cur_entry);
        if (prev_hte)
            prev_hte->index_next = hte->index_next;
        else
            index_table[cur_entry->index % size] = hte->index_next;

        // -- update all entries after cur_entry, by decrimenting and updating the index table
        for (int _index = cur_entry->index + 1; _index < used; ++_index)
        {
            CHashTableEntry* prev_hte = NULL;
            CHashTableEntry* hte = FindRawEntryByIndex(_index, prev_hte);

            // -- remove the hte from the linked list in the index_table bucket
            assert(hte != 0);
            if (prev_hte)
                prev_hte->index_next = hte->index_next;
            else
                index_table[_index % size] = hte->index_next;

            // -- decriment the index add it to the previous index bucket
            --hte->index;
            hte->index_next = index_table[hte->index % size];
            index_table[hte->index % size] = hte;
        }
    }

	void RemoveItem(uint32 _hash)
    {
		uint32 bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry)
        {
			if (curentry->hash == _hash)
            {
				*prevptr = curentry->nextbucket;

                // -- update the iterators
                // $$$TZA This is reliable if'f the table is being iterated by a single
                // -- loop - not attempting to share iterators.
                if (curentry == bucketiter)
                {
                    bucketiter = curentry->next;
                    iter_was_removed = true;
                }

                if (curentry == iter)
                {
                    iter = curentry->next;
                    iter_was_removed = true;
                }

                // -- remove from the double-linked list
                if (curentry->prev)
                    curentry->prev->next = curentry->next;
                if (curentry->next)
                    curentry->next->prev = curentry->prev;
                if (curentry == head)
                    head = curentry->next;
                if (curentry == tail)
                    tail = curentry->prev;

                // -- remove the entry from the index table
                RemoveRawEntryFromIndexTable(curentry);

                // -- delete the entry, and decriment the count
				delete curentry;
                --used;
				return;
			}
			else
            {
				prevptr = &curentry->nextbucket;
				curentry = curentry->nextbucket;
			}
		}
	}

	void RemoveItem(T* _item, uint32 _hash)
    {
        if (!_item)
            return;

		int32 bucket = _hash % size;
		CHashTableEntry** prevptr = &table[bucket];
		CHashTableEntry* curentry = table[bucket];
		while (curentry)
        {
			if (curentry->hash == _hash && curentry->item == _item)
            {
				*prevptr = curentry->nextbucket;

                // -- update the iterators
                // $$$TZA This is reliable if'f the table is being iterated by a single
                // -- loop - not attempting to share iterators.  Should convert to CTable<>
                if (curentry == bucketiter)
                {
                    bucketiter = curentry->next;
                    iter_was_removed = true;
                }

                if (curentry == iter)
                {
                    iter = curentry->next;
                    iter_was_removed = true;
                }

                // -- remove from the double-linked list
                if (curentry->prev)
                    curentry->prev->next = curentry->next;
                if (curentry->next)
                    curentry->next->prev = curentry->prev;
                if (curentry == head)
                    head = curentry->next;
                if (curentry == tail)
                    tail = curentry->prev;

                // -- remove the entry from the index table
                RemoveRawEntryFromIndexTable(curentry);

				delete curentry;
                --used;
				return;
			}
			else
            {
				prevptr = &curentry->nextbucket;
				curentry = curentry->nextbucket;
			}
		}
	}

    T* First(uint32* out_hash = NULL) const
    {
        iter = head;
        iter_was_removed = false;
        if (head)
        {
            // -- return the hash value, if requested
            if (out_hash)
                *out_hash = head->hash;
            return (head->item);
        }
        else
        {
            if (out_hash)
                *out_hash = 0;
            return (NULL);
        }
    }

    T* Next(uint32* out_hash = NULL) const
    {
        if (iter && !iter_was_removed)
            iter = iter->next;

        iter_was_removed = false;
        if (iter)
        {
            // -- return the hash value, if requested
            if (out_hash)
                *out_hash = iter->hash;
            return (iter->item);
        }
        else
        {
            if (out_hash)
                *out_hash = 0;
            return (NULL);
        }
    }

    T* Last(uint32* out_hash = NULL) const
    {
        iter = tail;
        iter_was_removed = false;
        if (tail)
        {
            // -- return the hash value, if requested
            if (out_hash)
                *out_hash = tail->hash;
            return (tail->item);
        }
        else
        {
            if (out_hash)
                *out_hash = 0;
            return (NULL);
        }
    }

	T* FindItemByBucket(int32 bucket) const
    {
		if (bucket >= size)
			return (NULL);
		bucketiter = table[bucket];
        iter_was_removed = false;
		if (table[bucket])
			return table[bucket]->item;
		else
			return (NULL);
	}

	T* GetNextItemInBucket(int32 bucket) const
    {
		// -- ensure it's the same bucket
		if (bucket >= size || !table[bucket])
        {
			bucketiter = NULL;
            iter_was_removed = false;
			return (NULL);
		}
		if (!bucketiter)
			bucketiter = table[bucket];
		else if (!iter_was_removed)
			bucketiter = bucketiter->nextbucket;
        iter_was_removed = false;

		return (bucketiter ? bucketiter->item : NULL);
	}

	CHashTableEntry* FindRawEntryByBucket(int32 bucket) const
    {
		if (bucket >= size)
			return (NULL);

		bucketiter = table[bucket];
        iter_was_removed = false;
		if (table[bucket])
			return table[bucket];
		else
			return (NULL);
	}

	CHashTableEntry* GetNextRawEntryInBucket(int32 bucket) const
    {
		// -- ensure it's the same bucket
		if (bucket >= size || !table[bucket])
        {
			bucketiter = NULL;
			return (NULL);
		}

		if (!bucketiter)
			bucketiter = table[bucket];
		else if (!iter_was_removed)
			bucketiter = bucketiter->nextbucket;

        iter_was_removed = false;
		return bucketiter;
	}

	int32 Size() const
    {
		return size;
	}

    int32 Used() const
    {
        return used;
    }

    bool8 IsEmpty() const
    {
        return (used == 0);
    }

    void RemoveAll()
    {
        // -- reset any iterators
        iter = NULL;
        bucketiter = NULL;
        iter_was_removed = false;

		// -- delete all the entries
		for (int32 i = 0; i < Size(); ++i)
        {
			CHashTableEntry* entry = FindRawEntryByBucket(i);
			while (entry != NULL)
            {
				int32 hash = entry->hash;
				RemoveItem(entry->item, hash);
				entry = FindRawEntryByBucket(i);
			}
		}
        used = 0;
    }

    // -- This method doesn't just remove all entries from the
    // -- hash table, but it deletes the actual items stored
    void DestroyAll()
    {
        // -- reset any iterators
        iter = NULL;
        bucketiter = NULL;
        iter_was_removed = false;

        for (int32 i = 0; i < Size(); ++i)
        {
			CHashTableEntry* entry = FindRawEntryByBucket(i);
		    T* object = FindItemByBucket(i);
		    while(object != NULL)
            {
                RemoveItem(entry->item, entry->hash);
                delete object;
			    entry = FindRawEntryByBucket(i);
		        object = FindItemByBucket(i);
		    }
        }
        used = 0;
    }

	private:
		CHashTableEntry** table;
		CHashTableEntry** index_table;
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

// ====================================================================================================================
// EOF
// ====================================================================================================================
