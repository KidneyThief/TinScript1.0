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
static const unsigned int gFramesPerSecond = 33;
static const unsigned int gMSPerFrame = 1000 / gFramesPerSecond;
static const float gSecPerFrame = (1.0f / float(gFramesPerSecond));
static unsigned int gCurrentTime = 0;
static bool gRefreshConsoleString = false;
static unsigned int gRefreshConsoleTimestamp = 0;
static char gConsoleInputBuf[TinScript::kMaxTokenLength];
static const float gRefreshDelay = 0.25f;

// ------------------------------------------------------------------------------------------------
// quick and dirty console framework
// ------------------------------------------------------------------------------------------------
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

extern void BeginUnitTests(int teststart, int testend);
REGISTER_FUNCTION_P2(BeginUnitTests, BeginUnitTests, void, int, int);

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
    while(gRunning) {
        // -- simulate a 33ms frametime
        // -- time needs to stand still while an assert is active
        Sleep(gMSPerFrame);
        if(!gPaused) {
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
