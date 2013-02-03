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
// TinScript.cpp : Defines the entry point for the console application.
//

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
static const unsigned int gFramesPerSecond = 33;
static const unsigned int gMSPerFrame = 1000 / gFramesPerSecond;
static const float gSecPerFrame = (1.0f / float(gFramesPerSecond));
static unsigned int gCurrentTime = 0;
static bool gRefreshConsoleString = false;
static unsigned int gRefreshConsoleTimestamp = 0;
static char gConsoleInputBuf[TinScript::kMaxTokenLength];
static const float gRefreshDelay = 0.25f;

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
	    unsigned int hash = ve->GetHash();
	    GetGlobalNamespace()->GetVarTable()->AddItem(*ve, hash);

        // -- next registration object
        global = global->next;
    }
}

// ------------------------------------------------------------------------------------------------
static bool gAssertEnableTrace = false;
static bool gAssertStackSkipped = false;
void ResetAssertStack() {
    gAssertEnableTrace = false;
    gAssertStackSkipped = false;
}

// -- returns false if we should break
bool AssertHandled(const char* condition, const char* file, int linenumber, const char* fmt, ...) {
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

void Update(unsigned int curtime) {
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
unsigned int Hash(const char *string, int length) {
	if(!string || !string[0])
		return 0;
    const char* s = string;
	int remaining = length;

	unsigned int h = 5381;
	for (unsigned char c = *s; c != '\0' && remaining != 0; c = *++s) {
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

unsigned int HashAppend(unsigned int h, const char *string, int length) {
	if(!string || !string[0])
		return h;
    const char* s = string;
	int remaining = length;

	for (unsigned char c = *s; c != '\0' && remaining != 0; c = *++s) {
		--remaining;
		h = ((h << 5) + h) + c;
	}
	return h;
}

const char* UnHash(unsigned int hash) {
    const char* string = CStringTable::FindString(hash);
    if(!string || !string[0]) {
        static char buffers[8][20];
        static int bufindex = -1;
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
	int result = fopen_s(&filehandle, gStringTableFileName, "wb");
	if (result != 0) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

	if(!filehandle) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

    for(unsigned int i = 0; i < stringtable->Size(); ++i) {
	    CHashTable<const char>::CHashTableEntry* ste = stringtable->FindRawEntryByBucket(i);
	    while (ste) {
            unsigned int stringhash = ste->hash;
            const char* string = ste->item;
            unsigned int length = strlen(string);
            char tempbuf[kMaxTokenLength];

            // -- write the hash
            sprintf_s(tempbuf, "0x%08x: ", stringhash);
            int count = fwrite(tempbuf, sizeof(char), 12, filehandle);
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
	int result = fopen_s(&filehandle, gStringTableFileName, "rb");
	if (result != 0) {
		return;
    }

	if(!filehandle) {
        ScriptAssert_(0, "<internal>", -1, "Error - unable to write file %s\n", gStringTableFileName);
		return;
    }

    while(!feof(filehandle)) {

        // -- read the hash
        unsigned int hash = 0;
        unsigned int length = 0;
        char string[kMaxTokenLength];
        char tempbuf[16];

        // -- read the hash
        int count = fread(tempbuf, sizeof(char), 12, filehandle);
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

bool GetLastWriteTime(const char* filename, FILETIME& writetime)
{
    if(!filename || !filename[0])
        return false;

    // -- convert the filename to a wchar_t array
    int length = strlen(filename);
    wchar_t wfilename[kMaxNameLength];
    for(int i = 0; i < length + 1; ++i)
        wfilename[i] = filename[i];

    HANDLE hFile = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    // Retrieve the file times for the file.
    FILETIME ftCreate, ftAccess;
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &writetime))
        return false;

    return true;
}

bool GetBinaryFileName(const char* filename, char* binfilename, int maxnamelength) {
    if(!filename)
        return false;

    // -- a script file should end in ".cs"
    const char* extptr = strrchr(filename, '.');
    if(!extptr || _stricmp(extptr, ".cs") != 0)
        return false;

    // -- copy the root name
    unsigned int length = (unsigned int)extptr - unsigned int(filename);
    SafeStrcpy(binfilename, filename, maxnamelength);
    SafeStrcpy(&binfilename[length], ".cso", maxnamelength - length);

    return true;
}

bool NeedToCompile(const char* filename, const char* binfilename) {

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

bool ExecScript(const char* filename) {

    char binfilename[kMaxNameLength];
    if(!GetBinaryFileName(filename, binfilename, kMaxNameLength)) {
        ScriptAssert_(0, "<internal>", -1, "Error - invalid script filename: %s\n", filename ? filename : "");
        ResetAssertStack();
        return false;
    }

    CCodeBlock* codeblock = NULL;

    bool needtocompile = NeedToCompile(filename, binfilename);
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
    bool result = true;
    if(codeblock) {
	    bool result = ExecuteCodeBlock(*codeblock);
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

bool ExecCommand(const char* statement) {

    CCodeBlock* stmtblock = CompileCommand(statement);
    if(stmtblock) {
        bool result = ExecuteCodeBlock(*stmtblock);

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
bool Compile(const char* filename) {
    CCodeBlock* result = TinScript::CompileScript(filename);
    ResetAssertStack();
    return (result != NULL);
}
REGISTER_FUNCTION_P1(Compile, Compile, bool, const char*);
REGISTER_FUNCTION_P1(Exec, ExecScript, bool, const char*);

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
static bool gRunning = true;
void Quit() {
    gRunning = false;
}

static bool gPaused = false;
void Pause() {
    gPaused = true;
}
void UnPause() {
    gPaused = false;
}

REGISTER_FUNCTION_P0(Quit, Quit, void);
REGISTER_FUNCTION_P0(Pause, Pause, void);
REGISTER_FUNCTION_P0(UnPause, UnPause, void);

// ------------------------------------------------------------------------------------------------
bool IsObject(unsigned int objectid) {
    bool found = TinScript::CNamespace::FindObject(objectid) != NULL;
    return found;
}

void* FindObject(unsigned int objectid) {
    return TinScript::CNamespace::FindObjectAddr(objectid);
}

unsigned int FindObjectByName(const char* objname) {
    TinScript::CObjectEntry* oe = TinScript::CNamespace::FindObjectByName(objname);
    return oe ? oe->GetID() : 0;
}

REGISTER_FUNCTION_P1(IsObject, IsObject, bool, unsigned int);
REGISTER_FUNCTION_P1(FindObjectByName, FindObjectByName, unsigned int, const char*);
REGISTER_FUNCTION_P3(AddDynamicVariable, TinScript::CNamespace::AddDynamicVariable, void, unsigned int, const char*, const char*);
REGISTER_FUNCTION_P2(LinkNamespaces, TinScript::CNamespace::LinkNamespaces, void, const char*, const char*);
REGISTER_FUNCTION_P0(ListObjects, TinScript::CNamespace::ListObjects, void);

} // TinScript

// ------------------------------------------------------------------------------------------------
// quick and dirty console framework
// ------------------------------------------------------------------------------------------------
unsigned int GetCurrentSimTime() {
    return gCurrentTime;
}

float GetSimTime() {
    float curtime = (float)(GetCurrentSimTime()) / 1000.0f;
    return curtime;
}
REGISTER_FUNCTION_P0(GetSimTime, GetSimTime, float);

float TimeDiffSeconds(unsigned int starttime, unsigned int endtime) {
    if(endtime <= starttime)
        return 0.0f;
    unsigned int framecount = (endtime - starttime) / gMSPerFrame;
    return float(framecount) * gSecPerFrame;
}

void RefreshConsoleInput(bool force = false) {
    if(force || gRefreshConsoleString) {
        gRefreshConsoleString = false;
        printf("\nConsole => %s", gConsoleInputBuf);
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
	TinScript::Initialize();

	// -- convert all the wide args into an array of const char*
	char argstring[kMaxArgs][kMaxArgLength];
	for(int i = 0; i < argc; ++i) {
		size_t arglength = 0;
		if(wcstombs_s(&arglength, argstring[i], kMaxArgLength, argv[i], _TRUNCATE) != 0) {
			printf("Error - invalid arg# %d\n", i);
			return 1;
		}
	}

	// -- info passed in via command line arguments
	const char* infilename = NULL;
	int argindex = 1;
	while (argindex < argc) {
		size_t arglength = 0;
		char currarg[kMaxArgLength] = { 0 };
		if(wcstombs_s(&arglength, currarg, kMaxArgLength, argv[argindex], _TRUNCATE) != 0) {
			printf("Error - invalid arg# %d\n", argindex);
			return 1;
		}
		if (!_stricmp(currarg, "-f") || !_stricmp(currarg, "-file")) {
			if (argindex >= argc - 1) {
				printf("Error - invalid arg '-f': no filename given\n");
				return 1;
			}
			else {
				infilename = argstring[argindex + 1];
				argindex += 2;
			}
		}
		else {
			printf("Error - unknown arg: %s\n", argstring[argindex]);
			return 1;
		}
	}

	// -- parse the file
	if(infilename && infilename[0] && ! TinScript::ExecScript(infilename)) {
		printf("Error - unable to parse file: %s\n", infilename);
		return 1;
	}

    // -- q&d history implementation
    bool historyfull = false;
    const int maxhistory = 64;
    int historyindex = -1;
    int historylastindex = -1;
    char history[TinScript::kMaxTokenLength][maxhistory];
    for(int i = 0; i < maxhistory; ++i)
        *history[i] = '\0';

    char* inputptr = gConsoleInputBuf;
    printf("\nConsole => ");
    while(TinScript::gRunning) {
        // -- simulate a 33ms frametime
        // -- time needs to stand still while an assert is active
        Sleep(gMSPerFrame);
        if(!TinScript::gPaused) {
            gCurrentTime += gMSPerFrame;
        }

        // -- keep the system running...
        TinScript::Update(gCurrentTime);
        
        // -- see if we should auto-refresh the console
        if(gRefreshConsoleString && TimeDiffSeconds(gRefreshConsoleTimestamp, gCurrentTime) >
                                    gRefreshDelay) {
            RefreshConsoleInput();
        }

        // -- see if we hit a key
        if(_kbhit()) {

            // -- read the next key
            bool special_key = false;
            char c = _getch();
            if(c == -32) {
                special_key = true;
                c = _getch();
            }


            // -- esc
            if(!special_key && c == 27) {
                inputptr = gConsoleInputBuf;
                *inputptr = '\0';
                historyindex = -1;
                RefreshConsoleInput(true);
            }

            // -- uparrow
            else if(special_key && c == 72) {
                int oldhistory = historyindex;
                if(historyindex < 0)
                    historyindex = historylastindex;
                else if(historylastindex > 0) {
                    if(historyfull)
                        historyindex = (historyindex + maxhistory - 1) % maxhistory;
                    else
                        historyindex = (historyindex + historylastindex) % (historylastindex + 1);
                }

                // -- see if we actually changed
                if(historyindex != oldhistory && historyindex >= 0) {
                    TinScript::SafeStrcpy(gConsoleInputBuf, history[historyindex], TinScript::kMaxTokenLength);
                    inputptr = &gConsoleInputBuf[strlen(gConsoleInputBuf)];
                    *inputptr = '\0';
                    RefreshConsoleInput(true);
                }
            }

            // -- downarrow
            else if(special_key && c == 80) {
                int oldhistory = historyindex;
                if(historyindex < 0)
                    historyindex = historylastindex;
                else if(historylastindex > 0) {
                    if(historyfull)
                        historyindex = (historyindex + 1) % maxhistory;
                    else
                        historyindex = (historyindex + 1) % (historylastindex + 1);
                }

                // -- see if we actually changed
                if(historyindex != oldhistory && historyindex >= 0) {
                    TinScript::SafeStrcpy(gConsoleInputBuf, history[historyindex], TinScript::kMaxTokenLength);
                    inputptr = &gConsoleInputBuf[strlen(gConsoleInputBuf)];
                    *inputptr = '\0';
                    RefreshConsoleInput(true);
                }
            }

            // -- backspace keypress
            else if(!special_key && c == 8 && inputptr > gConsoleInputBuf) {
                *--inputptr = '\0';
                gRefreshConsoleString = true;
                gRefreshConsoleTimestamp = gCurrentTime;
            }

            // -- return keypress
            else if(!special_key && c == 13) {
                // -- echo the input and execute it
                *inputptr = '\0';
                RefreshConsoleInput();
                printf("\n>> %s\n", gConsoleInputBuf);

                // -- add this to the history buf
                const char* historyptr = (historylastindex < 0) ? NULL : history[historylastindex];
                if(gConsoleInputBuf[0] != '\0' && (!historyptr ||
                                                   strcmp(historyptr, gConsoleInputBuf) != 0)) {
                    historyfull = historyfull || historylastindex == maxhistory - 1;
                    historylastindex = (historylastindex + 1) % maxhistory;
                    TinScript::SafeStrcpy(history[historylastindex], gConsoleInputBuf, TinScript::kMaxTokenLength);
                }
                historyindex = -1;

                TinScript::ExecCommand(gConsoleInputBuf);
                inputptr = gConsoleInputBuf;
                *inputptr = '\0';
                printf("\nConsole => ");
            }

            // ignore any other non-printable character
            else if(!special_key && (unsigned int)c >= 0x20) {
                RefreshConsoleInput();
                *inputptr++ = c;
                *inputptr = '\0';
                printf("%c", c);
            }
        }
    }

	TinScript::Shutdown();

	return 0;
}

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
