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
// TinScript.cpp
// ------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "TinRegistration.h"
#include "TinScheduler.h"
#include "TinObjectGroup.h"
#include "TinScript.h"

// ------------------------------------------------------------------------------------------------
// statics - mostly for the quick and dirty console implementation

namespace TinScript {

CScriptContext* CScriptContext::gMainThreadContext = NULL;
CHashTable<CScriptContext>* CScriptContext::gScriptContextList = NULL;

CRegFunctionBase* CRegFunctionBase::gRegistrationList = NULL;

// ------------------------------------------------------------------------------------------------
// --  CRegisterGlobal  ---------------------------------------------------------------------------

CRegisterGlobal* CRegisterGlobal::head = NULL;
CRegisterGlobal::CRegisterGlobal(const char* _name, TinScript::eVarType _type, void* _addr) {
    name = _name;
    type = _type;
    addr = _addr;
    next = NULL;

    // -- if we've got and address, hook it into the linked list for registration
    if(name && name[0] && addr) {
        next = head;
        head = this;
    }
}

void CRegisterGlobal::RegisterGlobals(CScriptContext* script_context) {
    CRegisterGlobal* global = CRegisterGlobal::head;
    while(global) {
        // -- create the var entry, add it to the global namespace
        CVariableEntry* ve = TinAlloc(ALLOC_VarEntry, CVariableEntry, script_context, global->name,
                                                                      global->type, global->addr);
	    uint32 hash = ve->GetHash();
	    script_context->GetGlobalNamespace()->GetVarTable()->AddItem(*ve, hash);

        // -- next registration object
        global = global->next;
    }
}

// ------------------------------------------------------------------------------------------------
// -- Set of functions that all access the interals of the ScriptContext...
// -- need to be regeistered separately, as they all take a CScriptContext* as the first param

void ContextPrint(CScriptContext* script_context, const char* msg) {
    TinPrint(script_context, "%s\n", msg);
}

bool8 ContextCompile(CScriptContext* script_context, const char* filename) {
    CCodeBlock* result = TinScript::CScriptContext::CompileScript(script_context, filename);
    script_context->ResetAssertStack();
    return (result != NULL);
}

bool8 ContextExecScript(CScriptContext* script_context, const char* filename) {
    return (script_context->ExecScript(filename));
}

void ContextListObjects(CScriptContext* script_context) {
    script_context->ListObjects();
}

bool8 ContextIsObject(CScriptContext* script_context, uint32 objectid) {
    bool8 found = script_context->FindObject(objectid) != NULL;
    return found;
}

uint32 ContextFindObjectByName(CScriptContext* script_context, const char* objname) {
    TinScript::CObjectEntry* oe = script_context->FindObjectByName(objname);
    return oe ? oe->GetID() : 0;
}

void ContextAddDynamicVariable(CScriptContext* script_context, uint32 objectid,
                               const char* varname, const char* vartype) {
    script_context->AddDynamicVariable(objectid, varname, vartype);
}

void ContextLinkNamespaces(CScriptContext* script_context, const char* childns,
                           const char* parentns) {
    script_context->LinkNamespaces(childns, parentns);
}

void ContextListVariables(CScriptContext* script_context, uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = script_context->FindObjectEntry(objectid);
        if(!oe) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to find object: %d\n", objectid);
        }
        else {
            TinScript::DumpVarTable(oe);
        }
    }
    else {
        TinScript::DumpVarTable(script_context, NULL,
                                script_context->GetGlobalNamespace()->GetVarTable());
    }
}

void ContextListFunctions(CScriptContext* script_context, uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = script_context->FindObjectEntry(objectid);
        if(!oe) {
            ScriptAssert_(script_context, 0, "<internal>", -1,
                          "Error - Unable to find object: %d\n", objectid);
        }
        else {
            TinScript::DumpFuncTable(oe);
        }
    }
    else {
        TinScript::DumpFuncTable(script_context, script_context->GetGlobalNamespace()->GetFuncTable());
    }
}

const char* ContextGetObjectNamespace(CScriptContext* script_context, uint32 objectid) {
    if(objectid > 0) {
        TinScript::CObjectEntry* oe = script_context->FindObjectEntry(objectid);
        if(!oe) {
            return "";
        }
        else {
            return TinScript::UnHash(oe->GetNamespace()->GetHash());
        }
    }
    else {
        return "";
    }
}

void ContextListSchedules(CScriptContext* script_context) {
    script_context->GetScheduler()->Dump();
}

void ContextScheduleCancel(CScriptContext* script_context, int32 reqid) {
    script_context->GetScheduler()->CancelRequest(reqid);
}

void ContextScheduleCancelObject(CScriptContext* script_context, uint32 objectid) {
    script_context->GetScheduler()->CancelObject(objectid);
}

uint32 ContextCreateObjectSet(CScriptContext* script_context, const char* name) {
    CObjectSet* object_set = TinAlloc(ALLOC_ObjectGroup, CObjectSet, script_context,
                                      kObjectGroupTableSize);
    uint32 object_id = script_context->RegisterObject(object_set, "CObjectSet", name);
    return (object_id);
}

uint32 ContextCreateObjectGroup(CScriptContext* script_context, const char* name) {
    CObjectSet* object_set = TinAlloc(ALLOC_ObjectGroup, CObjectGroup, script_context,
        kObjectGroupTableSize);
    uint32 object_id = script_context->RegisterObject(object_set, "CObjectGroup", name);
    return (object_id);
}

// ------------------------------------------------------------------------------------------------
// -- all CScriptContexts have these functions registered automatically
void CScriptContext::RegisterContextFunctions() {
    CONTEXT_FUNCTION_P1(Print, ContextPrint, void, const char*);
    CONTEXT_FUNCTION_P1(Compile, ContextCompile, bool8, const char*);
    CONTEXT_FUNCTION_P1(Exec, ContextExecScript, bool8, const char*);
    CONTEXT_FUNCTION_P0(ListObjects, ContextListObjects, void);
    CONTEXT_FUNCTION_P1(IsObject, ContextIsObject, bool8, uint32);
    CONTEXT_FUNCTION_P1(FindObject, ContextFindObjectByName, uint32, const char*);
    CONTEXT_FUNCTION_P3(AddDynamicVar, ContextAddDynamicVariable, void, uint32, const char*, const char*);
    CONTEXT_FUNCTION_P2(LinkNamespaces, ContextLinkNamespaces, void, const char*, const char*);
    CONTEXT_FUNCTION_P1(ListVariables, ContextListVariables, void, uint32);
    CONTEXT_FUNCTION_P1(ListFunctions, ContextListFunctions, void, uint32);
    CONTEXT_FUNCTION_P1(GetObjectNamespace, ContextGetObjectNamespace, const char*, uint32);
    CONTEXT_FUNCTION_P1(CreateObjectSet, ContextCreateObjectSet, uint32, const char*);
    CONTEXT_FUNCTION_P1(CreateObjectGroup, ContextCreateObjectGroup, uint32, const char*);

    CONTEXT_FUNCTION_P0(ListSchedules, ContextListSchedules, void);
    CONTEXT_FUNCTION_P1(ScheduleCancel, ContextScheduleCancel, void, int32);
    CONTEXT_FUNCTION_P1(ScheduleCancelObject, ContextScheduleCancelObject, void, uint32);
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
