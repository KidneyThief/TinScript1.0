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

#include "qmainwindow.h"
#include "qmetaobject.h"
#include "qevent.h"

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"

// ------------------------------------------------------------------------------------------------
// -- override the macro from integration.h
#undef TinPrint
#define TinPrint Print

TinScript::CScriptContext* gScriptContext = NULL;

// ------------------------------------------------------------------------------------------------
CConsoleWindow* gConsoleWindow = NULL;
CConsoleWindow::CConsoleWindow() {

    // -- create the Qt application components
    int argcount = 0;
    mApp = new QApplication(argcount, NULL);
    mApp->setOrganizationName("QtProject");
    mApp->setApplicationName("TinConsole");

    // -- create the main window
    mMainWindow = new QWidget();
    mMainWindow->resize(QSize(640, 480));

    // -- create the output widget
    mConsoleOutput = new CConsoleOutput(this);
    mConsoleOutput->addItem("Welcome to the TinConsole!");

    // -- create the consoleinput
    mConsoleInput = new CConsoleInput(this);
    mConsoleInput->setFixedHeight(24);

    mGridLayout = new QGridLayout();
    mGridLayout->addWidget(mConsoleOutput, 0, 0, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mConsoleInput, 1, 0, 1, Qt::AlignLeft);

    // -- connect the widgets
    QObject::connect(mConsoleInput, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnReturnPressed()));

    mMainWindow->setLayout(mGridLayout);
    mMainWindow->show();

    // -- force the console input to hold on to keyboard focus
    mConsoleInput->grabKeyboard();

    // -- initialize the running members
    mQuit = false;
    mPaused = false;;
}

CConsoleWindow::~CConsoleWindow() {
    delete mConsoleInput;
    delete mConsoleOutput;
    delete mGridLayout;
    delete mMainWindow;
    delete mApp;
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

    gConsoleWindow->AddText(buffer);
    return(0);
}

// -- returns false if we should break
bool8 AssertHandler(const char* condition, const char* file,
                                           int32 linenumber, const char* fmt, ...) {
    if(!gScriptContext->IsAssertStackSkipped() || gScriptContext->IsAssertEnableTrace()) {
        if(!gScriptContext->IsAssertStackSkipped())
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

        if(!gScriptContext->IsAssertStackSkipped())
            ConsolePrint("*************************************************************\n");
        if(!gScriptContext->IsAssertStackSkipped()) {
            bool press_skip = false;
            bool press_trace = false;
            bool result = PushAssertDialog(assertmsg, errormsg, press_skip, press_trace);
            gScriptContext->SetAssertEnableTrace(press_trace);
            gScriptContext->SetAssertStackSkipped(press_skip);
            return (result);
        }
    }

    // -- handled - return true so we don't break
    return true;
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

    gScriptContext->ExecCommand(input_text);
    setText("");
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
        gScriptContext->Update(mCurrentTime);
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
    extern bool8 gUnitTestIncludeMe;
    gUnitTestIncludeMe = true;

    // -- initialize
    gScriptContext = TinScript::CScriptContext::Create("", ConsolePrint, AssertHandler);

    gConsoleWindow = new CConsoleWindow();
    int result = gConsoleWindow->Exec();

    // -- shutdown
    TinScript::CScriptContext::Destroy(gScriptContext);

    return result;
}

// ------------------------------------------------------------------------------------------------
void Quit() {
    gConsoleWindow->Quit();
}

void Pause() {
    gConsoleWindow->Pause();
}

void Unpause() {
    gConsoleWindow->Unpause();
}

REGISTER_FUNCTION_P0(Quit, Quit, void);
REGISTER_FUNCTION_P0(Pause, Pause, void);
REGISTER_FUNCTION_P0(Unpause, Unpause, void);

void Print(const char* string) {
    ConsolePrint(string);
}

REGISTER_FUNCTION_P1(Print, Print, void, const char*);

// ------------------------------------------------------------------------------------------------
#include "TinQTMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
