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
#define DECLARE_SCRIPT_CLASS(classname, parentclass)                                                    \
    static const char* GetParentName() { return #parentclass; }                                         \
    static const char* GetClassName() { return #classname; }                                            \
    static classname* Create() {                                                                        \
        classname* newobj = new classname();                                                            \
        return newobj;                                                                                  \
    }                                                                                                   \
    static void Destroy(void* addr) {                                                                   \
        if(addr) {                                                                                      \
            classname* obj = static_cast<classname*>(addr);                                             \
            delete obj;                                                                                 \
        }                                                                                               \
    }                                                                                                   \
    uint32 GetObjectID() const { return TinScript::CNamespace::FindIDByAddress((void*)this); }    \
    SCRIPT_DEFAULT_METHODS(classname);                                                                  \
    static void Register(TinScript::CNamespace* _classnamespace);                                       \
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
        uint32 varhash = TinScript::Hash(#scriptname);                                 \
        TinScript::CVariableEntry* ve = new TinScript::CVariableEntry(#scriptname, varhash,  \
            TinScript::GetRegisteredType(TinScript::GetTypeID(classptr->membername)), true,  \
            offsetof(classname, membername));                                                \
        classnamespace->GetVarTable()->AddItem(*ve, varhash);                                \
    }

#define REGISTER_GLOBAL_VAR(scriptname, var)                                                 \
    TinScript::CRegisterGlobal _reg_gv_##scriptname(#scriptname,                             \
        TinScript::GetRegisteredType(TinScript::GetTypeID(var)), (void*)&var);

// ------------------------------------------------------------------------------------------------
// constants

const int32 kCompilerVersion = 1;

const int32 kMaxArgs = 256;
const int32 kMaxArgLength = 256;

const int32 kGlobalFuncTableSize = 97;
const int32 kGlobalVarTableSize = 97;

const int32 kLocalFuncTableSize = 17;
const int32 kLocalVarTableSize = 17;

const int32 kFunctionCallStackSize = 32;

const int32 kStringTableSize = 32 * 1024;
const int32 kStringTableDictionarySize = 199;

const int32 kObjectTableSize = 10007;

#define kBytesToWordCount(a) ((a) + 3) / 4;

// ------------------------------------------------------------------------------------------------
#define ScriptAssert_(condition, file, linenumber, fmt, ...)                                    \
    {                                                                                           \
        if(!(condition)) {                                                                      \
            if(!TinScript::AssertHandled(#condition, file, linenumber, fmt, __VA_ARGS__)) {     \
                __asm   int 3                                                                   \
            }                                                                                   \
        }                                                                                       \
    }

namespace TinScript {

// ------------------------------------------------------------------------------------------------
// forward declarations

class CVariableEntry;
class CFunctionEntry;
class CNamespace;
class CCodeBlock;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

void SaveStringTable();
void LoadStringTable();

CNamespace* GetGlobalNamespace();

// --Global Var Registration-----------------------------------------------------------------------
class CRegisterGlobal {
    public:
        CRegisterGlobal(const char* _name = NULL, TinScript::eVarType _type = TinScript::TYPE_NULL,
                        void* _addr = NULL);
        virtual ~CRegisterGlobal() { }

        static void RegisterGlobals();
        static CRegisterGlobal* head;
        CRegisterGlobal* next;

        const char* name;
        TinScript::eVarType type;
        void* addr;
};

// ------------------------------------------------------------------------------------------------
// -- returns false if we should break
void ResetAssertStack();
nflag AssertHandled(const char* condition, const char* file, int32 linenumber, const char* fmt, ...);

// ------------------------------------------------------------------------------------------------
// external interface
void Initialize();
void Update(uint32 curtime);
void Shutdown();

CCodeBlock* CompileScript(const char* filename);
nflag ExecScript(const char* filename);

CCodeBlock* CompileCommand(const char* filename);
nflag ExecCommand(const char* statement);

nflag IsObject(uint32 objectid);
void* FindObject(uint32 objectid);

}  // TinScript



#endif // __TINSCRIPT_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
