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

// ====================================================================================================================
// TinScript.h  This is the main header file to be included by every source file requiring access to TinScript.
// ====================================================================================================================

#ifndef __TINSCRIPT_H
#define __TINSCRIPT_H

// -- includes
#include "stddef.h"
#include "stdarg.h"

#include "integration.h"
#include "TinTypes.h"
#include "TinNamespace.h"

// --------------------------------------------------------------------------------------------------------------------
// -- compile flags
#define DEBUG_CODEBLOCK 1
#define FORCE_COMPILE 1
#define CASE_SENSITIVE 1
#define DEBUG_TRACE 1
#define DEBUG_COMPILE_SYMBOLS 0
#define TIN_DEBUGGER 1

// --------------------------------------------------------------------------------------------------------------------
// -- only case_sensitive has been extensively tested, however theoretically TinScript should function as a
// -- insensitive language by affecting just a few functions, such as Hash().
#if CASE_SENSITIVE
    #define Strncmp_ strncmp
#else
    #define Strncmp_ _strnicmp
#endif

// ====================================================================================================================
// -- Registration macros
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
    SCRIPT_DEFAULT_METHODS(classname);                                                            \
    static void Register(::TinScript::CScriptContext* script_context, ::TinScript::CNamespace* _classnamespace);    \

#define IMPLEMENT_SCRIPT_CLASS_BEGIN(classname, parentname)                                             \
    ::TinScript::CNamespaceReg reg_##classname(#classname, #parentname, ::TinScript::GetTypeID<classname*>(), (void*)classname::Create,       \
                                             (void*)classname::Destroy, (void*)classname::Register);    \
    IMPLEMENT_DEFAULT_METHODS(classname);                                                               \
    void classname::Register(::TinScript::CScriptContext* script_context, ::TinScript::CNamespace* classnamespace)  \
    {                                                                                                   \
        Unused_(script_context);                                                                        \
        Unused_(classnamespace);

#define IMPLEMENT_SCRIPT_CLASS_END()    \
    }

#define REGISTER_MEMBER(classname, scriptname, membername)                                              \
    {                                                                                                   \
        classname* classptr = reinterpret_cast<classname*>(0);                                          \
        uint32 varhash = ::TinScript::Hash(#scriptname);                                                \
        ::TinScript::CVariableEntry* ve =                                                               \
            TinAlloc(ALLOC_VarEntry, ::TinScript::CVariableEntry, script_context, #scriptname, varhash, \
            ::TinScript::GetRegisteredType(::TinScript::GetTypeID(classptr->membername)),               \
            ::TinScript::IsArray(classptr->membername) ?                                                \
            (sizeof(classptr->membername) / ::TinScript::GetTypeSize(classptr->membername)) : 1, true,  \
            Offsetof_(classname, membername));                                                          \
        classnamespace->GetVarTable()->AddItem(*ve, varhash);                                           \
    }

#define REGISTER_GLOBAL_VAR(scriptname, var)                                                        \
    ::TinScript::CRegisterGlobal _reg_gv_##scriptname(#scriptname,                                  \
        ::TinScript::GetRegisteredType(::TinScript::GetTypeID(var)), (void*)&var,                   \
        ::TinScript::IsArray(var) ? (sizeof(var) / ::TinScript::GetTypeSize(var)) : 1)

#define DECLARE_FILE(filename) \
    bool8 g_##filename##_registered = false;

#define REGISTER_FILE(filename) \
    extern bool8 g_##filename##_registered; \
    g_##filename##_registered = true;


// ====================================================================================================================
// -- constants

const int32 kCompilerVersion = 1;

const int32 kMaxArgs = 256;
const int32 kMaxArgLength = 256;

const int32 kMaxVariableArraySize = 256;

const int32 kScriptContextThreadSize = 7;

const int32 kDebuggerCallstackSize = 32;
const int32 kDebuggerWatchWindowSize = 128;
const int32 kBreakpointTableSize = 17;

