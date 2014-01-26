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
// TinQTConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <QApplication>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QTextlist>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <qcolor.h>

#include "qmainwindow.h"
#include "qmetaobject.h"
#include "qevent.h"

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTWatchWin.h"

// ------------------------------------------------------------------------------------------------
// -- override the macro from integration.h
#undef TinPrint
#define TinPrint Print

// ------------------------------------------------------------------------------------------------
CConsoleWindow* CConsoleWindow::gConsoleWindow = NULL;
CConsoleWindow::CConsoleWindow() {
    // -- set the singleton
    gConsoleWindow = this;

    // -- create the Qt application components
    int argcount = 0;
    mApp = new QApplication(argcount, NULL);
    mApp->setOrganizationName("QtProject");
    mApp->setApplicationName("TinConsole");

    // -- create the main window
    mMainWindow = new QWidget();
    mMainWindow->resize(QSize(1200, 800));

    // -- create the output widget
    mConsoleOutput = new CConsoleOutput(this);
    mConsoleOutput->addItem("Welcome to the TinConsole!");

    // -- create the consoleinput
    mConsoleInput = new CConsoleInput(this);
    mConsoleInput->setFixedHeight(24);

    // -- create the debugger components
    mToolbarLayout = new QHBoxLayout();
    mFileLabel = new QLabel("File:");
    mFileLineEdit = new QLineEdit();
    mButtonRun = new QPushButton();
    mButtonRun->setText("Run");
    mButtonRun->setGeometry(0, 0, 32, 24); 
    mButtonStep = new QPushButton();
    mButtonStep->setText("Step");
    mButtonStep->setGeometry(0, 0, 32, 24);
    mButtonPause = new QPushButton();
    mButtonPause->setText("Pause");
    mButtonPause->setGeometry(0, 0, 32, 24);
    mSpacer = new QWidget();

    mToolbarLayout->addWidget(mFileLabel);
    mToolbarLayout->addWidget(mFileLineEdit);
    mToolbarLayout->addWidget(mButtonRun);
    mToolbarLayout->addWidget(mButtonStep);
    mToolbarLayout->addWidget(mButtonPause);
    mToolbarLayout->addWidget(mSpacer, 1);

    // -- create the source window
    mDebugSourceWin = new CDebugSourceWin(this);

    // -- create the callstack window
    mCallstackWin = new CDebugCallstackWin(this);

    // -- create the breakpoints window
    mBreakpointsWin = new CDebugBreakpointsWin(this);

    // -- create the watch window
    mWatchWin = new CDebugWatchWin(this);

    // -- column 0
    mGridLayout = new QGridLayout();
    mGridLayout->addLayout(mToolbarLayout,  0, 0, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mDebugSourceWin, 1, 0, 2, Qt::AlignLeft);
    mGridLayout->addWidget(mConsoleOutput,  3, 0, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mConsoleInput,   4, 0, 1, Qt::AlignLeft);

    // -- column 1
    mGridLayout->addWidget(mCallstackWin,   1, 1, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mBreakpointsWin, 2, 1, 1, Qt::AlignLeft);

    mGridLayout->addWidget(mWatchWin,       3, 1, 1, Qt::AlignLeft);

    // -- temp test
    /*
    mWatchWin->BeginVariable("TestObject", TinScript::TYPE_object, "ID:  5");
    mWatchWin->AddVariable("Foo", TinScript::TYPE_float, "3.0f");
    mWatchWin->AddVariable("Bar", TinScript::TYPE_int, "17");
    mWatchWin->AddNamespace("BaseObject");
    mWatchWin->AddVariable("Name", TinScript::TYPE_string, "Whatever");
    mWatchWin->AddVariable("Class", TinScript::TYPE_string, "Heffe");
    */

    // -- connect the widgets
    QObject::connect(mConsoleInput, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnReturnPressed()));

    QObject::connect(mDebugSourceWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mDebugSourceWin,
                                      SLOT(OnDoubleClicked(QListWidgetItem*)));

    QObject::connect(mFileLineEdit, SIGNAL(returnPressed()), mConsoleInput,
                                    SLOT(OnFileEditReturnPressed()));

    QObject::connect(mButtonRun, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonRunPressed()));
    QObject::connect(mButtonStep, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonStepPressed()));
    QObject::connect(mButtonPause, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonPausePressed()));

    QObject::connect(mCallstackWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mCallstackWin,
                                    SLOT(OnDoubleClicked(QListWidgetItem*)));

    QObject::connect(mBreakpointsWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mBreakpointsWin,
                                      SLOT(OnDoubleClicked(QListWidgetItem*)));
    QObject::connect(mBreakpointsWin, SIGNAL(itemClicked(QListWidgetItem*)), mBreakpointsWin,
                                      SLOT(OnClicked(QListWidgetItem*)));

    mMainWindow->setLayout(mGridLayout);
    mMainWindow->show();

    // -- initialize the running members
    mQuit = false;
    mPaused = false;
}

