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

#include "TinHash.h"
#include "TinParse.h"
#include "TinCompile.h"
#include "TinExecute.h"
#include "TinNamespace.h"
#include "TinScheduler.h"
#include "TinStringTable.h"
#include "TinRegistration.h"

#include "TinScript.h"

// ------------------------------------------------------------------------------------------------
// statics - mostly for the quick and dirty console implementation
static const uint32 gFramesPerSecond = 33;
static const uint32 gMSPerFrame = 1000 / gFramesPerSecond;
static const real gSecPerFrame = (1.0f / real(gFramesPerSecond));
static uint32 gCurrentTime = 0;
static nflag gRefreshConsoleString = false;
static uint32 gRefreshConsoleTimestamp = 0;
static char gConsoleInputBuf[TinScript::kMaxTokenLength];
static const real gRefreshDelay = 0.25f;

namespace TinScript {

CNamespace* gGlobalNamespace = NULL;

CRegFunctionBase* CRegFunctionBase::gRegistrationList = NULL;

static const char* gStringTableFileName = "stringtable.txt";

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

void CRegisterGlobal::RegisterGlobals() {
    CRegisterGlobal* global = CRegisterGlobal::head;
    while(global) {
        // -- create the var entry, add it to the global namespace
        CVariableEntry* ve = new CVariableEntry(global->name, global->type, global->addr);
	    uint32 hash = ve->GetHash();
	    GetGlobalNamespace()->GetVarTable()->AddItem(*ve, hash);

        // -- next registration object
        global = global->next;
    }
}

// ------------------------------------------------------------------------------------------------
static nflag gAssertEnableTrace = false;
static nflag gAssertStackSkipped = false;
void ResetAssertStack() {
    gAssertEnableTrace = false;
    gAssertStackSkipped = false;
}

// -- returns false if we should break
nflag AssertHandled(const char* condition, const char* file, int32 linenumber, const char* fmt, ...) {
    if(!gAssertStackSkipped || gAssertEnableTrace) {
        if(!gAssertStackSkipped)
            printf("*************************************************************\n");
        else
            printf("\n");

        if(linenumber >= 0)
            printf("Assert(%s) file: %s, line %d:\n", condition, file, linenumber + 1);
        else
            printf("Exec Assert(%s):\n", condition);

        va_list args;
        va_start(args, fmt);
        char msgbuf[kMaxTokenLength];
        vsprintf_s(msgbuf, kMaxTokenLength, fmt, args);
        va_end(args);
        printf(msgbuf);

        if(!gAssertStackSkipped)
            printf("*************************************************************\n");
        if(!gAssertStackSkipped) {
            printf("Press 'b' to break, 't' to trace, otherwise skip...\n");
            char ch = getchar();
            if(ch == 'b')
                return false;
            else if(ch == 't') {
                gAssertStackSkipped = true;
                gAssertEnableTrace = true;
                return true;
            }
            else {
                gAssertStackSkipped = true;
                gAssertEnableTrace = false;
                return true;
            }
        }
    }

    // -- handled - return true so we don't break
    return true;
}

// ------------------------------------------------------------------------------------------------
void Initialize() {

    // -- initialize and populate the string table
    CStringTable::Initialize();
    LoadStringTable();

    // -- initialize the namespace
    CNamespace::Initialize();

    // -- create the global namespace
    gGlobalNamespace = CNamespace::FindOrCreateNamespace(NULL, true);

    // -- register functions, each to their namespace
    CRegFunctionBase* regfunc = CRegFunctionBase::gRegistrationList;
    while(regfunc != NULL) {
        regfunc->Register();
        regfunc = regfunc->GetNext();
    }

    // -- register globals
    CRegisterGlobal::RegisterGlobals();

    // -- initialize the scheduler
    CScheduler::Initialize();

    // -- initialize the code block hash table
    CCodeBlock::Initialize();
}

void Update(uint32 curtime) {
    CScheduler::Update(curtime);
    CCodeBlock::DestroyUnusedCodeBlocks();
}

void Shutdown() {
    // -- note, the global namespace is owned by the CNamespace::gNamespaceDictionary
    //delete gGlobalNamespace;
    //gGlobalNamespace = NULL;

    CNamespace::Shutdown();
    CStringTable::Shutdown();
    CScheduler::Shutdown();
    CCodeBlock::Shutdown();
}

// ------------------------------------------------------------------------------------------------
CNamespace* GetGlobalNamespace() {
    return gGlobalNamespace;
}

// ------------------------------------------------------------------------------------------------
uint32 Hash(const char *string, int32 length) {
	if(!string || !string[0])
		return 0;
    const char* s = string;
	int32 remaining = length;

	uint32 h = 5381;
	for (uint8 c = *s; c != '\0' && remaining != 0; c = *++s) {
		--remaining;

#if !CASE_SENSITIVE
        // -- if we're using this language as case insensitive, ensure the character is lower case
        if(c >= 'A' && c <= 'Z')
            c = 'z' + (c - 'A');
#endif

		h = ((h << 5) + h) + c;
	}

    // $$$TZA this should only happen in a DEBUG build
    // -- add to the string table
    CStringTable::AddString(string, length, h);

	return h;
}

uint32 HashAppend(uint32 h, const char *string, int32 length) {
	if(!string || !string[0])
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
    const char* string = CStringTable::FindString(hash);
    if(!string || !string[0]) {
        static char buffers[8][20];
        static int32 bufindex = -1;
        bufindex = (bufindex + 1) % 8;
        sprintf_s(buffers[bufindex], 20, "<hash:0x%08x>", hash);
        return buffers[bufindex];
    }
    else
        return string;
}

void SaveStringTable() {

    const CHashTable<const char>* stringtable = CStringTable::GetStringDictionary();
    if(!stringtable)
        return;

  	// -- open the file
	FILE* filehandle = NULL;
	int32 result = fopen_s(&filehandle, gStringTableFileName, "wb");
	if (result != 0) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

	if(!filehandle) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

    for(uint32 i = 0; i < stringtable->Size(); ++i) {
	    CHashTable<const char>::CHashTableEntry* ste = stringtable->FindRawEntryByBucket(i);
	    while (ste) {
            uint32 stringhash = ste->hash;
            const char* string = ste->item;
            uint32 length = strlen(string);
            char tempbuf[kMaxTokenLength];

            // -- write the hash
            sprintf_s(tempbuf, "0x%08x: ", stringhash);
            int32 count = fwrite(tempbuf, sizeof(char), 12, filehandle);
            if(count != 12) {
                fclose(filehandle);
                ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- write the string length
            sprintf_s(tempbuf, "%04d: ", length);
            count = fwrite(tempbuf, sizeof(char), 6, filehandle);
            if(count != 6) {
                fclose(filehandle);
                ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- write the string
            count = fwrite(string, sizeof(char), length, filehandle);
            if(count != length) {
                fclose(filehandle);
                ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- write the eol
            count = fwrite("\r\n", sizeof(char), 2, filehandle);
            if(count != 2) {
                fclose(filehandle);
                ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
                return;
            }

            // -- next entry
       	    ste = stringtable->GetNextRawEntryInBucket(i);
	    }
    }

    // -- close the file before we leave
	fclose(filehandle);
}

void LoadStringTable() {

  	// -- open the file
	FILE* filehandle = NULL;
	int32 result = fopen_s(&filehandle, gStringTableFileName, "rb");
	if (result != 0) {
		return;
    }

	if(!filehandle) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

    while(!feof(filehandle)) {

        // -- read the hash
        uint32 hash = 0;
        uint32 length = 0;
        char string[kMaxTokenLength];
        char tempbuf[16];

        // -- read the hash
        int32 count = fread(tempbuf, sizeof(char), 12, filehandle);
        if(ferror(filehandle) || count != 12) {
            // -- we're done
            break;
        }
        tempbuf[12] = '\0';
        sscanf_s(tempbuf, "0x%08x: ", &hash);

        // -- read the string length
        count = fread(tempbuf, sizeof(char), 6, filehandle);
        if(ferror(filehandle) || count != 6) {
            fclose(filehandle);
            ScriptAssert_(0, "<internal>", -1, "Error - unable to read file: %s\n", gStringTableFileName);
            return;
        }
        tempbuf[count] = '\0';
        sscanf_s(tempbuf, "%04d: ", &length);


        // -- read the string
        count = fread(string, sizeof(char), length, filehandle);
        if(ferror(filehandle) || count != length) {
            fclose(filehandle);
            ScriptAssert_(0, "<internal>", -1, "Error - unable to read file: %s\n", gStringTableFileName);
            return;
        }
        string[length] = '\0';

        // -- read the eol
        count = fread(tempbuf, sizeof(char), 2, filehandle);
        if(ferror(filehandle) || count != 2) {
            fclose(filehandle);
            ScriptAssert_(0, "<internal>", -1, "Error - unable to read file: %s\n", gStringTableFileName);
            return;
        }

        // -- add the string to the table
        CStringTable::AddString(string, length, hash);
    }

    // -- close the file before we leave
	fclose(filehandle);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// helper functions

nflag GetLastWriteTime(const char* filename, FILETIME& writetime)
{
    if(!filename || !filename[0])
        return false;

    // -- convert the filename to a wchar_t array
    int32 length = strlen(filename);
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

nflag GetBinaryFileName(const char* filename, char* binfilename, int32 maxnamelength) {
    if(!filename)
        return false;

    // -- a script file should end in ".cs"
    const char* extptr = strrchr(filename, '.');
    if(!extptr || _stricmp(extptr, ".cs") != 0)
        return false;

    // -- copy the root name
    uint32 length = (uint32)extptr - uint32(filename);
    SafeStrcpy(binfilename, filename, maxnamelength);
    SafeStrcpy(&binfilename[length], ".cso", maxnamelength - length);

    return true;
}

nflag NeedToCompile(const char* filename, const char* binfilename) {

#if FORCE_COMPILE
    return true;
#endif

    // -- get the filetime for the original script
    // -- if fail, then we have nothing to compile
    FILETIME scriptft;
    if(!GetLastWriteTime(filename, scriptft))
        return false;

    // -- get the filetime for the binary file
    // -- if fail, we need to compile
    FILETIME binft;
    if(!GetLastWriteTime(binfilename, binft))
        return true;

    // -- if the binft is more recent, then we don't need to compile
    if(CompareFileTime(&binft, &scriptft) < 0)
        return true;
    else
        return false;
}

// ------------------------------------------------------------------------------------------------

CCodeBlock* CompileScript(const char* filename) {

    // -- get the name of the output binary file
    char binfilename[kMaxNameLength];
    if(!GetBinaryFileName(filename, binfilename, kMaxNameLength)) {
        ScriptAssert_(0, "<internal>", -1, "Error - invalid script filename: %s\n", filename ? filename : "");
        return NULL;
    }

    // -- compile the source
    CCodeBlock* codeblock = ParseFile(filename);
    if(codeblock == NULL) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to parse file: %s\n", filename);
        return NULL;
    }

    // -- write the binary
    if(!SaveBinary(codeblock, binfilename))
        return NULL;

    return codeblock;
}

nflag ExecScript(const char* filename) {

    char binfilename[kMaxNameLength];
    if(!GetBinaryFileName(filename, binfilename, kMaxNameLength)) {
        ScriptAssert_(0, "<internal>", -1, "Error - invalid script filename: %s\n", filename ? filename : "");
        ResetAssertStack();
        return false;
    }

    CCodeBlock* codeblock = NULL;

    nflag needtocompile = NeedToCompile(filename, binfilename);
    if(needtocompile) {
        codeblock = CompileScript(filename);
        if(!codeblock) {
            ResetAssertStack();
            return false;
        }
    }
    else {
        codeblock = LoadBinary(binfilename);
    }

    // -- execute the codeblock
    nflag result = true;
    if(codeblock) {
	    nflag result = ExecuteCodeBlock(*codeblock);
        if(!result) {
            ScriptAssert_(0, "<internal>", -1, "Error - unable to execute file: %s\n", filename);
            result = false;
        }
        else if(!codeblock->IsInUse()) {
            CCodeBlock::DestroyCodeBlock(codeblock);
        }
    }

    ResetAssertStack();
    return result;
}

// ------------------------------------------------------------------------------------------------
CCodeBlock* CompileCommand(const char* statement) {

    CCodeBlock* commandblock = ParseText("<stdin>", statement);
    return commandblock;
}

nflag ExecCommand(const char* statement) {

    CCodeBlock* stmtblock = CompileCommand(statement);
    if(stmtblock) {
        nflag result = ExecuteCodeBlock(*stmtblock);

        ResetAssertStack();

        // -- if the codeblock didn't define any functions, we're finished with it
        if(!stmtblock->IsInUse())
            CCodeBlock::DestroyCodeBlock(stmtblock);
        return result;
    }
    else {
        ScriptAssert_(0, "<internal>", -1, "Error - Unable to compile: %s\n", statement);
    }

    ResetAssertStack();

    // -- failed
    return false;
}

// ------------------------------------------------------------------------------------------------
// -- TinScript functions and registrations
// ------------------------------------------------------------------------------------------------
nflag Compile(const char* filename) {
    CCodeBlock* result = TinScript::CompileScript(filename);
    ResetAssertStack();
    return (result != NULL);
}
REGISTER_FUNCTION_P1(Compile, Compile, nflag, const char*);
REGISTER_FUNCTION_P1(Exec, ExecScript, nflag, const char*);

// $$$TZA complete hack, but if anything prints to the screen, after it's done, we'll need to
// -- reprint the console input...  having an actual QT app to separate input and output is needed
void Print(const char* string) {
    if(string && string[0]) {
        printf("%s\n", string);

        gRefreshConsoleString = true;
        gRefreshConsoleTimestamp = gCurrentTime;
    }
}

REGISTER_FUNCTION_P1(Print, Print, void, const char*);

// ------------------------------------------------------------------------------------------------
nflag IsObject(uint32 objectid) {
    nflag found = TinScript::CNamespace::FindObject(objectid) != NULL;
    return found;
}

void* FindObject(uint32 objectid) {
    return TinScript::CNamespace::FindObjectAddr(objectid);
}

uint32 FindObjectByName(const char* objname) {
    TinScript::CObjectEntry* oe = TinScript::CNamespace::FindObjectByName(objname);
    return oe ? oe->GetID() : 0;
}

REGISTER_FUNCTION_P1(IsObject, IsObject, nflag, uint32);
REGISTER_FUNCTION_P1(FindObjectByName, FindObjectByName, uint32, const char*);
REGISTER_FUNCTION_P3(AddDynamicVariable, TinScript::CNamespace::AddDynamicVariable, void, uint32, const char*, const char*);
REGISTER_FUNCTION_P2(LinkNamespaces, TinScript::CNamespace::LinkNamespaces, void, const char*, const char*);
REGISTER_FUNCTION_P0(ListObjects, TinScript::CNamespace::ListObjects, void);

} // TinScript

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