const int32 kGlobalFuncTableSize = 97;
const int32 kGlobalVarTableSize = 97;

const int32 kLocalFuncTableSize = 17;
const int32 kLocalVarTableSize = 17;

const int32 kFunctionCallStackSize = 2048;

const int32 kExecStackSize = 4096;
const int32 kExecFuncCallDepth = 2048;

const int32 kStringTableSize = 32 * 1024;
const int32 kStringTableDictionarySize = 199;

const int32 kObjectTableSize = 10007;

const int32 kMasterMembershipTableSize = 97;
const int32 kObjectGroupTableSize = 17;

const int32 kMaxScratchBuffers = 32;

const int32 kThreadExecBufferSize = 8 * 1024;

// ====================================================================================================================
// -- debugger constants
const int32 k_DebuggerCurrentWorkingDirPacketID     = 0x01;
const int32 k_DebuggerCodeblockLoadedPacketID       = 0x02;
const int32 k_DebuggerBreakpointHitPacketID         = 0x03;
const int32 k_DebuggerBreakpointConfirmPacketID     = 0x04;
const int32 k_DebuggerVarWatchConfirmPacketID       = 0x05;
const int32 k_DebuggerCallstackPacketID             = 0x06;
const int32 k_DebuggerWatchVarEntryPacketID         = 0x07;
const int32 k_DebuggerAssertMsgPacketID             = 0x08;
const int32 k_DebuggerPrintMsgPacketID              = 0x09;
const int32 k_DebuggerMaxPacketID                   = 0xff;

// == namespace TinScript =============================================================================================

namespace TinScript
{

// ====================================================================================================================
// forward declarations

class CVariableEntry;
class CFunctionEntry;
class CNamespace;
class CCodeBlock;
class CStringTable;
class CScheduler;
class CScriptContext;
class CObjectEntry;
class CMasterMembershipList;
class CFunctionCallStack;
class CExecStack;

typedef CHashTable<CVariableEntry> tVarTable;
typedef CHashTable<CFunctionEntry> tFuncTable;

const char* GetStringTableName();
void SaveStringTable(const char* filename = NULL);
void LoadStringTable(const char* filename = NULL);

// -- CThreadMutex is only functional in Win32
// ====================================================================================================================
// class CThreadMutex:  Prevents access to namespace objects from different threads
// ====================================================================================================================
class CThreadMutex
{
    public:
        CThreadMutex();
        void Lock();
        void Unlock();

    protected:
        void* mThreadMutex;
        bool8 mIsLocked;
};

// ====================================================================================================================
// class CRegisterGlobal:  Instantiated in the global namespace to stor info needed to register global variables.
// ====================================================================================================================
class CRegisterGlobal
{
    public:
        CRegisterGlobal(const char* _name = NULL, eVarType _type = TYPE_NULL, void* _addr = NULL,
                        int32 _array_size = 1);
        virtual ~CRegisterGlobal() { }

        static void RegisterGlobals(CScriptContext* script_context);
        static CRegisterGlobal* head;
        CRegisterGlobal* next;

        const char* name;
        TinScript::eVarType type;
        void* addr;
        int32 array_size;
};

bool8 AssertHandled(const char* condition, const char* file, int32 linenumber, const char* fmt, ...);

// ====================================================================================================================
// class CDebuggerWatchVarEntry:  Class used to send variable details to the debugger.
// ====================================================================================================================
class CDebuggerWatchVarEntry
{
    public:
		// -- watches that are part of the call stack are well defined.
		// -- watches that are dynamic user requests are "iffy", and
		// -- we'll use a request ID, if we're able to match a watch expression
		// -- with a type and value
		uint32 mWatchRequestID;
        int32 mStackLevel;

        // -- three members identifying the calling function
        uint32 mFuncNamespaceHash;
        uint32 mFunctionHash;
        uint32 mFunctionObjectID;