CConsoleWindow::~CConsoleWindow() {
    delete mFileLabel;
    delete mFileLineEdit;
    delete mButtonRun;
    delete mButtonStep;
    delete mButtonPause;
    delete mSpacer;
    delete mToolbarLayout;

    delete mCallstackWin;
    delete mBreakpointsWin;
    delete mDebugSourceWin;
    delete mConsoleInput;
    delete mConsoleOutput;
    delete mGridLayout;
    delete mMainWindow;
    delete mApp;
}

int CConsoleWindow::Exec() {
    return mApp->exec();
}

// ------------------------------------------------------------------------------------------------
// -- create a handler to register, so we can receive print messages and asserts
bool PushAssertDialog(const char* assertmsg, const char* errormsg, bool& skip, bool& trace);

int ConsolePrint(const char* fmt, ...) {
    // -- write the string into the buffer
    va_list args;
    va_start(args, fmt);
    char buffer[2048];
    vsprintf_s(buffer, 2048, fmt, args);
    va_end(args);

    CConsoleWindow::GetInstance()->AddText(buffer);
    return(0);
}

// -- returns false if we should break
bool8 AssertHandler(TinScript::CScriptContext* script_context, const char* condition,
                    const char* file, int32 linenumber, const char* fmt, ...) {
    if(!script_context->IsAssertStackSkipped() || script_context->IsAssertEnableTrace()) {
        if(!script_context->IsAssertStackSkipped())
            ConsolePrint("*************************************************************\n");
        else
            ConsolePrint("\n");

        // -- get the assert msg
        char assertmsg[2048];
        if(linenumber >= 0)
            sprintf_s(assertmsg, 2048, "Assert(%s) file: %s, line %d:\n", condition, file, linenumber + 1);
        else
            sprintf_s(assertmsg, 2048, "Exec Assert(%s):\n", condition);

        char errormsg[2048];
        va_list args;
        va_start(args, fmt);
        vsprintf_s(errormsg, 2048, fmt, args);
        va_end(args);

        ConsolePrint(assertmsg);
        ConsolePrint(errormsg);

        if(!script_context->IsAssertStackSkipped())
            ConsolePrint("*************************************************************\n");
        if(!script_context->IsAssertStackSkipped()) {
            bool press_skip = false;
            bool press_trace = false;
            bool result = PushAssertDialog(assertmsg, errormsg, press_skip, press_trace);
            script_context->SetAssertEnableTrace(press_trace);
            script_context->SetAssertStackSkipped(press_skip);
            return (result);
        }
    }

    // -- handled - return true so we don't break
    return true;
}

// ------------------------------------------------------------------------------------------------
void PushBreakpointDialog(const char* breakpoint_msg) {
    QMessageBox msgBox;
    msgBox.setModal(true);
    msgBox.setText(breakpoint_msg);
    msgBox.show();
}

