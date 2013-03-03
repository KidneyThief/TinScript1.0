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
// TinQTConsole.h

#ifndef __TINQTCONSOLE_H
#define __TINQTCONSOLE_H

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

class CConsoleInput;
class CConsoleOutput;
class CDebugSourceWin;
class CDebugToolBar;
class QGridLayout;
class CDebugBreakpointsWin;
class CDebugCallstackWin;

class CConsoleWindow {
    public:
        CConsoleWindow();
        virtual ~CConsoleWindow();
        static CConsoleWindow* GetInstance() { return (gConsoleWindow); }

        int Exec();

        // -- scripted methods
        void Quit() { mQuit = true; }
        void Pause() { mPaused = true; }
        void Unpause() { mPaused = false; }

        bool IsQuit() { return mQuit; }
        bool IsPaused() { return mPaused; }

        void AddText(char* msg);

        // -- Qt component accessors
        CConsoleOutput* GetOutput() { return (mConsoleOutput); }
        CConsoleInput* GetInput() { return (mConsoleInput); }
        QLineEdit* GetFileLineEdit() { return (mFileLineEdit); }
        CDebugSourceWin* GetDebugSourceWin() { return (mDebugSourceWin); }
        CDebugBreakpointsWin* GetDebugBreakpointsWin() { return (mBreakpointsWin); }
        CDebugCallstackWin* GetDebugCallstackWin() { return (mCallstackWin); }

        // -- Qt components
        QApplication* mApp;
        QWidget* mMainWindow;
        QGridLayout* mGridLayout;

        CConsoleOutput* mConsoleOutput;
        CConsoleInput* mConsoleInput;
        CDebugSourceWin* mDebugSourceWin;
        CDebugBreakpointsWin* mBreakpointsWin;
        CDebugCallstackWin* mCallstackWin;

        QHBoxLayout* mToolbarLayout;
        QLabel* mFileLabel;
        QLineEdit* mFileLineEdit;
        QPushButton* mButtonRun;
        QPushButton* mButtonStep;
        QPushButton* mButtonPause;
        QWidget* mSpacer;

        // -- debugger methods
        int32 ToggleBreakpoint(uint32 codeblock_hash, int32 line_number, bool add, bool enable);

        void HandleBreakpointHit(const char* breakpoint_msg);
        bool mBreakpointRun;
        bool mBreakpointStep;


    private:
        static CConsoleWindow* gConsoleWindow;

        // -- pause/unpause members
        bool mQuit;
        bool mPaused;
};

class CConsoleInput : public QLineEdit {
    Q_OBJECT;

    public:
        explicit CConsoleInput(CConsoleWindow* owner);
        virtual ~CConsoleInput() { }

    public slots:
        void OnReturnPressed();
        void OnFileEditReturnPressed();
        void OnButtonRunPressed();
        void OnButtonStepPressed();
        void OnButtonPausePressed();

    protected:
        virtual void keyPressEvent(QKeyEvent * event);

    private:
        CConsoleWindow* mOwner;

    static const int32 kMaxHistory = 64;
    bool8 mHistoryFull;
    int32 mHistoryIndex;
    int32 mHistoryLastIndex;
    char mHistory[TinScript::kMaxTokenLength][kMaxHistory];
};

class CConsoleOutput : public QListWidget {
    Q_OBJECT;

    public:
        explicit CConsoleOutput(CConsoleWindow* owner);
        virtual ~CConsoleOutput();

        static const unsigned int kUpdateTime = 33;
        int32 GetSimTime() { return (mCurrentTime); }

    public slots:
        void Update();

    private:
        // -- the console output handles the current time, and timer events to call Update()
        CConsoleWindow* mOwner;
        QTimer* mTimer;

        unsigned int mCurrentTime;
};

TinScript::CScriptContext* GetScriptContext();
int ConsolePrint(const char* fmt, ...);

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
