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
// TinScript.h
//

#include <new>

#ifndef __INTEGRATION_H
#define __INTEGRATION_H

// ------------------------------------------------------------------------------------------------
// -- TYPES
// ------------------------------------------------------------------------------------------------

typedef bool            bool8;
typedef char            int8;
typedef unsigned char   uint8;
typedef short           int16;
typedef unsigned short  uint16;
typedef int             int32;
typedef unsigned int    uint32;
typedef float           float32;

// ------------------------------------------------------------------------------------------------
// -- MEMORY
// ------------------------------------------------------------------------------------------------

// -- memory allocation types - to adjust to custom memory strategies
// -- e.g.  TreeNode is temporary mem used only for compiling
// --       VarEntry, ObjEntry, FuncEntry are all fixed sizes, and would
// --       perform well if allocated from a memory pool...
#define AllocTypeTuple              \
    AllocTypeEntry(ScriptContext)   \
    AllocTypeEntry(TreeNode)        \
    AllocTypeEntry(CodeBlock)       \
    AllocTypeEntry(FuncCallStack)   \
    AllocTypeEntry(VarTable)        \
    AllocTypeEntry(FuncTable)       \
    AllocTypeEntry(FuncContext)     \
    AllocTypeEntry(VarEntry)        \
    AllocTypeEntry(VarStorage)      \
    AllocTypeEntry(HashTable)       \
    AllocTypeEntry(ObjEntry)        \
    AllocTypeEntry(Namespace)       \
    AllocTypeEntry(SchedCmd)        \
    AllocTypeEntry(FuncCallEntry)   \
    AllocTypeEntry(CreateObj)       \
    AllocTypeEntry(StringTable)     \
    AllocTypeEntry(FileBuf)         \

enum eAllocType {
    #define AllocTypeEntry(a) ALLOC_##a,
    AllocTypeTuple
    #undef AllocTypeEntry
};

#define TinAlloc(alloctype, T, ...) \
    new (reinterpret_cast<T*>(::operator new(sizeof(T)))) T(__VA_ARGS__)

#define TinAllocVarContent(type) \
    new char[gRegisteredTypeSize[_type]];

#define TinAllocInstrBlock(_size) \
    new unsigned int[_size];

#define TinAllocArray(alloctype, T, size) \
    new T[size];

#define TinFree(addr) \
    delete addr;

#define TinFreeArray(addr) \
    delete [] addr;

// ------------------------------------------------------------------------------------------------
// Misc hooks
// ------------------------------------------------------------------------------------------------
// -- Pass a function of the following prototype when creating the CScriptContext
typedef bool8 (*TinAssertHandler)(const char* condition, const char* file, int32 linenumber,
                                  const char* fmt, ...);
#define ScriptAssert_(scriptcontext, condition, file, linenumber, fmt, ...)                     \
    {                                                                                           \
        if(!(condition)) {                                                                      \
            if(!scriptcontext->GetAssertHandler()(#condition, file, linenumber,                 \
                                                  fmt, __VA_ARGS__)) {                          \
                __asm   int 3                                                                   \
            }                                                                                   \
        }                                                                                       \
    }

// -- Pass a function of the following prototype when creating the CScriptContext
typedef int (*TinPrintHandler)(const char* fmt, ...);
#define TinPrint(scriptcontext, fmt, ...)                           \
    {                                                               \
        scriptcontext->GetPrintHandler()(fmt, __VA_ARGS__);         \
    }

#endif // __INTEGRATION_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