// ------------------------------------------------------------------------------------------------
int32 CConsoleWindow::ToggleBreakpoint(uint32 codeblock_hash, int32 line_number,
                                       bool add, bool enable) {
    const char* filename = TinScript::UnHash(codeblock_hash);
    if(!filename)
        return (-1);

    // -- three components need to be informed:  the script context
    int32 actual_line = add && enable ?
                        TinScript::GetContext()->AddBreakpoint(filename, line_number) :
                        TinScript::GetContext()->RemoveBreakpoint(filename, line_number);

    // -- if the codeblock hasn't yet been compiled (loaded), use the original line_number
    if(actual_line < 0)
        actual_line = line_number;

    // -- the Source Window
    GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, actual_line, add, enable);

    // -- the Breakpoints Window
    GetDebugBreakpointsWin()->ToggleBreakpoint(codeblock_hash, actual_line, add, enable);

    return (actual_line);
}

// ------------------------------------------------------------------------------------------------
void CConsoleWindow::HandleBreakpointHit(const char* breakpoint_msg) {
    mBreakpointRun = false;
    mBreakpointStep = false;

    // -- highlight the button in green
    mButtonRun->setAutoFillBackground(true);
	QPalette myPalette = mButtonRun->palette();
	myPalette.setColor(QPalette::Button, Qt::red);	
	mButtonRun->setPalette(myPalette);

    while(!mBreakpointRun && !mBreakpointStep) {
        QCoreApplication::processEvents();
    }

    // -- back to normal color
	myPalette.setColor(QPalette::Button, Qt::transparent);	
	mButtonRun->setPalette(myPalette);

    // -- clear the callstack and the watch window
    GetDebugCallstackWin()->ClearCallstack();
    GetDebugWatchWin()->ClearWatchWin();
}

// ------------------------------------------------------------------------------------------------
// -- debugger support
bool8 DebuggerBreakpointHit(uint32 codeblock_hash, int32& line_number) {
    bool8 press_ignore_run = false;
    bool8 press_trace_step = false;
    char breakpoint_msg[256];
    sprintf_s(breakpoint_msg, 256, "Break on line: %d", line_number);

    // -- set the PC
    CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(codeblock_hash, line_number);

    // -- loop until we press either "Run" or "Step"
    CConsoleWindow::GetInstance()->HandleBreakpointHit(breakpoint_msg);

    // -- clear the PC
    CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(codeblock_hash, -1);

    // -- return true if we're supposed to keep running
    if(CConsoleWindow::GetInstance()->mBreakpointRun)
        return (true);

    // -- otherwise return false, to step to the next statement
    else
        return (false);
}

void NotifyCallstack(uint32* codeblock_array, uint32* objid_array, uint32* namespace_array,
                     uint32* func_array, uint32* linenumber_array, int array_size) {
    CConsoleWindow::GetInstance()->GetDebugCallstackWin()->
        NotifyCallstack(codeblock_array, objid_array, namespace_array,
                        func_array, linenumber_array, array_size);
    CConsoleWindow::GetInstance()->GetDebugWatchWin()->ClearWatchWin();
}

void NotifyCodeblockLoaded(uint32 codeblock_hash) {
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->NotifyCodeblockLoaded(codeblock_hash);
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->NotifyCodeblockLoaded(codeblock_hash);
}

void NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry) {
    CConsoleWindow::GetInstance()->GetDebugWatchWin()->NotifyWatchVarEntry(watch_var_entry);
}
        
// ------------------------------------------------------------------------------------------------
void CConsoleWindow::AddText(char* msg) {
    if(!msg)
        return;

    // -- AddItem implicitly adds a newline...  strip one off if found
    int length = strlen(msg);
    if(length > 0 && msg[length - 1] == '\n')
        msg[length - 1] = '\0';

    // -- add to the output window
    mConsoleOutput->addItem(msg);

    // -- scroll to the bottom of the window
    int count = mConsoleOutput->count();
    mConsoleOutput->setCurrentRow(count - 1);
}

