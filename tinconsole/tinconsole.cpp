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
// tinconsole.cpp - a crappy shell used to demonstrate and develop the tinscript library
// ------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "windows.h"
#include "conio.h"

#include "TinScript.h"
#include "TinRegistration.h"

// ------------------------------------------------------------------------------------------------
// statics - mostly for the quick and dirty console implementation
static const uint32 gFramesPerSecond = 33;
static const uint32 gMSPerFrame = 1000 / gFramesPerSecond;
static const float32 gSecPerFrame = (1.0f / float32(gFramesPerSecond));
static uint32 gCurrentTime = 0;
static bool8 gRefreshConsoleString = false;
static uint32 gRefreshConsoleTimestamp = 0;
static char gConsoleInputBuf[TinScript::kMaxTokenLength];
static const float32 gRefreshDelay = 0.25f;

// ------------------------------------------------------------------------------------------------
// quick and dirty console framework
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static bool8 gRunning = true;
void Quit() {
    gRunning = false;
}

static bool8 gPaused = false;
void Pause() {
    gPaused = true;
}
void UnPause() {
    gPaused = false;
}

REGISTER_FUNCTION_P0(Quit, Quit, void);
REGISTER_FUNCTION_P0(Pause, Pause, void);
REGISTER_FUNCTION_P0(UnPause, UnPause, void);

void Print(const char* string) {
    printf("%s\n", string);
}

REGISTER_FUNCTION_P1(Print, Print, void, const char*);

uint32 GetCurrentSimTime() {
    return gCurrentTime;
}

float32 GetSimTime() {
    float32 curtime = (float32)(GetCurrentSimTime()) / 1000.0f;
    return curtime;
}
REGISTER_FUNCTION_P0(GetSimTime, GetSimTime, float32);

float32 TimeDiffSeconds(uint32 starttime, uint32 endtime) {
    if(endtime <= starttime)
        return 0.0f;
    uint32 framecount = (endtime - starttime) / gMSPerFrame;
    return float32(framecount) * gSecPerFrame;
}

void RefreshConsoleInput(bool8 force = false) {
    if(force || gRefreshConsoleString) {
        gRefreshConsoleString = false;
        printf("\nConsole => %s", gConsoleInputBuf);
    }
}

TinScript::CScriptContext* gScriptContext = NULL;

// -- returns false if we should break
bool8 AssertHandler(TinScript::CScriptContext* script_context, const char* condition,
                    const char* file, int32 linenumber, const char* fmt, ...) {
    if(!script_context->IsAssertStackSkipped() || script_context->IsAssertEnableTrace()) {
        if(!script_context->IsAssertStackSkipped())
            printf("*************************************************************\n");
        else
            printf("\n");

        if(linenumber >= 0)
            printf("Assert(%s) file: %s, line %d:\n", condition, file, linenumber + 1);
        else
            printf("Exec Assert(%s):\n", condition);

        va_list args;
        va_start(args, fmt);
        char msgbuf[2048];
        vsprintf_s(msgbuf, 2048, fmt, args);
        va_end(args);
        printf(msgbuf);

        if(!script_context->IsAssertStackSkipped())
            printf("*************************************************************\n");
        if(!script_context->IsAssertStackSkipped()) {
            printf("Press 'b' to break, 't' to trace, otherwise skip...\n");
            char ch = getchar();
            if(ch == 'b')
                return false;
            else if(ch == 't') {
                script_context->SetAssertStackSkipped(true);
                script_context->SetAssertEnableTrace(true);
                return true;
            }
            else {
                script_context->SetAssertStackSkipped(true);
                script_context->SetAssertEnableTrace(false);
                return true;
            }
        }
    }

    // -- handled - return true so we don't break
    return true;
}

int32 _tmain(int32 argc, _TCHAR* argv[])
{
    // -- initialize
    gScriptContext = TinScript::CScriptContext::Create(NULL, printf, AssertHandler);

    // -- required to ensure registered functions from unittest.cpp are linked.
    REGISTER_FILE(unittest_cpp);

	// -- convert all the wide args into an array of const char*
	char argstring[kMaxArgs][kMaxArgLength];
	for(int32 i = 0; i < argc; ++i) {
		size_t arglength = 0;
		if(wcstombs_s(&arglength, argstring[i], kMaxArgLength, argv[i], _TRUNCATE) != 0) {
			printf("Error - invalid arg# %d\n", i);
			return 1;
		}
	}

	// -- info passed in via command line arguments
	const char* infilename = NULL;
	int32 argindex = 1;
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
	if(infilename && infilename[0] && !gScriptContext->ExecScript(infilename)) {
		printf("Error - unable to parse file: %s\n", infilename);
		return 1;
	}

    // -- q&d history implementation
    bool8 historyfull = false;
    const int32 maxhistory = 64;
    int32 historyindex = -1;
    int32 historylastindex = -1;
    char history[TinScript::kMaxTokenLength][maxhistory];
    for(int32 i = 0; i < maxhistory; ++i)
        *history[i] = '\0';

    char* inputptr = gConsoleInputBuf;
    printf("\nConsole => ");
    while(gRunning) {
        // -- simulate a 33ms frametime
        // -- time needs to stand still while an assert is active
        Sleep(gMSPerFrame);
        if(!gPaused) {
            gCurrentTime += gMSPerFrame;
        }

        // -- keep the system running...
        gScriptContext->Update(gCurrentTime);
        
        // -- see if we should auto-refresh the console
        if(gRefreshConsoleString && TimeDiffSeconds(gRefreshConsoleTimestamp, gCurrentTime) >
                                    gRefreshDelay) {
            RefreshConsoleInput();
        }

        // -- see if we hit a key
        if(_kbhit()) {

            // -- read the next key
            bool8 special_key = false;
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
                int32 oldhistory = historyindex;
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
                int32 oldhistory = historyindex;
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

                gScriptContext->ExecCommand(gConsoleInputBuf);
                inputptr = gConsoleInputBuf;
                *inputptr = '\0';
                printf("\nConsole => ");
            }

            // ignore any other non-printable character
            else if(!special_key && (uint32)c >= 0x20) {
                RefreshConsoleInput();
                *inputptr++ = c;
                *inputptr = '\0';
                printf("%c", c);
            }
        }
    }

    TinScript::CScriptContext::Destroy(gScriptContext);

	return 0;
}

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
