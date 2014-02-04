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
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "windows.h"
#include "conio.h"

#include "integration.h"

#include "TinHash.h"
#include "TinParse.h"
#include "TinCompile.h"
#include "TinExecute.h"
#include "TinNamespace.h"
#include "TinScheduler.h"
#include "TinObjectGroup.h"
#include "TinStringTable.h"
#include "TinRegistration.h"

#include "TinScript.h"

namespace TinScript {

// --  statics --------------------------------------------------------------------------------------------------------
static const char* gStringTableFileName = "stringtable.txt";

// -- this is a *thread* variable, each thread can reference a separate context
_declspec(thread) CScriptContext* gThreadContext = NULL;

// == Interface implementation ========================================================================================

// ====================================================================================================================
// CreateContext():  Creates a singleton context, max of one for each thread
// ====================================================================================================================
CScriptContext* CreateContext(TinPrintHandler printhandler, TinAssertHandler asserthandler, bool is_main_thread)
{
    CScriptContext* script_context = CScriptContext::Create(printhandler, asserthandler, is_main_thread);
    return (script_context);
}

// ====================================================================================================================
// UpdateContext():  Updates the singleton context in the calling thread
// ====================================================================================================================
void UpdateContext(uint32 current_time_msec)
{
    CScriptContext* script_context = GetContext();
    assert(script_context != NULL);
    script_context->Update(current_time_msec);
}

// ====================================================================================================================
// DestroyContext():  Destroys the context created from the calling thread
// ====================================================================================================================
void DestroyContext()
{
    CScriptContext::Destroy();
}

// ====================================================================================================================
// GetContext():  Uses a thread local global var to return the specific context created from this thread
// ====================================================================================================================
CScriptContext* GetContext()
{
    return (gThreadContext);
}

// ====================================================================================================================
// ExecCommand():  Executes a text block of valid script code
// ====================================================================================================================
bool8 ExecCommand(const char* statement)
{
    CScriptContext* script_context = GetContext();
    assert(script_context != NULL);
    return (script_context->ExecCommand(statement));
}

// ====================================================================================================================
// CompileScript():  Compiles (without executing) a text file containing script code
// ====================================================================================================================
bool8 CompileScript(const char* filename)
{
    CScriptContext* script_context = GetContext();
    assert(script_context != NULL);
    CCodeBlock* codeblock = script_context->CompileScript(filename);
    return (codeblock != NULL);
}

// ====================================================================================================================
// ExecScript():  Executes a text file containing script code
// ====================================================================================================================
bool8 ExecScript(const char* filename)
{
    CScriptContext* script_context = GetContext();
    assert(script_context != NULL);
    return (script_context->ExecScript(filename));
}

REGISTER_FUNCTION_P1(Compile, CompileScript, bool8, const char*);
REGISTER_FUNCTION_P1(Exec, ExecScript, bool8, const char*);

// ====================================================================================================================
// NullAssertHandler():  Default assert handler called, if one isn't provided
// ====================================================================================================================
bool8 NullAssertHandler(CScriptContext*, const char*, const char*, int32, const char*, ...)
{
    return false;
}

// ====================================================================================================================
// NullAssertHandler():  Default assert handler called, if one isn't provided
// ====================================================================================================================
int NullPrintHandler(const char*, ...) {
    return (0);
}

// == CScriptContext ==================================================================================================

// ====================================================================================================================
// ResetAssertStack():  Allows the next assert to trace it's own (error) path
// ====================================================================================================================
void CScriptContext::ResetAssertStack()
{
    mAssertEnableTrace = false;
    mAssertStackSkipped = false;
}

// ====================================================================================================================
// Create():  Static interface - only one context per thread
// ====================================================================================================================
CScriptContext* CScriptContext::Create(TinPrintHandler printhandler, TinAssertHandler asserthandler,
                                       bool is_main_thread)
{
    // -- only one script context per thread
    if (gThreadContext != NULL)
    {
        assert(gThreadContext == NULL);
        return (gThreadContext);
    }

    // -- set the thread context
    TinAlloc(ALLOC_ScriptContext, CScriptContext, printhandler, asserthandler, is_main_thread);
    return (gThreadContext);
}

// ====================================================================================================================
// Destroy():  Destroys the context singleton specific to the calling thread
// ====================================================================================================================
void CScriptContext::Destroy()
{
    if (gThreadContext)
    {
        TinFree(gThreadContext);
        gThreadContext = NULL;
    }
}

// --------------------------------------------------------------------------------------------------------------------
// Constructor()
// --------------------------------------------------------------------------------------------------------------------
CScriptContext::CScriptContext(TinPrintHandler printfunction, TinAssertHandler asserthandler, bool is_main_thread) {

    // -- initialize and populate the string table
    mStringTable = TinAlloc(ALLOC_StringTable, CStringTable, this, kStringTableSize);
    LoadStringTable(this);

    // -- set the flag
    mIsMainThread = is_main_thread;

    // -- initialize the ID generator
    mObjectIDGenerator = 0;

    // -- set the thread local singleton
    gThreadContext = this;

    // -- ensure our types have all been initialized - only from the main thread
    // -- this will set up global tables of type info... convert functions, op overrides, etc...
    if (is_main_thread)
    {
        InitializeTypes();
    }

    // -- set the handlers
    mTinPrintHandler = printfunction ? printfunction : NullPrintHandler;
    mTinAssertHandler = asserthandler ? asserthandler : NullAssertHandler;
    mAssertStackSkipped = false;
    mAssertEnableTrace = false;

    // -- initialize the namespaces dictionary, and all object dictionaries
    InitializeDictionaries();

    // -- create the global namespace for this context
    mGlobalNamespace = FindOrCreateNamespace(NULL, true);

    // -- register functions, each to their namespace
    CRegFunctionBase* regfunc = CRegFunctionBase::gRegistrationList;
    while(regfunc != NULL) {
        regfunc->Register(this);
        regfunc = regfunc->GetNext();
    }

    // -- register globals
    CRegisterGlobal::RegisterGlobals(this);

    // -- initialize the scheduler
    mScheduler = TinAlloc(ALLOC_SchedCmd, CScheduler, this);

    // -- initialize the master object list
    mMasterMembershipList = TinAlloc(ALLOC_ObjectGroup, CMasterMembershipList, this,
                                     kMasterMembershipTableSize);

    // -- initialize the code block hash table
    mCodeBlockList = TinAlloc(ALLOC_HashTable, CHashTable<CCodeBlock>, kGlobalFuncTableSize);

    // -- initialize the scratch buffer index
    mScratchBufferIndex = 0;

    // -- register the debugger interface separately
    mBreakpointCallback = NULL;
    mCallstackCallback = NULL;
    mCodeblockLoadedCallback = NULL;
    mWatchVarEntryCallback = NULL;

    mDebuggerBreakStep = false;
    mDebuggerLastBreak = -1;
}

void CScriptContext::InitializeDictionaries() {

    // -- allocate the dictinary to store creation functions
    mNamespaceDictionary = TinAlloc(ALLOC_HashTable, CHashTable<CNamespace>, kGlobalFuncTableSize);

    // -- allocate the dictionary to store the address of all objects created from script.
    mObjectDictionary = TinAlloc(ALLOC_HashTable, CHashTable<CObjectEntry>, kObjectTableSize);
    mAddressDictionary = TinAlloc(ALLOC_HashTable, CHashTable<CObjectEntry>, kObjectTableSize);
    mNameDictionary = TinAlloc(ALLOC_HashTable, CHashTable<CObjectEntry>, kObjectTableSize);

    // $$$TZA still working on how we're going to handle different threads
    // -- for now, every thread populates its dictionaries from the same list of registered objects
    CNamespaceReg* tempptr = CNamespaceReg::head;
    while (tempptr)
    {
        tempptr->SetRegistered(false);
        tempptr = tempptr->next;
    }

    // -- register the namespace - these are the namespaces
    // -- registered from code, so we need to populate the NamespaceDictionary,
    // -- and register the members/methods
    // -- note, because we register class derived from parent, we need to
    // -- iterate and ensure parents are always registered before children
    while (true)
    {
        CNamespaceReg* found_unregistered = NULL;
        bool8 abletoregister = false;
        CNamespaceReg* regptr = CNamespaceReg::head;
        while(regptr) {

            // -- see if this namespace is already registered
            if (regptr->GetRegistered()) {
                regptr = regptr->GetNext();
                continue;
            }

            // -- there's at least one namespace awaiting registration
            found_unregistered = regptr;

            // -- see if this namespace still requires its parent to be registered
            static const uint32 nullparenthash = Hash("VOID");
            CNamespace* parentnamespace = NULL;
            if (regptr->GetParentHash() != nullparenthash)
            {
                parentnamespace = mNamespaceDictionary->FindItem(regptr->GetParentHash());
                if (!parentnamespace) {
                    // -- skip this one, and wait until the parent is registered
                    regptr = regptr->GetNext();
                    continue;
                }
            }

            // -- set the bool8 to track that we're actually making progress
            abletoregister = true;

            // -- ensure the namespace doesn't already exist
            CNamespace* namespaceentry = mNamespaceDictionary->FindItem(regptr->GetHash());
            if (namespaceentry == NULL) {
                // -- create the namespace
                CNamespace* newnamespace = TinAlloc(ALLOC_Namespace, CNamespace,
                                                    this, regptr->GetName(),
                                                    regptr->GetTypeID(),
                                                    regptr->GetCreateFunction(),
                                                    regptr->GetDestroyFunction());

                // -- add the creation method to the hash dictionary
                mNamespaceDictionary->AddItem(*newnamespace, regptr->GetHash());

                // -- link this namespace to its parent
                if (parentnamespace) {
                    LinkNamespaces(newnamespace, parentnamespace);
                }

                // -- call the class registration method, to register members/methods
                regptr->RegisterNamespace(this, newnamespace);
                regptr->SetRegistered(true);
            }
            else {
                ScriptAssert_(this, 0, "<internal>", -1,
                              "Error - Namespace already created: %s\n",
                              UnHash(regptr->GetHash()));
                return;
            }

            regptr = regptr->GetNext();
        }

        // -- we'd better have registered at least one namespace, otherwise we're stuck
        if (found_unregistered && !abletoregister) {
            ScriptAssert_(this, 0, "<internal>", -1,
                          "Error - Unable to register Namespace: %s\n",
                          UnHash(found_unregistered->GetHash()));
            return;
        }

        // -- else see if we're done
        else if (!found_unregistered) {
            break;
        }
    }
}

CScriptContext::~CScriptContext() {
    // -- cleanup the namespace context
    // -- note:  the global namespace is owned by the namespace dictionary
    // -- within the context - it'll be automatically cleaned up
    ShutdownDictionaries();
    
    // -- cleanup all related codeblocks
    // -- by deleting the namespace dictionaries, all codeblocks should now be unused
    CCodeBlock::DestroyUnusedCodeBlocks(mCodeBlockList);
    assert(mCodeBlockList->IsEmpty());
    TinFree(mCodeBlockList);

    // -- clean up the scheduleer
    TinFree(mScheduler);

    // -- cleanup the membership list
    TinFree(mMasterMembershipList);

    // -- clean up the string table
    TinFree(mStringTable);

    // -- if this is the MainThread context, shutdown types
    if (mIsMainThread)
    {
        ShutdownTypes();
    }
}

void CScriptContext::ShutdownDictionaries() {

    // -- delete the Namespace dictionary
    if (mNamespaceDictionary) {
        mNamespaceDictionary->DestroyAll();
        TinFree(mNamespaceDictionary);
    }

    // -- delete the Object dictionaries
    if (mObjectDictionary) {
        mObjectDictionary->DestroyAll();
        TinFree(mObjectDictionary);
    }

    // -- objects will have been destroyed above, so simply clear this hash table
    if (mAddressDictionary) {
        mAddressDictionary->RemoveAll();
        TinFree(mAddressDictionary);
    }
    if (mNameDictionary) {
        mNameDictionary->RemoveAll();
        TinFree(mNameDictionary);
    }
}

void CScriptContext::Update(uint32 curtime) {
    mScheduler->Update(curtime);

    // $$$TZA This doesn't need to happen every frame...
    CCodeBlock::DestroyUnusedCodeBlocks(mCodeBlockList);
}

// ------------------------------------------------------------------------------------------------
uint32 Hash(const char *string, int32 length, bool add_to_table) {
	if (!string || !string[0])
		return 0;
    const char* s = string;
	int32 remaining = length;

	uint32 h = 5381;
	for (uint8 c = *s; c != '\0' && remaining != 0; c = *++s) {
		--remaining;

#if !CASE_SENSITIVE
        // -- if we're using this language as case insensitive, ensure the character is lower case
        if (c >= 'A' && c <= 'Z')
            c = 'z' + (c - 'A');
#endif

		h = ((h << 5) + h) + c;
	}

    // $$$TZA this should only happen in a DEBUG build
    // $$$TZA This is also not thread safe - only the main thread should be allowed to populate the
    // -- the string dictionary
    if (TinScript::GetContext() &&
        TinScript::GetContext()->GetStringTable())
    {
        TinScript::GetContext()->GetStringTable()->AddString(string, length, h, add_to_table);
    }

	return h;
}

uint32 HashAppend(uint32 h, const char *string, int32 length) {
	if (!string || !string[0])
		return h;
    const char* s = string;
	int32 remaining = length;

	for (uint8 c = *s; c != '\0' && remaining != 0; c = *++s) {
		--remaining;
		h = ((h << 5) + h) + c;
	}
	return h;
}

const char* UnHash(uint32 hash) {
    const char* string =
        TinScript::GetContext()->GetStringTable()->FindString(hash);
    if (!string || !string[0]) {
        static char buffers[8][20];
        static int32 bufindex = -1;
        bufindex = (bufindex + 1) % 8;
        sprintf_s(buffers[bufindex], 20, "<hash:0x%08x>", hash);
        return buffers[bufindex];
    }
    else
        return string;
}

void SaveStringTable(CScriptContext* script_context) {

    if (!script_context)
        return;

    const CHashTable<CStringTable::tStringEntry>* string_table =
        script_context->GetStringTable()->GetStringDictionary();
    if (!string_table)
        return;

  	// -- open the file
	FILE* filehandle = NULL;
	int32 result = fopen_s(&filehandle, gStringTableFileName, "wb");
	if (result != 0) {
        ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to write file %s\n",
                      gStringTableFileName);
		return;
    }

