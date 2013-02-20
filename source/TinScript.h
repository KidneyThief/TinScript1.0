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

#ifndef __TINSCRIPT_H
#define __TINSCRIPT_H

#include "stddef.h"
#include "stdarg.h"

#include "integration.h"
#include "TinTypes.h"
#include "TinNamespace.h"

#define DEBUG_CODEBLOCK 1
#define FORCE_COMPILE 1
#define CASE_SENSITIVE 1
#define DEBUG_TRACE 1

#if CASE_SENSITIVE
    #define Strncmp_ strncmp
#else
    #define Strncmp_ _strnicmp
#endif

// ------------------------------------------------------------------------------------------------
// Registration macros
// $$$TZA FindIDByAddress currently assumes the MainThreadContext... need to ... figure this out
#define DECLARE_SCRIPT_CLASS(classname, parentclass)                                              \
    static const char* GetParentName() { return #parentclass; }                                   \
    static const char* GetClassName() { return #classname; }                                      \
    static classname* Create() {                                                                  \
        classname* newobj = TinAlloc(ALLOC_CreateObj, classname);                                 \
        return newobj;                                                                            \
    }                                                                                             \
    static void Destroy(void* addr) {                                                             \
        if(addr) {                                                                                \
            classname* obj = static_cast<classname*>(addr);                                       \
            TinFree(obj);                                                                         \
        }                                                                                         \
    }                                                                                             \
    uint32 GetObjectID() const {                                                                  \
        return TinScript::CScriptContext::GetMainThreadContext()->FindIDByAddress((void*)this);  \
    }                                                                                             \
    SCRIPT_DEFAULT_METHODS(classname);                                                            \
    static void Register(TinScript::CNamespace* _classnamespace);                                 \
    static TinScript::CNamespace* classnamespace;

#define IMPLEMENT_SCRIPT_CLASS(classname, parentname)                                        \
    TinScript::CNamespace* classname::classnamespace = NULL;                                 \
    TinScript::CNamespaceReg reg_##classname(#classname, #parentname, classname::Create,     \
                                             classname::Destroy, classname::Register,        \
                                             &classname::classnamespace);                    \
    IMPLEMENT_DEFAULT_METHODS(classname);                                                    \
    void classname::Register(TinScript::CNamespace *classnamespace)

#define REGISTER_MEMBER(classname, scriptname, membername)                                   \
    {                                                                                        \
        classname* classptr = reinterpret_cast<classname*>(0);                               \
        uint32 varhash = TinScript::Hash(#scriptname);                                       \
        TinScript::CVariableEntry* ve =                                                      \
            TinAlloc(ALLOC_VarEntry, TinScript::CVariableEntry, TinScript::CScriptContext::GetMainThreadContext(), #scriptname, varhash,        \
            TinScript::GetRegisteredType(TinScript::GetTypeID(classptr->membername)), true,  \
            offsetof(classname, membername));                                                \
        classnamespace->GetVarTable()->AddItem(*ve, varhash);                                \
    }

#define REGISTER_GLOBAL_VAR(scriptname, var)                                                 \
    TinScript::CRegisterGlobal _reg_gv_##scriptname(#scriptname,                             \
        TinScript::GetRegisteredType(TinScript::GetTypeID(var)), (void*)&var);

// ------------------------------------------------------------------------------------------------
// constants

#define kMainThreadName "MainThread"

const int32 kCompilerVersion = 1;

const int32 kMaxArgs = 256;
const int32 kMaxArgLength = 256;

const int32 kScriptContextThreadSize = 7;

const int32 kGlobalFuncTableSize = 97;
const int32 kGlobalVarTableSize = 97;

const int32 kLocalFuncTableSize = 17;
const int32 kLocalVarTableSize = 17;

const int32 kFunctionCallStackSize = 32;

const int32 kStringTableSize = 32 * 1024;
const int32 kStringTableDictionarySize = 199;

const int32 kObjectTableSize = 10007;

#define kBytesToWordCount(a) ((a) + 3) / 4;

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// forward declarations

class CVariableEntry;
class CFunctionEntry;
class CNamespace;
class CCodeBlock;
class CStringTable;
class CScheduler;
class CScriptContext;
class CNamespaceContext;
class CObjectEntry;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

void SaveStringTable(CScriptContext* script_context);
void LoadStringTable(CScriptContext* script_context);

// --Global Var Registration-----------------------------------------------------------------------
class CRegisterGlobal {
    public:
        CRegisterGlobal(const char* _name = NULL, TinScript::eVarType _type = TinScript::TYPE_NULL,
                        void* _addr = NULL);
        virtual ~CRegisterGlobal() { }

        static void RegisterGlobals(CScriptContext* script_context);
        static CRegisterGlobal* head;
        CRegisterGlobal* next;

        const char* name;
        TinScript::eVarType type;
        void* addr;
};

bool8 AssertHandled(const char* condition, const char* file, int32 linenumber,
                    const char* fmt, ...);

// ------------------------------------------------------------------------------------------------
class CScriptContext {

    public:
        // -- static constructor/destructor, to create without having to directly use allocators
        static CScriptContext* Create(const char* thread_name = NULL,
                                      TinPrintHandler printhandler = NULL,
                                      TinAssertHandler asserthandler = NULL);
        static void Destroy(CScriptContext* script_context);
        static CScriptContext* FindThreadContext(const char* thread_name);
        static CScriptContext* GetMainThreadContext();

        CScriptContext(const char* thread_name = NULL, TinPrintHandler printhandler = NULL,
                       TinAssertHandler asserthandler = NULL);
        void CScriptContext::InitializeDictionaries();

        virtual ~CScriptContext();
        void ShutdownDictionaries();

        void Update(uint32 curtime);

        static CCodeBlock* CompileScript(CScriptContext* script_context, const char* filename);
        bool8 ExecScript(const char* filename);

        CCodeBlock* CompileCommand(const char* filename);
        bool8 ExecCommand(const char* statement);

        TinPrintHandler GetPrintHandler() { return (mTinPrintHandler); }
        TinAssertHandler GetAssertHandler() { return (mTinAssertHandler); }
        bool8 IsAssertEnableTrace() { return (mAssertEnableTrace); }
        void SetAssertEnableTrace(bool8 torf) { mAssertEnableTrace = torf; }
        bool8 IsAssertStackSkipped() { return (mAssertStackSkipped); }
        void SetAssertStackSkipped(bool8 torf) { mAssertStackSkipped = torf; }
        void ResetAssertStack();

        CNamespace* GetGlobalNamespace() {
            return (mGlobalNamespace);
        }

        CStringTable* GetStringTable() {
            return (mStringTable);
        }

        CHashTable<CCodeBlock>* GetCodeBlockList() {
            return (mCodeBlockList);
        }

        CScheduler* GetScheduler() {
            return (mScheduler);
        }

        CHashTable<CNamespace>* GetNamespaceDictionary() {
            return (mNamespaceDictionary);
        }

        CHashTable<CObjectEntry>* GetObjectDictionary() {
            return (mObjectDictionary);
        }

        CHashTable<CObjectEntry>* GetAddressDictionary() {
            return (mAddressDictionary);
        }

        CHashTable<CObjectEntry>* GetNameDictionary() {
            return (mNameDictionary);
        }

        CNamespace* FindOrCreateNamespace(const char* _nsname, bool create);
        CNamespace* FindNamespace(uint32 nshash);
        void LinkNamespaces(const char* parentnsname, const char* childnsname);
        void LinkNamespaces(CNamespace* parentns, CNamespace* childns);

        static uint32 GetNextObjectID();
        uint32 CreateObject(uint32 classhash, uint32 objnamehash);
        uint32 RegisterObject(void* objaddr, const char* classname, const char* objectname);
        void DestroyObject(uint32 objectid);

        bool8 IsObject(uint32 objectid);
        void* FindObject(uint32 objectid);

        CObjectEntry* FindObjectByAddress(void* addr);
        CObjectEntry* FindObjectByName(const char* objname);
        CObjectEntry* FindObjectEntry(uint32 objectid);
        uint32 FindIDByAddress(void* addr);

        void AddDynamicVariable(uint32 objectid, uint32 varhash,
                                       eVarType vartype);
        void AddDynamicVariable(uint32 objectid, const char* varname,
                                       const char* vartypename);
        void ListObjects();

        static CHashTable<CScriptContext>* GetScriptContextList() {
            return (gScriptContextList);
        }

    private:
        static CScriptContext* gMainThreadContext;
        static CHashTable<CScriptContext>* gScriptContextList;

        // -- hash so we can find this context by name
        uint32 mHash;

        // -- assert/print handlers
        TinPrintHandler mTinPrintHandler;
        TinAssertHandler mTinAssertHandler;
        bool8 mAssertEnableTrace;
        bool8 mAssertStackSkipped;

        // -- global namespace for this context
        CNamespace* mGlobalNamespace;

        // -- context stringtable 
        CStringTable* mStringTable;

        // -- context codeblock list
        CHashTable<CCodeBlock>* mCodeBlockList;

        // -- context namespace dictionaries
        CHashTable<CNamespace>* mNamespaceDictionary;
        CHashTable<CObjectEntry>* mObjectDictionary;
        CHashTable<CObjectEntry>* mAddressDictionary;
        CHashTable<CObjectEntry>* mNameDictionary;

        // -- context scheduler
        CScheduler* mScheduler;
};

}  // TinScript

#endif // __TINSCRIPT_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