// ------------------------------------------------------------------------------------------------
CConsoleInput::CConsoleInput(CConsoleWindow* owner) : QLineEdit() {
    mOwner = owner;

    // -- q&d history implementation
    mHistoryFull = false;
    mHistoryIndex = -1;
    mHistoryLastIndex = -1;
    for(int32 i = 0; i < kMaxHistory; ++i)
        *mHistory[i] = '\0';
}

void CConsoleInput::OnReturnPressed() {
    QByteArray input_ba = text().toUtf8();
    const char* input_text = input_ba.data();
    ConsolePrint("> %s", input_text);

    // -- add this to the history buf
    const char* historyptr = (mHistoryLastIndex < 0) ? NULL : mHistory[mHistoryLastIndex];
    if(input_text[0] != '\0' && (!historyptr || strcmp(historyptr, input_text) != 0)) {
        mHistoryFull = mHistoryFull || mHistoryLastIndex == kMaxHistory - 1;
        mHistoryLastIndex = (mHistoryLastIndex + 1) % kMaxHistory;
        strcpy_s(mHistory[mHistoryLastIndex], TinScript::kMaxTokenLength, input_text);
    }
    mHistoryIndex = -1;

    TinScript::ExecCommand(input_text);
    setText("");
}

void CConsoleInput::OnFileEditReturnPressed() {
    QLineEdit* fileedit = CConsoleWindow::GetInstance()->GetFileLineEdit();
    QString filename = fileedit->text();
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->OpenSourceFile(filename.toUtf8());
}

void CConsoleInput::OnButtonRunPressed() {
    CConsoleWindow::GetInstance()->mBreakpointRun = true;
}

void CConsoleInput::OnButtonStepPressed() {
    CConsoleWindow::GetInstance()->mBreakpointStep = true;
}

void CConsoleInput::OnButtonPausePressed() {
    mOwner->mButtonPause->setAutoFillBackground(true);
	QPalette myPalette = mOwner->mButtonPause->palette();

    bool is_paused = mOwner->IsPaused();
    if(is_paused) {
        mOwner->Unpause();
        mOwner->mButtonPause->setText("Pause");

        myPalette.setColor(QPalette::Button, Qt::transparent);	
	    mOwner->mButtonPause->setPalette(myPalette);
    }
    else {
        mOwner->Pause();
        mOwner->mButtonPause->setText("Unpause");

        myPalette.setColor(QPalette::Button, Qt::red);	
	    mOwner->mButtonPause->setPalette(myPalette);
    }
}

// ------------------------------------------------------------------------------------------------
void CConsoleInput::keyPressEvent(QKeyEvent * event) {
    if(!event)
        return;

    // -- up arrow
    if(event->key() == Qt::Key_Up) {
        int32 oldhistory = mHistoryIndex;
        if(mHistoryIndex < 0)
            mHistoryIndex = mHistoryLastIndex;
        else if(mHistoryLastIndex > 0) {
            if(mHistoryFull)
                mHistoryIndex = (mHistoryIndex + kMaxHistory - 1) % kMaxHistory;
            else
                mHistoryIndex = (mHistoryIndex + mHistoryLastIndex) % (mHistoryLastIndex + 1);
        }

        // -- see if we actually changed
        if(mHistoryIndex != oldhistory && mHistoryIndex >= 0) {
            setText(mHistory[mHistoryIndex]);
        }
    }

    // -- down arrow
    else if (event->key() == Qt::Key_Down) {
        int32 oldhistory = mHistoryIndex;
        if(mHistoryIndex < 0)
            mHistoryIndex = mHistoryLastIndex;
        else if(mHistoryLastIndex > 0) {
            if(mHistoryFull)
                mHistoryIndex = (mHistoryIndex + 1) % kMaxHistory;
            else
                mHistoryIndex = (mHistoryIndex + 1) % (mHistoryLastIndex + 1);
        }

        // -- see if we actually changed
        if(mHistoryIndex != oldhistory && mHistoryIndex >= 0) {
            setText(mHistory[mHistoryIndex]);
        }
    }

    // -- esc
    else if (event->key() == Qt::Key_Escape) {
        setText("");
        mHistoryIndex = -1;
    }

    else {

        QLineEdit::keyPressEvent(event);
    }
}

