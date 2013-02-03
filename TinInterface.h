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
// TinInterface.h
// ------------------------------------------------------------------------------------------------

#ifndef __TININTERFACE_H
#define __TININTERFACE_H

#include "TinParse.h"

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// -- GetGlobalVar function to access scripted globals
template <typename T>
bool GetGlobalVar(const char* varname, T& value) {
    if(!GetGlobalNamespace() || !varname ||!varname[0])
        return false;

    CVariableEntry* ve = GetGlobalNamespace()->GetVarTable()->FindItem(Hash(varname));
    if(!ve)
        return false;

    // -- see if we can recognize an appropriate type
    eVarType returntype = GetRegisteredType(GetTypeID<T>());
    if(returntype == TYPE_NULL)
        return false;

    void* convertvalue = TypeConvert(ve->GetType(), ve->GetAddr(NULL), returntype);
    if(!convertvalue)
        return false;

    // -- set the return value
    if(returntype == TYPE_string)
        value = reinterpret_cast<T>(convertvalue);
    else
        value = *reinterpret_cast<T*>(convertvalue);

    return true;
}

template <typename T>
bool ObjExecF(unsigned int objectid, T& returnval, const char* methodformat, ...) {
    // -- sanity check
    if(objectid == 0 || !methodformat || !methodformat[0])
        return false;

    CObjectEntry* oe = CNamespace::FindObject(objectid);
    if(!oe) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to find object: %d\n", objectid);
        return false;
    }

	// -- ensure we have a variable to hold the return value
    AddVariable(GetGlobalNamespace()->GetVarTable(), NULL, "__return", Hash("__return"),
                TYPE_string);

    // -- expand the formated buffer
    va_list args;
    va_start(args, methodformat);
    char methodbuf[kMaxTokenLength];
    vsprintf_s(methodbuf, kMaxTokenLength, methodformat, args);
    va_end(args);

    char execbuf[kMaxTokenLength];
    sprintf_s(execbuf, kMaxTokenLength - strlen(methodbuf), "__return = %d.%s", objectid,
              methodbuf);

    // -- execute the command
    bool result = ExecCommand(execbuf);

    // -- if successful, return the result
    if(result)
        return GetGlobalVar("__return", returnval);
    else
        return false;
}

template <typename T>
bool ExecF(T& returnval, const char* stmtformat, ...) {
    // -- sanity check
    if(!stmtformat || !stmtformat[0])
        return false;

	// -- ensure we have a variable to hold the return value
    AddVariable(GetGlobalNamespace()->GetVarTable(), NULL, "__return", Hash("__return"),
                TYPE_string);

    va_list args;
    va_start(args, stmtformat);
    char stmtbuf[kMaxTokenLength];
    vsprintf_s(stmtbuf, kMaxTokenLength, stmtformat, args);
    va_end(args);

    char execbuf[kMaxTokenLength];
    sprintf_s(execbuf, kMaxTokenLength - strlen(stmtbuf), "__return = %s", stmtbuf);

    // -- execute the command
    bool result = ExecCommand(execbuf);

    // -- if successful, return the result
    if(result)
        return GetGlobalVar("__return", returnval);
    else
        return false;
}

} // TinScript

#endif // __TININTERFACE

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------