	if (!filehandle) {
        ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to write file %s\n",
                      gStringTableFileName);
		return;
    }

    for(int32 i = 0; i < string_table->Size(); ++i) {
	    CHashTable<CStringTable::tStringEntry>::CHashTableEntry* ste = string_table->FindRawEntryByBucket(i);
	    while (ste) {

            // -- only write out ref-counted strings (the remaining haven't been cleaned up)
            if (ste->item->mRefCount <= 0)
            {
                // -- next entry
       	        ste = string_table->GetNextRawEntryInBucket(i);
                continue;
            }

            uint32 stringhash = ste->hash;
            const char* string = ste->item->mString;
            int32 length = (int32)strlen(string);
            char tempbuf[kMaxTokenLength];

            // -- write the hash
            sprintf_s(tempbuf, kMaxTokenLength, "0x%08x: ", stringhash);
            int32 count = (int32)fwrite(tempbuf, sizeof(char), 12, filehandle);
            if (count != 12) {
                fclose(filehandle);
                ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to write file %s\n",
                              gStringTableFileName);
                return;
            }

            // -- write the string length
            sprintf_s(tempbuf, kMaxTokenLength, "%04d: ", length);
            count = (int32)fwrite(tempbuf, sizeof(char), 6, filehandle);
            if (count != 6) {
                fclose(filehandle);
                ScriptAssert_(script_context, 0, "<internal>", -1,
                              "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- write the string
            count = (int32)fwrite(string, sizeof(char), length, filehandle);
            if (count != length) {
                fclose(filehandle);
                ScriptAssert_(script_context, 0, "<internal>", -1,
                              "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- write the eol
            count = (int32)fwrite("\r\n", sizeof(char), 2, filehandle);
            if (count != 2) {
                fclose(filehandle);
                ScriptAssert_(script_context, 0, "<internal>", -1,
                              "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- next entry
       	    ste = string_table->GetNextRawEntryInBucket(i);
	    }
    }

    // -- close the file before we leave
	fclose(filehandle);
}

void LoadStringTable(CScriptContext* script_context) {

  	// -- open the file
	FILE* filehandle = NULL;
	int32 result = fopen_s(&filehandle, gStringTableFileName, "rb");
	if (result != 0) {
		return;
    }

	if (!filehandle) {
        ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to write file %s\n",
                      gStringTableFileName);
		return;
    }

    while(!feof(filehandle)) {

        // -- read the hash
        uint32 hash = 0;
        int32 length = 0;
        char string[kMaxTokenLength];
        char tempbuf[16];

        // -- read the hash
        int32 count = (int32)fread(tempbuf, sizeof(char), 12, filehandle);
        if (ferror(filehandle) || count != 12) {
            // -- we're done
            break;
        }
        tempbuf[12] = '\0';
        sscanf_s(tempbuf, "0x%08x: ", &hash);

        // -- read the string length
        count = (int32)fread(tempbuf, sizeof(char), 6, filehandle);
        if (ferror(filehandle) || count != 6) {
            fclose(filehandle);
            ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to read file: %s\n",
                          gStringTableFileName);
            return;
        }
        tempbuf[count] = '\0';
        sscanf_s(tempbuf, "%04d: ", &length);


        // -- read the string
        count = (int32)fread(string, sizeof(char), length, filehandle);
        if (ferror(filehandle) || count != length) {
            fclose(filehandle);
            ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to read file: %s\n",
                          gStringTableFileName);
            return;
        }
        string[length] = '\0';

        // -- read the eol
        count = (int32)fread(tempbuf, sizeof(char), 2, filehandle);
        if (ferror(filehandle) || count != 2) {
            fclose(filehandle);
            ScriptAssert_(script_context, 0, "<internal>", -1, "Error - unable to read file: %s\n",
                          gStringTableFileName);
            return;
        }

        // -- add the string to the table
        script_context->GetStringTable()->AddString(string, length, hash);
    }

    // -- close the file before we leave
	fclose(filehandle);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// helper functions

bool8 GetLastWriteTime(const char* filename, FILETIME& writetime)
{
    if (!filename || !filename[0])
        return false;

    // -- convert the filename to a wchar_t array
    int32 length = (int32)strlen(filename);
    wchar_t wfilename[kMaxNameLength];
    for(int32 i = 0; i < length + 1; ++i)
        wfilename[i] = filename[i];

    HANDLE hFile = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    // Retrieve the file times for the file.
    FILETIME ftCreate, ftAccess;
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &writetime))
        return false;

    return true;
}

