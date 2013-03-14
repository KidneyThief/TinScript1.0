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
// TinStringTable.h
// ------------------------------------------------------------------------------------------------

#ifndef __TINSTRINGTABLE_H
#define __TINSTRINGTABLE_H

#include "string.h"

namespace TinScript {

// CStringTable is a singleton, used to create a dictionary of hashed strings
class CStringTable {
    public:
        CStringTable(CScriptContext* owner, uint32 _size) {
            mContextOwner = owner;

            assert(_size > 0);
            mSize = _size;
            mBuffer = TinAllocArray(ALLOC_StringTable, char, mSize);
            mBufptr = mBuffer;

            mStringDictionary = TinAlloc(ALLOC_StringTable, CHashTable<const char>,
                                        kStringTableDictionarySize);
        }

        virtual ~CStringTable() {
            TinFree(mStringDictionary);
            TinFree(mBuffer);
        }

        CScriptContext* GetScriptContext() {
            return (mContextOwner);
        }

        const char* AddString(const char* s, int length = -1, uint32 hash = 0) {
            // -- sanity check
            if(!s)
                return "";

            if(hash == 0) {
                hash = Hash(s, length);
            }

            // -- see if the string is already in the dictionary
            const char* exists = FindString(hash);
            if(!exists)
            {
                if(length < 0)
                    length = (int32)strlen(s);

                // -- space left
                int32 remaining = int32(mSize - (kPointerToUInt32(mBufptr) -
                                                 kPointerToUInt32(mBuffer)));
                if(remaining < length + 1) {
                    ScriptAssert_(mContextOwner, 0,
                                  "<internal>", -1, "Error - StringTable of size %d is full!\n",
                                  mSize);
                    return NULL;
                }
                const char* stringptr = mBufptr;
                SafeStrcpy(mBufptr, s, length + 1);
                mBufptr += length + 1;

                // -- add the entry to the dictionary
                mStringDictionary->AddItem(*stringptr, hash);

                return stringptr;
            }

            // -- else check for a collision
            else
            {
                if(length < 0)
                    length = (int32)strlen(s);
                if(strncmp(exists, s, length) != 0) {
                    ScriptAssert_(mContextOwner, 0, "<internal>", -1,
                                  "Error - Hash collision: '%s', '%s'\n", exists, s);
                }
                return exists;
            }
        }
        const char* FindString(uint32 hash) {
            // -- sanity check
            if(hash == 0)
                return "";

            const char* stringptr = mStringDictionary->FindItem(hash);
            return stringptr;
        }

        const CHashTable<const char>* GetStringDictionary() {
            return mStringDictionary;
        }

    private:
        CScriptContext* mContextOwner;

        uint32 mSize;
        char* mBuffer;
        char* mBufptr;

        CHashTable<const char>* mStringDictionary;
};

} // TinScript

#endif // __TINSTRINGTABLE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
