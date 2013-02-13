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
        CStringTable(unsigned int _size) {
            assert(_size > 0);
            size = _size;
            buffer = TinAllocArray(ALLOC_StringTable, char, size);
            bufptr = buffer;

            stringdictionary = TinAlloc(ALLOC_StringTable, CHashTable<const char>,
                                        kStringTableDictionarySize);
        }

        virtual ~CStringTable() {
            TinFree(stringdictionary);
            TinFree(buffer);
        }

        static void Initialize() {
            assert(gInstance == NULL);
            gInstance = TinAlloc(ALLOC_StringTable, CStringTable, kStringTableSize);
        }

        static void Shutdown() {
            assert(gInstance != NULL);
            TinFree(gInstance);
            gInstance = NULL;
        }

        static const char* AddString(const char* s, int length = -1, unsigned int hash = 0) {
            // -- sanity check
            if(!gInstance || !s)
                return "";

            if(hash == 0) {
                hash = Hash(s, length);
            }

            // -- see if the string is already in the dictionary
            const char* exists = FindString(hash);
            if(!exists)
            {
                if(length < 0)
                    length = strlen(s);

                // -- space left
                int remaining = gInstance->size - ((unsigned int)gInstance->bufptr -
                                                            (unsigned int)gInstance->buffer);
                if(remaining < length + 1) {
                    ScriptAssert_(0, "<internal>", -1, "Error - StringTable of size %d is full!\n",
                                  gInstance->size);
                    return NULL;
                }
                const char* stringptr = gInstance->bufptr;
                SafeStrcpy(gInstance->bufptr, s, length + 1);
                gInstance->bufptr += length + 1;

                // -- add the entry to the dictionary
                gInstance->stringdictionary->AddItem(*stringptr, hash);

                return stringptr;
            }

            // -- else check for a collision
            else
            {
                if(length < 0)
                    length = strlen(s);
                if(strncmp(exists, s, length) != 0) {
                    ScriptAssert_(0, "<internal>", -1, "Error - Hash collision: '%s', '%s'\n", exists, s);
                }
                return exists;
            }
        }
        static const char* FindString(unsigned int hash) {
            // -- sanity check
            if(!gInstance || hash == 0)
                return "";

            const char* stringptr = gInstance->stringdictionary->FindItem(hash);
            return stringptr;
        }

        static const CHashTable<const char>* GetStringDictionary() {
            if(!gInstance)
                return NULL;
            return gInstance->stringdictionary;
        }

    private:
        static CStringTable* gInstance;

        unsigned int size;
        char* buffer;
        char* bufptr;

        CHashTable<const char>* stringdictionary;
};

} // TinScript

#endif // __TINSTRINGTABLE_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
