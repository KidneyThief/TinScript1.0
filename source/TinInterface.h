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

// -- includes
#include "TinVariableEntry.h"
#include "TinParse.h"

namespace TinScript {

// ====================================================================================================================
// CreateContext():  Creates a singleton context, max of one for each thread
// ====================================================================================================================
CScriptContext* CreateContext(TinPrintHandler printhandler, TinAssertHandler asserthandler, bool is_main_thread);

// ====================================================================================================================
// UpdateContext():  Updates the singleton context in the calling thread
// ====================================================================================================================
void UpdateContext(uint32 current_time_msec);

// ====================================================================================================================
// DestroyContext():  Destroys the context created from the calling thread
// ====================================================================================================================
void DestroyContext();

// ====================================================================================================================
// GetContext():  Uses a thread local global var to return the specific context created from this thread
// ====================================================================================================================
CScriptContext* GetContext();

// ====================================================================================================================
// ExecCommand():  Executes a text block of valid script code
// ====================================================================================================================
bool8 ExecCommand(const char* statement);

// ====================================================================================================================
// CompileScript():  Compile (without executing) a text file containing script code
// ====================================================================================================================
bool8 CompileScript(const char* filename);

// ====================================================================================================================
// ExecScript():  Executes a text file containing script code
// ====================================================================================================================
bool8 ExecScript(const char* filename);

// ====================================================================================================================
// GetGlobalVar():  Provides access from code, to a registered or scripted global variable
// Must be used if the global is declared in script (not registered from code)
// Must be used, of the global is of type const char* (or in string, in script)
// ====================================================================================================================
template <typename T>
bool8 GetGlobalVar(CScriptContext* script_context, const char* varname, T& value) {
    // -- sanity check
    if (!script_context->GetGlobalNamespace() || !varname ||!varname[0])
        return false;

    CVariableEntry*
        ve = script_context->GetGlobalNamespace()->GetVarTable()->FindItem(Hash(varname));
    if (!ve)
        return false;

    // -- see if we can recognize an appropriate type
    eVarType returntype = GetRegisteredType(GetTypeID<T>());
    if (returntype == TYPE_NULL)
        return false;

    // -- note we're using GetValueAddr() - which returns a const char*, not an STE, for TYPE_string
    void* convertvalue = TypeConvert(script_context, ve->GetType(), ve->GetValueAddr(NULL), returntype);
    if (!convertvalue)
        return false;

    // -- set the return value
    if (returntype == TYPE_string)
    {
        value = (T)(convertvalue);
    }
    else
    {
        value = *reinterpret_cast<T*>((uint32*)(convertvalue));
    }

    return true;
}

// ====================================================================================================================
// SetGlobalVar():  Provides access for code to modify the value of a registered or scripted global variable
// Must be used if the global is declared in script (not registered from code)
// Must be used, of the global is of type const char* (or in string, in script)
// ====================================================================================================================
template <typename T>
bool8 SetGlobalVar(CScriptContext* script_context, const char* varname, T value) {
    // -- sanity check
    if (!script_context->GetGlobalNamespace() || !varname ||!varname[0])
        return false;

    CVariableEntry*
        ve = script_context->GetGlobalNamespace()->GetVarTable()->FindItem(Hash(varname));
    if (!ve)
        return false;

    // -- see if we can recognize an appropriate type
    eVarType input_type = GetRegisteredType(GetTypeID<T>());
    if (input_type == TYPE_NULL)
        return false;

    void* convertvalue = TypeConvert(script_context, ve->GetType(), convert_to_void_ptr<T>::Convert(value), input_type);
    if (!convertvalue)
        return false;

    // -- set the value - note, we're using SetValueAddr(), not SetValue(), which uses a const char*,
    // -- not an STE, for TYPE_string
    ve->SetValueAddr(NULL, convertvalue);
    return true;
}

// ====================================================================================================================
// ObjExecF():  From code, Executed a method, either registered or scripted for an object
// Used when the actual object address is provided
// ====================================================================================================================
template <typename T>
bool8 ObjExecF(void* objaddr, T& returnval, const char* methodformat, ...)
{
    CScriptContext* script_context = TinScript::GetContext();

    // -- sanity check
    if (!script_context || !objaddr)
        return (false);

    uint32 objectid = script_context->FindIDByAddress(objaddr);
    if (objectid == 0)
    {
        ScriptAssert_(script_context, 0,
                      "<internal>", -1, "Error - object not registered: 0x%x\n",
                      kPointerToUInt32(objaddr));
        return false;
    }

	// -- ensure we have a variable to hold the return value
    AddVariable(script_context, script_context->GetGlobalNamespace()->GetVarTable(), NULL,
                "__return", Hash("__return"), TYPE_string);

    // -- expand the formatted buffer
    va_list args;
    va_start(args, methodformat);
    char methodbuf[kMaxTokenLength];
    vsprintf_s(methodbuf, kMaxTokenLength, methodformat, args);
    va_end(args);

    char execbuf[kMaxTokenLength];
    sprintf_s(execbuf, kMaxTokenLength - strlen(methodbuf), "__return = %d.%s", objectid,
              methodbuf);

        // -- execute the command
    bool result = script_context->ExecCommand(execbuf);

    // -- if successful, return the result
    if (result)
        return GetGlobalVar(script_context, "__return", returnval);
    else
        return false;
}

// ====================================================================================================================
// ObjExecF():  From code, Executed a method, either registered or scripted for an object
// Used when the object ID (not the address) is provided
// ====================================================================================================================
template <typename T>
bool8 ObjExecF(uint32 objectid, T& returnval, const char* methodformat, ...)
{
    CScriptContext* script_context = TinScript::GetContext();

    // -- sanity check
    if (!script_context || objectid == 0 || !methodformat || !methodformat[0])
        return false;

    CObjectEntry* oe = script_context->FindObjectEntry(objectid);
    if (!oe) {
        ScriptAssert_(script_context, 0,
                      "<internal>", -1, "Error - unable to find object: %d\n", objectid);
        return false;
    }

	// -- ensure we have a variable to hold the return value
    AddVariable(script_context, script_context->GetGlobalNamespace()->GetVarTable(), NULL,
                "__return", Hash("__return"), TYPE_string);

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
    bool result = script_context->ExecCommand(execbuf);

    // -- if successful, return the result
    if (result)
        return GetGlobalVar(script_context, "__return", returnval);
    else
        return false;
}

// ====================================================================================================================
// ExecF():  From code, Executed a global function either registered or scripted
// ====================================================================================================================
template <typename T>
bool ExecF(T& returnval, const char* stmtformat, ...) {

    CScriptContext* script_context = TinScript::GetContext();

    // -- sanity check
    if (!script_context || !stmtformat || !stmtformat[0])
        return false;

	// -- ensure we have a variable to hold the return value
    AddVariable(script_context, script_context->GetGlobalNamespace()->GetVarTable(),
                NULL, "__return", Hash("__return"), TYPE_string);

    va_list args;
    va_start(args, stmtformat);
    char stmtbuf[kMaxTokenLength];
    vsprintf_s(stmtbuf, kMaxTokenLength, stmtformat, args);
    va_end(args);

    char execbuf[kMaxTokenLength];
    sprintf_s(execbuf, kMaxTokenLength - strlen(stmtbuf), "__return = %s", stmtbuf);

    // -- execute the command
    bool result = script_context->ExecCommand(execbuf);

    // -- if successful, return the result
    if (result)
        return GetGlobalVar(script_context, "__return", returnval);
    else
        return false;
}

} // TinScript

#endif // __TININTERFACE

// ====================================================================================================================
// EOF
// ====================================================================================================================