// ------------------------------------------------------------------------------------------------
CConsoleOutput::CConsoleOutput(CConsoleWindow* owner) : QListWidget() {
    mOwner = owner;
    mCurrentTime = 0;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(Update()));
    mTimer->start(kUpdateTime);
}

CConsoleOutput::~CConsoleOutput() {
    delete mTimer;
}

void CConsoleOutput::Update() {
    // -- if we're not paused, update TinScript
    if(!mOwner->IsPaused()) {
        mCurrentTime += kUpdateTime;
        TinScript::UpdateContext(mCurrentTime);
    }

    // -- see if we're supposed to quit
    if(mOwner->IsQuit()) {
        QApplication::closeAllWindows();
    }
}

bool PushAssertDialog(const char* assertmsg, const char* errormsg, bool& skip, bool& trace) {
    QMessageBox msgBox;

    QString dialog_msg = QString(assertmsg) + QString("\n") + QString(errormsg);
    msgBox.setText(dialog_msg);

    QPushButton *ignore_button = msgBox.addButton("Ignore", QMessageBox::ActionRole);
    QPushButton *trace_button = msgBox.addButton("Trace", QMessageBox::ActionRole);
    QPushButton *break_button = msgBox.addButton("Break", QMessageBox::ActionRole);

    msgBox.exec();

    bool handled = false;
    if (msgBox.clickedButton() == ignore_button) {
        skip = true;
        trace = false;
        handled = true;
    }
    else if (msgBox.clickedButton() == trace_button) {
        skip = true;
        trace = true;
        handled = true;
    }
    else if (msgBox.clickedButton() == break_button) {
        skip = false;
        trace = false;
        handled = false;
    }
    return (handled);
}

// ------------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
    // -- required to ensure registered functions from unittest.cpp are linked.
    REGISTER_FILE(unittest_cpp);
    REGISTER_FILE(mathutil_cpp);

    // -- initialize (true for MainThread context)
    TinScript::CreateContext(ConsolePrint, AssertHandler, true);

    // -- register the debugger breakpoint function
    TinScript::GetContext()->RegisterDebugger(DebuggerBreakpointHit, NotifyCallstack,
                                     NotifyCodeblockLoaded, NotifyWatchVarEntry);

    new CConsoleWindow();
    int result = CConsoleWindow::GetInstance()->Exec();

    // -- shutdown
    TinScript::DestroyContext();

    return result;
}

// ------------------------------------------------------------------------------------------------
void Quit() {
    CConsoleWindow::GetInstance()->Quit();
}

void Pause() {
    CConsoleWindow::GetInstance()->Pause();
}

void Unpause() {
    CConsoleWindow::GetInstance()->Unpause();
}

float32 GetSimTime() {
    int32 cur_time = CConsoleWindow::GetInstance()->GetOutput()->GetSimTime();
    float32 seconds = (float32)cur_time / 1000.0f;
    return (seconds);
}

REGISTER_FUNCTION_P0(Quit, Quit, void);
REGISTER_FUNCTION_P0(Pause, Pause, void);
REGISTER_FUNCTION_P0(Unpause, Unpause, void);

REGISTER_FUNCTION_P0(GetSimTime, GetSimTime, float32);

// ------------------------------------------------------------------------------------------------
#include "TinQTConsoleMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