bool8 GetBinaryFileName(const char* filename, char* binfilename, int32 maxnamelength) {
    if (!filename)
        return false;

    // -- a script file should end in ".ts"
    const char* extptr = strrchr(filename, '.');
    if (!extptr || Strncmp_(extptr, ".ts", 4) != 0)
        return false;

    // -- copy the root name
    uint32 length = kPointerDiffUInt32(extptr, filename);
    SafeStrcpy(binfilename, filename, maxnamelength);
    SafeStrcpy(&binfilename[length], ".tso", maxnamelength - length);

    return true;
}

bool8 NeedToCompile(const char* filename, const char* binfilename) {

#if FORCE_COMPILE
    return true;
#else

    // -- get the filetime for the original script
    // -- if fail, then we have nothing to compile
    FILETIME scriptft;
    if (!GetLastWriteTime(filename, scriptft))
        return false;

    // -- get the filetime for the binary file
    // -- if fail, we need to compile
    FILETIME binft;
    if (!GetLastWriteTime(binfilename, binft))
        return true;

    // -- if the binft is more recent, then we don't need to compile
    if (CompareFileTime(&binft, &scriptft) < 0)
        return true;
    else
        return false;
#endif
}

// ------------------------------------------------------------------------------------------------
CCodeBlock* CScriptContext::CompileScript(const char* filename) {

    // -- get the name of the output binary file
    char binfilename[kMaxNameLength];
    if (!GetBinaryFileName(filename, binfilename, kMaxNameLength)) {
        ScriptAssert_(this, 0, "<internal>", -1, "Error - invalid script filename: %s\n",
                      filename ? filename : "");
        return NULL;
    }

    // -- compile the source
    CCodeBlock* codeblock = ParseFile(this, filename);
    if (codeblock == NULL) {
        ScriptAssert_(this, 0, "<internal>", -1, "Error - unable to parse file: %s\n",
                      filename);
        return NULL;
    }

    // -- write the binary
    if (!SaveBinary(codeblock, binfilename))
        return NULL;

    // -- save the string table - *if* we're the main thread
    if (mIsMainThread)
        SaveStringTable(this);

    // -- reset the assert stack
    ResetAssertStack();

    return codeblock;
}

