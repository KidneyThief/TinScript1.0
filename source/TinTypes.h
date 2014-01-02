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

#include "math.h"

#include "integration.h"

// ------------------------------------------------------------------------------------------------
// $$$TZA temporary C3Vector implementation
class C3Vector {
public:
    C3Vector(float32 _x = 0.0f, float32 _y = 0.0f, float32 _z = 0.0f) {
        x = _x; y = _y; z = _z;
    }

    C3Vector& operator=(const C3Vector& rhs) {
        x = rhs.x; y = rhs.y; z = rhs.z;
        return *this;
    }

    C3Vector& operator+(const C3Vector& rhs) {
        x += rhs.x; y += rhs.y; z += rhs.z;
        return *this;
    }

    C3Vector& operator-(const C3Vector& rhs) {
        x -= rhs.x; y -= rhs.y; z -= rhs.z;
        return *this;
    }

    C3Vector& operator*(const float32 s) {
        x *= s; y *= s; z *= s;
        return *this;
    }

    C3Vector& operator/(const float32 s) {
        x /= s; y /= s; z /= s;
        return *this;
    }

    bool8 operator==(const C3Vector& rhs) {
        return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    bool8 operator!=(const C3Vector& rhs) {
        return (x != rhs.x || y != rhs.y || z != rhs.z);
    }

    float32 Length() {
        float32 length = sqrt(x*x + y*y + z*z);
        return (length);
    }

    static const C3Vector zero;
    static const C3Vector realmax;

    float32 x;
    float32 y;
    float32 z;
};
// ------------------------------------------------------------------------------------------------

namespace TinScript {

// -- constants
const int32 kMaxNameLength = 256;
const int32 kMaxTokenLength = 2048;

// -- current largest var type is a hashtable entry, 16 bytes
const int32 kMaxTypeSize = 16;

// ------------------------------------------------------------------------------------------------
// ghetto type manipulation templates

template <typename T>
uint32 GetTypeID() {
    static T t;
    void* ptr = (void*)&t;
    return kPointerToUInt32(ptr);
}

template <typename T>
uint32 GetTypeID(T&) {
    return GetTypeID<T>();
}

template <typename T0, typename T1>
bool8 CompareTypes() {
    return GetTypeID<T0>() == GetTypeID<T1>();
}

// ------------------------------------------------------------------------------------------------
template<typename T>
struct is_pointer {
    static const bool8 value = false;
};

template<typename T>
struct is_pointer<T*> {
    static const bool8 value = true;
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

typedef bool8 (*TypeToString)(void* value, char* buf, int32 bufsize);
typedef bool8 (*StringToType)(void* addr, char* value);

// ------------------------------------------------------------------------------------------------
bool8 VoidToString(void* value, char* buf, int32 bufsize);
bool8 StringToVoid(void* addr, char* value);
bool8 STEToString(void* value, char* buf, int32 bufsize);
bool8 StringToSTE(void* addr, char* value);
bool8 IntToString(void* value, char* buf, int32 bufsize);
bool8 StringToInt(void* addr, char* value);
bool8 BoolToString(void* value, char* buf, int32 bufsize);
bool8 StringToBool(void* addr, char* value);
bool8 FloatToString(void* value, char* buf, int32 bufsize);
bool8 StringToFloat(void* addr, char* value);
bool8 C3VectorToString(void* value, char* buf, int32 bufsize);
bool8 StringToC3Vector(void* addr, char* value);

// -- for all non-first class types, declare a struct so GetTypeID<type> will be unique
struct sMember {
    typedef uint32 type;
};

struct sHashTable {
    typedef uint32 type;
};

struct sHashVar {
    typedef uint32 type;
};

// -- use a tuple to define the token types, and their debug names
// -- note:  everything from _member and below is essentially a reserved token, and cannot
// -- be used as an active part of the language.
// -- Everything from TYPE_type and on is a valid part of the language syntax,
// -- and the last column mapping requires each "registered type" to be unique.
#define FIRST_VALID_TYPE TYPE_object
#define VarTypeTuple \
	VarTypeEntry(NULL,		    0,		VoidToString,		StringToVoid,       uint8)          \
	VarTypeEntry(void,		    0,		VoidToString,		StringToVoid,       uint8)	        \
	VarTypeEntry(_resolve,	    16,		VoidToString,		StringToVoid,       uint8)	        \
	VarTypeEntry(_stackvar,     8,		IntToString,		StringToInt,        uint8)	        \
	VarTypeEntry(_var,          12,		IntToString,		StringToInt,        uint8)	        \
	VarTypeEntry(_member,       8,		IntToString,		StringToInt,        sMember)    	\
	VarTypeEntry(_hashvar,      16,		IntToString,		StringToInt,        sHashVar)    	\
    VarTypeEntry(hashtable,     4,      IntToString,        StringToInt,        sHashTable)     \
	VarTypeEntry(object,        4,		IntToString,		StringToInt,        uint32)         \
    VarTypeEntry(string,        4,      STEToString,        StringToSTE,        const char*)    \
	VarTypeEntry(int,		    4,		IntToString,		StringToInt,        int32)		    \
	VarTypeEntry(bool,		    1,		BoolToString,		StringToBool,       bool8)		    \
	VarTypeEntry(float,		    4,		FloatToString,		StringToFloat,      float32)		\
	VarTypeEntry(c3vector,	    12,		C3VectorToString,   StringToC3Vector,   C3Vector)		\

// -- 4x words actually, 16x bytes, the size of a HashVar
#define MAX_TYPE_SIZE 4

enum eVarType {
	#define VarTypeEntry(a, b, c, d, e) TYPE_##a,
	VarTypeTuple
	#undef VarTypeEntry

	TYPE_COUNT
};

// ------------------------------------------------------------------------------------------------
// -- for some reason, typedef'ing natural types (e.g. unsigned int to uint32)
// -- broke the population of gRegisteredTypeID, which uses GetTypeID();
/*
template<>
uint32 GetTypeID<uint32>() {
    return TYPE_object;
}

template<>
uint32 GetTypeID<const char*>() {
    return TYPE_string;
}

template<>
uint32 GetTypeID<int32>() {
    return TYPE_int;
}

template<>
uint32 GetTypeID<bool8>() {
    return TYPE_bool;
}

template<>
uint32 GetTypeID<float32>() {
    return TYPE_float;
}
*/

// ------------------------------------------------------------------------------------------------
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
eVarType GetRegisteredType(const char* token, int32 length);
eVarType GetRegisteredType(uint32 id);

bool8 SafeStrcpy(char* dest, const char* src, int32 max);
int32 Atoi(const char* src, int32 length = -1);

// ------------------------------------------------------------------------------------------------
// externs

extern const char* gRegisteredTypeNames[TYPE_COUNT];
extern int32 gRegisteredTypeSize[TYPE_COUNT];
extern TypeToString gRegisteredTypeToString[TYPE_COUNT];
extern StringToType gRegisteredStringToType[TYPE_COUNT];

} // TinScript

#endif // __TINTYPES_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
