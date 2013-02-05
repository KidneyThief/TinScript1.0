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
// TinTypes.cpp : Registered types for use with TinScript
// ------------------------------------------------------------------------------------------------

// -- lib includes
#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "TinTypes.h"
#include "TinScript.h"
#include "TinStringTable.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// types
const char* gRegisteredTypeNames[TYPE_COUNT] = {
	#define VarTypeEntry(a, b, c, d, e) #a,
	VarTypeTuple
	#undef VarTypeEntry
};

const char* GetRegisteredTypeName(eVarType vartype) {
	return gRegisteredTypeNames[vartype];
}

eVarType GetRegisteredType(const char* token, int length) {
    if(!token)
        return TYPE_NULL;
	for(eVarType i = TYPE_void; i < TYPE_COUNT; i = eVarType(i + 1)) {
		int comparelength = int(strlen(gRegisteredTypeNames[i])) > length ?
																 strlen(gRegisteredTypeNames[i]) :
																 length;
		if(!Strncmp_(token, gRegisteredTypeNames[i], comparelength)) {
			return i;
		}
	}
	return TYPE_NULL;
}

int gRegisteredTypeSize[TYPE_COUNT] = {
	#define VarTypeEntry(a, b, c, d, e) b,
	VarTypeTuple
	#undef VarTypeEntry
};

TypeToString gRegisteredTypeToString[TYPE_COUNT] = {
	#define VarTypeEntry(a, b, c, d, e) c,
	VarTypeTuple
	#undef VarTypeEntry
};

StringToType gRegisteredStringToType[TYPE_COUNT] = {
	#define VarTypeEntry(a, b, c, d, e) d,
	VarTypeTuple
	#undef VarTypeEntry
};

unsigned int gRegisteredTypeID[TYPE_COUNT] = {
	#define VarTypeEntry(a, b, c, d, e) GetTypeID<e>(),
	VarTypeTuple
	#undef VarTypeEntry
};

eVarType GetRegisteredType(unsigned int id) {
	for(eVarType i = FIRST_VALID_TYPE; i < TYPE_COUNT; i = eVarType(i + 1)) {
        if(id == gRegisteredTypeID[i]) {
			return i;
		}
	}
	return TYPE_NULL;
}

// ------------------------------------------------------------------------------------------------
bool VoidToString(void* value, char* buf, int bufsize) {
	if (buf && bufsize > 0) {
		*buf = '\0';
		return true;
	}
	return false;
}

bool StringToVoid(void* addr, char* value) {
	return true;
}

// ------------------------------------------------------------------------------------------------
bool STEToString(void* value, char* buf, int bufsize) {
	if(value && buf && bufsize > 0) {
        sprintf_s(buf, bufsize, "%s", CStringTable::FindString(*(unsigned int*)value));
		return true;
	}
	return false;
}