bool8 CScriptContext::ExecScript(const char* filename) {
    char binfilename[kMaxNameLength];
    if (!GetBinaryFileName(filename, binfilename, kMaxNameLength)) {
        ScriptAssert_(this, 0, "<internal>", -1, "Error - invalid script filename: %s\n",
                      filename ? filename : "");
        ResetAssertStack();
        return false;
    }

    CCodeBlock* codeblock = NULL;

    bool8 needtocompile = NeedToCompile(filename, binfilename);
    if (needtocompile) {
        codeblock = CompileScript(filename);
        if (!codeblock) {
            ResetAssertStack();
            return false;
        }
    }
    else {
        codeblock = LoadBinary(this, binfilename);
    }

    // -- notify the debugger
    // $$$TZA in a connected tool, we actually need to wait until the tool has a chance
    // -- to send the list of breakpoints for this codeblock
    if (codeblock) {
        NotifyCodeblockLoaded(codeblock->GetFilenameHash());
    }

    // -- execute the codeblock
    bool8 result = true;
    if (codeblock) {
	    bool8 result = ExecuteCodeBlock(*codeblock);
        codeblock->SetFinishedParsing();

        if (!result) {
            ScriptAssert_(this, 0, "<internal>", -1,
                          "Error - unable to execute file: %s\n", filename);
            result = false;
        }
        else if (!codeblock->IsInUse()) {
            CCodeBlock::DestroyCodeBlock(codeblock);
        }
    }

    ResetAssertStack();
    return result;
}

