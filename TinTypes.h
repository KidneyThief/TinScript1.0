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

#ifndef __TINTYPES_H
#define __TINTYPES_H

namespace TinScript {

// -- constants
const int kMaxNameLength = 255;
const int kMaxTokenLength = 2048;
const int kMaxTypeSize = 8;  // increase this if we register, say, a C3Vector type

// ------------------------------------------------------------------------------------------------
// ghetto type manipulation templates

template <typename T>
unsigned int GetTypeID() {
    static T t;
    return (unsigned int)&t;
}

template <typename T>
unsigned int GetTypeID(T& t) {
    return GetTypeID<T>();
}

template <typename T0, typename T1>
bool CompareTypes() {
    return GetTypeID<T0>() == GeTypeID<T1>();
}

template<typename T>
struct is_pointer {
    static const bool value = false;
};

template<typename T>
struct is_pointer<T*> {
    static const bool value = true;
};

template<typename T>
struct remove_ptr {
   typedef T type;
};
template<typename T>
struct remove_ptr<T*> {
   typedef T type;
};

template<typename T>
struct convert_from_void_ptr {
    static T Convert(void* addr) {
        return *reinterpret_cast<T*>(addr);
    }
};

template<typename T>
struct convert_from_void_ptr<T*> {
    static T* Convert(void* addr) {
        return reinterpret_cast<T*>(addr);
    }
};

template<typename T>
struct convert_to_void_ptr {
    static void* Convert(T& t) {
        return reinterpret_cast<void*>(&t);
    }
};

template<typename T>
struct convert_to_void_ptr<const T*> {
    static void* Convert(const T* t) {
        return reinterpret_cast<void*>(const_cast<T*>(t));
    }
};

// ------------------------------------------------------------------------------------------------
// implementation for integrating the registered types

typedef bool (*TypeToString)(void* value, char* buf, int bufsize);
typedef bool (*StringToType)(void* addr, char* value);

// ------------------------------------------------------------------------------------------------
bool VoidToString(void* value, char* buf, int bufsize);
bool StringToVoid(void* addr, char* value);
bool STEToString(void* value, char* buf, int bufsize);
bool StringToSTE(void* addr, char* value);
bool IntToString(void* value, char* buf, int bufsize);
bool StringToInt(void* addr, char* value);
bool BoolToString(void* value, char* buf, int bufsize);
bool StringToBool(void* addr, char* value);
bool FloatToString(void* value, char* buf, int bufsize);
bool StringToFloat(void* addr, char* value);

// -- for all non-first class types, declare a struct so GetTypeID<type> will be unique
struct sMember {
    typedef unsigned int type;
};

struct sHashTable {
    typedef unsigned int type;
};

struct sHashVar {
    typedef unsigned int type;
};

// -- use a tuple to define the token types, and their debug names
// -- note:  everything from _member and below is essentially a reserved token, and cannot
// -- be used as an active part of the language.
// -- Everything from TYPE_type and on is a valid part of the language syntax,
// -- and the last column mapping requires each "registered type" to be unique.
#define FIRST_VALID_TYPE TYPE_object
#define VarTypeTuple \
	VarTypeEntry(NULL,		    0,		VoidToString,		StringToVoid,       unsigned char)  \
	VarTypeEntry(void,		    0,		VoidToString,		StringToVoid,       unsigned char)	\
	VarTypeEntry(_resolve,	    0,		VoidToString,		StringToVoid,       unsigned char)	\
	VarTypeEntry(_stackvar,     8,		IntToString,		StringToInt,        unsigned char)	\
	VarTypeEntry(_var,          12,		IntToString,		StringToInt,        unsigned char)	\
	VarTypeEntry(_member,       8,		IntToString,		StringToInt,        sMember)    	\
	VarTypeEntry(_hashvar,      16,		IntToString,		StringToInt,        sHashVar)    	\
    VarTypeEntry(hashtable,     4,      IntToString,        StringToInt,        sHashTable)     \
	VarTypeEntry(object,        4,		IntToString,		StringToInt,        unsigned int)   \
    VarTypeEntry(string,        4,      STEToString,        StringToSTE,        const char*)    \
	VarTypeEntry(int,		    4,		IntToString,		StringToInt,        int)		    \
	VarTypeEntry(bool,		    1,		BoolToString,		StringToBool,       bool)		    \
	VarTypeEntry(float,		    4,		FloatToString,		StringToFloat,      float)		    \

// -- 4x words actually, 16x bytes, the size of a HashVar
#define MAX_TYPE_SIZE 4

enum eVarType {
	#define VarTypeEntry(a, b, c, d, e) TYPE_##a,
	VarTypeTuple
	#undef VarTypeEntry

	TYPE_COUNT
};

void* TypeConvert(eVarType fromtype, void* fromaddr, eVarType totype);
const char* DebugPrintVar(void* addr, eVarType vartype);

// ------------------------------------------------------------------------------------------------
// Three types of registered functions: script, global, and method
#define FunctionTypeTuple \
	FunctionTypeEntry(NULL)		    	\
	FunctionTypeEntry(Script)			\
	FunctionTypeEntry(Global)			\
	FunctionTypeEntry(Method)			\

enum EFunctionType {
	#define FunctionTypeEntry(a) eFuncType##a,
	FunctionTypeTuple
	#undef FunctionTypeEntry
	eFuncTypeCount
};

// ------------------------------------------------------------------------------------------------
const char* GetRegisteredTypeName(eVarType vartype);
eVarType GetRegisteredType(const char* token, int length);
eVarType GetRegisteredType(unsigned int id);

bool SafeStrcpy(char* dest, const char* src, int max);

// ------------------------------------------------------------------------------------------------
// externs

extern const char* gRegisteredTypeNames[TYPE_COUNT];
extern int gRegisteredTypeSize[TYPE_COUNT];
extern TypeToString gRegisteredTypeToString[TYPE_COUNT];
extern StringToType gRegisteredStringToType[TYPE_COUNT];

} // TinScript

#endif // __TINTYPES_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