        // -- two members if this variable is an object member
        uint32 mObjectID;
        uint32 mNamespaceHash;

        // -- type, name, and value of variable/member
        // -- if the mType is void, and we have an objectID and namespace hash,
        // -- then we have a namespace label
        eVarType mType;
        int mArraySize;
        char mVarName[kMaxNameLength];
        char mValue[kMaxNameLength];

        // -- cached values for the hash of the VarName, and the var objectID (for TYPE_object vars)
        uint32 mVarHash;
        uint32 mVarObjectID;
};

// ====================================================================================================================
// class CDebuggerWatchExpression:  Holds the string expression, and the codeblock and function entry
// so repeated evaluation is efficient
// ====================================================================================================================
class CDebuggerWatchExpression
{
    public:
        CDebuggerWatchExpression(bool8 isConditional, bool break_enabled, const char* expression, const char* trace,
                                 bool8 trace_on_condition);
        ~CDebuggerWatchExpression();

        void SetAttributes(bool8 break_enabled, const char* conditional, const char* trace, bool8 trace_on_condition);

        static int gWatchExpressionID;
        bool8 mIsEnabled;
        bool8 mIsConditional;
        char mConditional[kMaxNameLength];
        char mTrace[kMaxNameLength];
        bool8 mTraceOnCondition;
        CFunctionEntry* mWatchFunctionEntry;
        CFunctionEntry* mTraceFunctionEntry;
};

// ====================================================================================================================
// class CScriptContext:  The singleton (per thread) root of all access to TinScript.
// ====================================================================================================================
class CScriptContext
{
    public:
        // -- static constructor/destructor, to create without having to directly use allocators
        static CScriptContext* Create(TinPrintHandler printhandler = NULL, TinAssertHandler asserthandler = NULL,
                                      bool is_main_thread = true);
        static void Destroy();

        void InitializeDictionaries();

        void ShutdownDictionaries();

        void Update(uint32 curtime);

        CCodeBlock* CompileScript(const char* filename);
        bool8 ExecScript(const char* filename);

        CCodeBlock* CompileCommand(const char* statement);
        bool8 ExecCommand(const char* statement);

        // -- if the command contains a function call, we need to be able to access the result
        void SetFunctionReturnValue(void* value, eVarType valueType);
        bool8 GetFunctionReturnValue(void*& value, eVarType& valueType);

        TinPrintHandler GetPrintHandler() { return (mTinPrintHandler); }
        TinAssertHandler GetAssertHandler() { return (mTinAssertHandler); }
        bool8 IsAssertEnableTrace() { return (mAssertEnableTrace); }
        void SetAssertEnableTrace(bool8 torf) { mAssertEnableTrace = torf; }
        bool8 IsAssertStackSkipped() { return (mAssertStackSkipped); }
        void SetAssertStackSkipped(bool8 torf) { mAssertStackSkipped = torf; }
        void ResetAssertStack();

        static const char* kGlobalNamespace;
        static uint32 kGlobalNamespaceHash;

        CNamespace* GetGlobalNamespace() { return (mGlobalNamespace); }
        CStringTable* GetStringTable() { return (mStringTable); }
        CHashTable<CCodeBlock>* GetCodeBlockList() { return (mCodeBlockList); }
        CScheduler* GetScheduler() { return (mScheduler); }
        CMasterMembershipList* GetMasterMembershipList() { return (mMasterMembershipList); }

        CHashTable<CNamespace>* GetNamespaceDictionary() { return (mNamespaceDictionary); }
        CHashTable<CObjectEntry>* GetObjectDictionary() { return (mObjectDictionary); }
        CHashTable<CObjectEntry>* GetAddressDictionary() { return (mAddressDictionary); }
        CHashTable<CObjectEntry>* GetNameDictionary() { return (mNameDictionary); }