// ------------------------------------------------------------------------------------------------
CCodeBlock* CScriptContext::CompileCommand(const char* statement) {

    CCodeBlock* commandblock = ParseText(this, "<stdin>", statement);
    return commandblock;
}

bool8 CScriptContext::ExecCommand(const char* statement) {
    CCodeBlock* stmtblock = CompileCommand(statement);
    if (stmtblock) {
        bool8 result = ExecuteCodeBlock(*stmtblock);
        stmtblock->SetFinishedParsing();

        ResetAssertStack();

        // -- if the codeblock didn't define any functions, we're finished with it
        if (!stmtblock->IsInUse())
            CCodeBlock::DestroyCodeBlock(stmtblock);
        return result;
    }
    else {
        ScriptAssert_(this, 0, "<internal>", -1, "Error - Unable to compile: %s\n",
                      statement);
    }

    ResetAssertStack();

    // -- failed
    return false;
}

// ------------------------------------------------------------------------------------------------
void CScriptContext::SetFunctionReturnValue(void* value, eVarType valueType)
{
    // -- if the current value is a string, we do need to update the refcount
    if (mFunctionReturnValType == TYPE_string)
    {
        uint32 string_hash = *(uint32*)mFunctionReturnValue;
        GetStringTable()->RefCountDecrement(string_hash);
    }

    // -- sanity check
    if (!value || valueType < FIRST_VALID_TYPE)
    {
        mFunctionReturnValType = TYPE_NULL;
    }
    else
    {
        mFunctionReturnValType = valueType;
        memcpy(mFunctionReturnValue, value, kMaxTypeSize);

        // -- update the string table for strings
        if (mFunctionReturnValType == TYPE_string)
        {
            uint32 string_hash = *(uint32*)mFunctionReturnValue;
            GetStringTable()->RefCountIncrement(string_hash);
        }
    }
}