bool StringToSTE(void* addr, char* value) {
    // -- an STE is simply an address, copy the 4x bytes verbatim
	if(addr && value) {
		unsigned int* varaddr = (unsigned int*)addr;
		*varaddr = Hash(value);
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
bool IntToString(void* value, char* buf, int bufsize) {
	if(value && buf && bufsize > 0) {
		sprintf_s(buf, bufsize, "%d", *(int*)(value));
		return true;
	}
	return false;
}

bool StringToInt(void* addr, char* value) {
	if(addr && value) {
		int* varaddr = (int*)addr;
		*varaddr = atoi(value);
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
bool BoolToString(void* value, char* buf, int bufsize) {
	if(value && buf && bufsize > 0) {
		sprintf_s(buf, bufsize, "%s", *(bool*)(value) ? "true" : "false");
		return true;
	}
	return false;
}

bool StringToBool(void* addr, char* value) {
	if(addr && value) {
		bool* varaddr = (bool*)addr;
		if (!_stricmp(value, "false") || !_stricmp(value, "0") ||
					!_stricmp(value, "0.0") || !_stricmp(value, "0.0f") || !_stricmp(value, "")) {
			*varaddr = false;
			return true;
		}
		else {
			*varaddr = true;
			return true;
		}
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
bool FloatToString(void* value, char* buf, int bufsize) {
	if(value && buf && bufsize > 0) {
		sprintf_s(buf, bufsize, "%.4f", *(float*)(value));
		return true;
	}
	return false;
}

bool StringToFloat(void* addr, char* value) {
	if(addr && value) {
		float* varaddr = (float*)addr;
		*varaddr = float(atof(value));
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
// -- non-string conversions - mostly to avoid converting fromtype -> string -> totype
void* TypeConvert(eVarType fromtype, void* fromaddr, eVarType totype) {

    // -- writing here, since this conversion should only be called from CCodeBlock::Execute()
    // -- used mostly for binary ops, but allowing for 8x simultaneous conversion
    static int bufferindex = 0;
    static char buffers[8][kMaxTokenLength];
    char* bufferptr = buffers[(++bufferindex) % 8];

    static char outbuf[kMaxTokenLength];

    // -- sanity check
    if(fromtype == totype || !fromaddr)
        return fromaddr;

    switch(fromtype) {
        case TYPE_int:
            switch(totype) {
                case TYPE_bool:
                    *(bool*)(bufferptr) = (*(int*)(fromaddr) != 0);
                    return (void*)(bufferptr);
                case TYPE_float:
                    *(float*)(bufferptr) = (float)*(int*)(fromaddr);
                    return (void*)(bufferptr);
                default:
                    break;
            }
        break;
        case TYPE_bool:
            switch(totype) {
                case TYPE_int:
                    *(int*)(bufferptr) = *(bool*)fromaddr ? 1 : 0;
                    return (void*)(bufferptr);
                case TYPE_float:
                    *(float*)(bufferptr) = *(bool*)fromaddr ? 1.0f : 0.0f;
                    return (void*)(bufferptr);
                default:
                    break;
            }
        break;
        case TYPE_float:
            switch(totype) {
                case TYPE_bool:
                    *(bool*)(bufferptr) = (*(float*)(fromaddr) != 0.0f);
                    return (void*)(bufferptr);
                case TYPE_int:
                    *(int*)(bufferptr) = (int)*(float*)(fromaddr);
                    return (void*)(bufferptr);
                default:
                    break;
            }
        break;

        default:
            break;
    }

    // -- if we haven't handled the conversion above, do it the slow way
    // $$$TZA This is nasty - any conversion that hasn't been implemented
    // -- above, goes into string, then back out into the destination type
    char* convertbuf = buffers[(++bufferindex) % 8];
    char* destbuf = buffers[(++bufferindex) % 8];
    // convert to string, then back into a value of the correct type
	gRegisteredTypeToString[fromtype](fromaddr, convertbuf, kMaxTokenLength);
	gRegisteredStringToType[totype]((void*)destbuf, convertbuf);
    return (void*)destbuf;
}

const char* DebugPrintVar(void* addr, eVarType vartype) {
    static int bufferindex = 0;
    static char buffers[8][kMaxTokenLength];
    char* bufferptr = buffers[(++bufferindex) % 8];

    if(!addr)
        return "";
    char* convertbuf = buffers[(++bufferindex) % 8];
    char* destbuf = buffers[(++bufferindex) % 8];
	gRegisteredTypeToString[vartype](addr, convertbuf, kMaxTokenLength);
    sprintf_s(destbuf, kMaxTokenLength, "[%s] %s", GetRegisteredTypeName(vartype), convertbuf);
    return convertbuf;
}

bool SafeStrcpy(char* dest, const char* src, int max) {
	// terminate the dest pointer, in case we copy a zero length string
	if (dest)
		*dest = '\0';

	// -- sanity check
	if(! dest || ! src || max <= 0) {
		return false;
	}

	char* destptr = dest;
	const char* srcptr = src;
	int count = max - 1;
	while (*srcptr != '\0' && count > 0) {
		*destptr++ = *srcptr++;
		--count;
	}
	*destptr = '\0';
	return true;
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