        CNamespace* FindOrCreateNamespace(const char* _nsname, bool create);
        CNamespace* FindNamespace(uint32 nshash);
        void LinkNamespaces(const char* parentnsname, const char* childnsname);
        void LinkNamespaces(CNamespace* parentns, CNamespace* childns);

        uint32 GetNextObjectID();
        uint32 CreateObject(uint32 classhash, uint32 objnamehash);
        uint32 RegisterObject(void* objaddr, const char* classname, const char* objectname);
        void UnregisterObject(void* objaddr);
        void DestroyObject(uint32 objectid);

        bool8 IsObject(uint32 objectid);
        void* FindObject(uint32 objectid, const char* required_namespace = NULL);

        CObjectEntry* FindObjectByAddress(void* addr);
        CObjectEntry* FindObjectByName(const char* objname);
        CObjectEntry* FindObjectEntry(uint32 objectid);
        uint32 FindIDByAddress(void* addr);

        bool8 HasMethod(void* addr, const char* method_name);
        bool8 HasMethod(uint32 objectid, const char* method_name);

        bool8 AddDynamicVariable(uint32 objectid, uint32 varhash, eVarType vartype, int32 array_size = 1);
        bool8 AddDynamicVariable(uint32 objectid, const char* varname, const char* vartypename, int32 array_size = 1);
        bool8 SetMemberVar(uint32 objectid, const char* varname, void* value);

        void PrintObject(CObjectEntry* oe, int32 indent = 0);
        void ListObjects();

        // -- convenience buffer
        char* GetScratchBuffer();

        // -- debugger interface
        void SetDebuggerConnected(bool8 connected);
        bool IsDebuggerConnected(int32& debugger_session);
        void DebuggerNotifyAssert();
        void AddBreakpoint(const char* filename, int32 line_number, bool8 break_enabled, const char* conditional,
                           const char* trace, bool8 trace_on_condition);
        void RemoveBreakpoint(const char* filename, int32 line_number);
        void RemoveAllBreakpoints(const char* filename);
        void SetForceBreak(int32 watch_var_request_id);
        void SetBreakActionStep(bool8 torf, bool8 step_in = false, bool8 step_out = false);
        void SetBreakActionRun(bool8 torf);

		void InitWatchEntryFromVarEntry(CVariableEntry& ve, CObjectEntry* parent_oe,
                                        CDebuggerWatchVarEntry& watch_entry, CObjectEntry*& oe);
		void AddVariableWatch(int32 request_id, const char* expression, bool breakOnWrite, const char* new_value);

        bool8 HasWatchExpression(CDebuggerWatchExpression& debugger_watch);
        bool8 HasTraceExpression(CDebuggerWatchExpression& debugger_watch);
        bool8 InitWatchExpression(CDebuggerWatchExpression& debugger_watch, bool use_trace,
                                  CFunctionCallStack& call_stack);
        bool8 EvalWatchExpression(CDebuggerWatchExpression& debugger_watch, bool use_trace,
                                  CFunctionCallStack& cur_call_stack, CExecStack& cur_exec_stack);

        bool8 EvaluateWatchExpression(const char* expression, bool8 conditional);
		void ToggleVarWatch(int32 watch_request_id, uint32 object_id, uint32 var_name_hash, bool breakOnWrite,
                            const char* condition, const char* trace, bool8 trace_on_cond);

        // -- set the bool to indicate we're not stepping through each line in a debugger
		int32 mDebuggerSessionNumber;
        bool8 mDebuggerConnected;
        bool8 mDebuggerActionForceBreak;
        bool8 mDebuggerActionStep;
        bool8 mDebuggerActionStepOver;
        bool8 mDebuggerActionStepOut;
        bool8 mDebuggerActionRun;

        bool8 mDebuggerBreakLoopGuard;
		CFunctionCallStack* mDebuggerBreakFuncCallStack;
		CExecStack* mDebuggerBreakExecStack;
        int32 mDebuggerVarWatchRequestID;