bool8 CScriptContext::GetFunctionReturnValue(void*& value, eVarType& valueType)
{
    if (mFunctionReturnValType >= FIRST_VALID_TYPE)
    {
        value = mFunctionReturnValue;
        valueType = mFunctionReturnValType;
        return (true);
    }
    else
    {
        value = NULL;
        valueType = TYPE_NULL;
        return (false);
    }
}

char* CScriptContext::GetScratchBuffer()
{
    char* scratch_buffer = mScratchBuffers[(++mScratchBufferIndex) % kMaxScratchBuffers];
    return (scratch_buffer);
}

// ------------------------------------------------------------------------------------------------
// -- debugger interface
void CScriptContext::SetBreakpointCallback(DebuggerBreakpointHit breakpoint_callback) {
    mBreakpointCallback = breakpoint_callback;
}

void CScriptContext::SetCallstackCallback(DebuggerCallstackFunc callstack_callback) {
    mCallstackCallback = callstack_callback;
}

void CScriptContext::SetCodeblockLoadedCallback(CodeblockLoadedFunc codeblock_callback) {
    mCodeblockLoadedCallback = codeblock_callback;
}

void CScriptContext::SetWatchVarEntryCallback(DebuggerWatchVarFunc watch_var_callback) {
    mWatchVarEntryCallback = watch_var_callback;
}

bool8 CScriptContext::NotifyBreakpointHit(uint32 codeblock_hash, int32& line_number) {

    // -- no debugger registered - return true - allows us to keep running
    if (!mBreakpointCallback)
        return (true);

    // -- otherwise, let the callback determine if we should run or step
    else {
        bool8 continue_run = mBreakpointCallback(codeblock_hash, line_number);
        return (continue_run);
    }
}

void CScriptContext::NotifyCallstack(uint32* codeblock_array, uint32* objid_array,
                                     uint32* namespace_array,uint32* func_array,
                                     uint32* linenumber_array, int array_size) {
    if (!mCallstackCallback)
        return;
    mCallstackCallback(codeblock_array, objid_array, namespace_array, func_array,
                       linenumber_array, array_size);
}

void CScriptContext::NotifyCodeblockLoaded(uint32 codeblock_hash) {
    if (mCodeblockLoadedCallback) {
        mCodeblockLoadedCallback(codeblock_hash);
    }
}

void CScriptContext::NotifyWatchVarEntry(CDebuggerWatchVarEntry* watch_var_entry) {
    if (!mWatchVarEntryCallback)
        return;
    mWatchVarEntryCallback(watch_var_entry);
}

// $$$TZA when the debugger becomes remote, do we want debugging on just the MainThread?
void CScriptContext::RegisterDebugger(DebuggerBreakpointHit breakpoint_func,
                                      DebuggerCallstackFunc callstack_func,
                                      CodeblockLoadedFunc codeblock_func,
                                      DebuggerWatchVarFunc watch_var_func) {
    CScriptContext* script_context = GetContext();
    if (script_context) {
        script_context->SetBreakpointCallback(breakpoint_func);
        script_context->SetCallstackCallback(callstack_func);
        script_context->SetCodeblockLoadedCallback(codeblock_func);
        script_context->SetWatchVarEntryCallback(watch_var_func);
    }
}

int32 CScriptContext::AddBreakpoint(const char* filename, int32 line_number)
{
    // -- sanity check
    if (!filename || !filename[0])
        return (-1);

    CScriptContext* script_context = GetContext();
    if (!script_context) {
        return (-1);
    }

    // -- find the code block within the thread
    uint32 filename_hash = Hash(filename);
    CCodeBlock* code_block = script_context->GetCodeBlockList()->FindItem(filename_hash);
    if (! code_block) {
        return (-1);
    }

    // -- add the breakpoint
    return (code_block->AddBreakpoint(line_number));
}

int32 CScriptContext::RemoveBreakpoint(const char* filename, int32 line_number)
{
    // -- sanity check
    if (!filename || !filename[0])
        return (-1);

    CScriptContext* script_context = GetContext();
    if (!script_context) {
        return (-1);
    }

    // -- find the code block within the thread
    uint32 filename_hash = Hash(filename);
    CCodeBlock* code_block = script_context->GetCodeBlockList()->FindItem(filename_hash);
    if (! code_block) {
        return (-1);
    }

    // -- add the breakpoint
    return (code_block->RemoveBreakpoint(line_number));
}

void CScriptContext::RemoveAllBreakpoints(const char* filename)
{
    // -- sanity check
    if (!filename || !filename[0])
        return;

    CScriptContext* script_context = GetContext();
    if (!script_context) {
        return;
    }

    // -- find the code block within the thread
    uint32 filename_hash = Hash(filename);
    CCodeBlock* code_block = script_context->GetCodeBlockList()->FindItem(filename_hash);
    if (! code_block) {
        return;
    }

    code_block->RemoveAllBreakpoints();
}

} // TinScript

// ------------------------------------------------------------------------------------------------
// -- generic object - has absolutely no functionality except to serve as something to
// -- instantiate, that you can name, and then implement methods on the namespace

class CScriptObject {
public:
    CScriptObject() { dummy = 0; }
    virtual ~CScriptObject() {}

    DECLARE_SCRIPT_CLASS(CScriptObject, VOID);

private:
    int64 dummy;
};

IMPLEMENT_SCRIPT_CLASS_BEGIN(CScriptObject, VOID)
IMPLEMENT_SCRIPT_CLASS_END()

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