        // -- communication with the debugger
        void DebuggerCurrentWorkingDir(const char* cwd);
        void DebuggerCodeblockLoaded(uint32 codeblock_hash);
        void DebuggerBreakpointHit(int32 watch_var_request_id, uint32 codeblock_hash, int32 line_number);
        void DebuggerBreakpointConfirm(uint32 codeblock_hash, int32 line_number, int32 actual_line_number);
        void DebuggerVarWatchConfirm(int32 request_id, uint32 watch_object_id, uint32 var_name_hash);
        void DebuggerSendCallstack(uint32* codeblock_array, uint32* objid_array,
                                   uint32* namespace_array,uint32* func_array,
                                   uint32* linenumber_array, int array_size);
        void DebuggerSendWatchVariable(CDebuggerWatchVarEntry* watch_var_entry);
        void DebuggerSendObjectMembers(CDebuggerWatchVarEntry* callingFunction, uint32 objectID);
        void DebuggerSendObjectVarTable(CDebuggerWatchVarEntry* callingFunction, CObjectEntry* oe, uint32 ns_hash,
                                        tVarTable* var_table);
        void DebuggerSendAssert(const char* assert_msg, uint32 codeblock_hash, int32 line_number);
        void DebuggerSendPrint(const char* fmt, ...);

        // -- methods to send object status updates to the debugger
        void DebuggerNotifyCreateObject(CObjectEntry* oe);
        void DebuggerNotifyDestroyObject(uint32 object_id);
        void DebuggerNotifySetAddObject(uint32 parent_id, uint32 object_id);
        void DebuggerNotifySetRemoveObject(uint32 parent_id, uint32 object_id);
        void DebuggerListObjects(uint32 parent_id, uint32 object_id);
        void DebuggerInspectObject(uint32 object_id);

        // -- useful debugging statics
        static bool8 gDebugParseTree;
        static bool8 gDebugCodeBlock;
        static bool8 gDebugTrace;
        
        // -- Thread commands are only supported in WIN32
        #ifdef WIN32
            void AddThreadCommand(const char* command);
            void ProcessThreadCommands();
        #endif // WIN32

    private:
        // -- use the static Create() method
        CScriptContext(TinPrintHandler printhandler = NULL, TinAssertHandler asserthandler = NULL,
                       bool is_main_thread = true);

        // -- use the static Destroy() method
        // -- not virtual - this is a final class
        ~CScriptContext();

        // -- in case we need to differentiate - likely only the main thread
        // -- will be permitted to write out the string dictionary
        bool mIsMainThread;
        uint32 mObjectIDGenerator;

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

        // -- when a script function returns (even void), a value is always pushed
        // -- if ExecF() calls a script function, we'll want to return that value to code
        char mFunctionReturnValue[kMaxTypeSize];
        eVarType mFunctionReturnValType;

        // -- used mostly for a place to do type conversions, this is a convenience
        // -- feature to avoid allocations, and to ensure that converted results
        // -- always have a reliable place to live.
        // -- if kMaxScratchBuffers is 32, then there would have to be
        // -- 32 type conversions in a single expression before we get buffer overrun...
        int32 mScratchBufferIndex;
        char mScratchBuffers[kMaxScratchBuffers][kMaxTokenLength];

        // -- master object list
        CMasterMembershipList* mMasterMembershipList;

        // -- buffer to store the results, when executing script commands from code
        char mExecfResultBuffer[kMaxArgLength];

        // -- We may need to queue script commands from a remote connection
        // -- which requires a thread lock
        // -- Thread commands are only supported in WIN32
        #ifdef WIN32
            CThreadMutex mThreadLock;
            char mThreadExecBuffer[kThreadExecBufferSize];
        #endif // WIN32
            char* mThreadBufPtr;
};

}  // TinScript

#endif // __TINSCRIPT_H

// ====================================================================================================================
// EOF
// ====================================================================================================================
