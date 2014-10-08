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
#include <QDockWidget>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextlist>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <qcolor.h>
#include <QShortcut>

#include "qmainwindow.h"
#include "qmetaobject.h"
#include "qevent.h"

#include "TinScript.h"
#include "TinRegistration.h"
#include "socket.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTWatchWin.h"

#include "mainwindow.h"

// ------------------------------------------------------------------------------------------------
// -- override the macro from integration.h
#undef TinPrint
#define TinPrint Print

bool PushDebuggerAssertDialog(const char* assertmsg, bool& ignore);

// ------------------------------------------------------------------------------------------------
CConsoleWindow* CConsoleWindow::gConsoleWindow = NULL;
CConsoleWindow::CConsoleWindow()
{
    // -- set the singleton
    gConsoleWindow = this;

    // -- create the Qt application components
    int argcount = 0;
    mApp = new QApplication(argcount, NULL);
    mApp->setOrganizationName("QtProject");
    mApp->setApplicationName("TinConsole");

    // -- create the main window
    QMap<QString, QSize> customSizeHints;
    mMainWindow = new MainWindow(customSizeHints);
    mMainWindow->resize(QSize(1200, 800));

    // -- create the output widget
    QDockWidget* outputDockWidget = new QDockWidget();
    outputDockWidget->setObjectName("Console Output");
    outputDockWidget->setWindowTitle("Console Output");
    mConsoleOutput = new CConsoleOutput(outputDockWidget);
    mConsoleOutput->addItem("Welcome to the TinConsole!");

    // -- create the consoleinput
    mConsoleInput = new CConsoleInput(outputDockWidget);
    mConsoleInput->setFixedHeight(24);

    // -- create the IPConnect
    mStatusLabel = new QLabel("Not Connected");
    mStatusLabel->setFixedWidth(120);
    mTargetInfoLabel = new QLabel("");
    mIPLabel = new QLabel("IP:");
    mConnectIP = new QLineEdit();
    mConnectIP->setText("127.0.0.1");
    mButtonConnect = new QPushButton();
    mButtonConnect->setText("Connect");
    mButtonConnect->setGeometry(0, 0, 32, 24); 

    // -- color the pushbutton
    mButtonConnect->setAutoFillBackground(true);
	QPalette myPalette = mButtonConnect->palette();
	myPalette.setColor(QPalette::Button, Qt::red);	
	mButtonConnect->setPalette(myPalette);

    mMainWindow->statusBar()->addWidget(mStatusLabel, 1);
    mMainWindow->statusBar()->addWidget(mTargetInfoLabel, 1);
    mMainWindow->statusBar()->addWidget(mIPLabel, 0);
    mMainWindow->statusBar()->addWidget(mConnectIP, 0);
    mMainWindow->statusBar()->addWidget(mButtonConnect, 0);

    // -- create the toolbar components
    QToolBar* toolbar = new QToolBar();
	toolbar->setWindowTitle("Debug Toolbar");
    QLabel* file_label = new QLabel("File:");
    QWidget* spacer_0a = new QWidget();
    spacer_0a->setFixedWidth(8);
    mFileLineEdit = new QLineEdit();
    mFileLineEdit->setFixedWidth(200);
    QWidget* spacer_0 = new QWidget();
    spacer_0->setFixedWidth(16);
    mButtonRun = new QPushButton();
    mButtonRun->setText("Run");
    mButtonRun->setGeometry(0, 0, 32, 24); 
    mButtonStep = new QPushButton();
    mButtonStep->setText("Step");
    mButtonStep->setGeometry(0, 0, 32, 24);
    mButtonStepIn = new QPushButton();
    mButtonStepIn->setText("Step In");
    mButtonStepIn->setGeometry(0, 0, 32, 24);
    QWidget* spacer_1 = new QWidget();
    spacer_1->setFixedWidth(16);
    QLabel* find_label = new QLabel("Find:");
    QWidget* spacer_1a = new QWidget();
    spacer_1a->setFixedWidth(8);
    mFindLineEdit = new QLineEdit();
    mFindLineEdit->setFixedWidth(200);
    QWidget* spacer_2 = new QWidget();
    spacer_2->setFixedWidth(8);
    mFindResult = new QLabel("Ctrl + F");
    mFindResult->setFixedWidth(120);
    mFindResult->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    toolbar->addWidget(file_label);
    toolbar->addWidget(spacer_0a);
    toolbar->addWidget(mFileLineEdit);
    toolbar->addWidget(spacer_0);
    toolbar->addWidget(mButtonRun);
    toolbar->addWidget(mButtonStep);
    toolbar->addWidget(mButtonStepIn);
    toolbar->addWidget(spacer_1);
    toolbar->addWidget(find_label);
    toolbar->addWidget(spacer_1a);
    toolbar->addWidget(mFindLineEdit);
    toolbar->addWidget(spacer_2);
    toolbar->addWidget(mFindResult);
    mMainWindow->addToolBar(toolbar);

    // -- create the source window
    QDockWidget* sourceWinDockWidget = new QDockWidget();
    sourceWinDockWidget->setObjectName("Source View");
    sourceWinDockWidget->setWindowTitle("Source View");
    mDebugSourceWin = new CDebugSourceWin(sourceWinDockWidget);

    // -- create the callstack window
    QDockWidget* callstackDockWidget = new QDockWidget();
    callstackDockWidget->setObjectName("Call Stack");
    callstackDockWidget->setWindowTitle("Call Stack");
    mCallstackWin = new CDebugCallstackWin(callstackDockWidget);

    // -- create the breakpoints window
    QDockWidget* breakpointsDockWidget = new QDockWidget();
    breakpointsDockWidget->setObjectName("Breakpoints");
    breakpointsDockWidget->setWindowTitle("Breakpoints");
    mBreakpointsWin = new CDebugBreakpointsWin(breakpointsDockWidget);

    // -- create the autos window
    QDockWidget* autosDockWidget = new QDockWidget();
    autosDockWidget->setObjectName("Autos");
    autosDockWidget->setWindowTitle("Autos");
    mAutosWin = new CDebugWatchWin(autosDockWidget);

    // -- create the watches window
    QDockWidget* watchesDockWidget = new QDockWidget();
    watchesDockWidget->setObjectName("Watches");
    watchesDockWidget->setWindowTitle("Watches");
    mWatchesWin = new CDebugWatchWin(watchesDockWidget);

    // -- connect the widgets
    QObject::connect(mButtonConnect, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonConnectPressed()));
    QObject::connect(mConnectIP, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnConnectIPReturnPressed()));

    QObject::connect(mConsoleInput, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnReturnPressed()));

    QObject::connect(mDebugSourceWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mDebugSourceWin,
                                      SLOT(OnDoubleClicked(QListWidgetItem*)));

    QObject::connect(mFileLineEdit, SIGNAL(returnPressed()), mConsoleInput,
                                    SLOT(OnFileEditReturnPressed()));

    QObject::connect(mButtonRun, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonRunPressed()));
    QObject::connect(mButtonStep, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonStepPressed()));
    QObject::connect(mButtonStepIn, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonStepInPressed()));

    QObject::connect(mFindLineEdit, SIGNAL(returnPressed()), mConsoleInput,
                                    SLOT(OnFindEditReturnPressed()));

    QObject::connect(mCallstackWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mCallstackWin,
                                    SLOT(OnDoubleClicked(QListWidgetItem*)));

    QObject::connect(mBreakpointsWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mBreakpointsWin,
                                      SLOT(OnDoubleClicked(QListWidgetItem*)));
    QObject::connect(mBreakpointsWin, SIGNAL(itemClicked(QListWidgetItem*)), mBreakpointsWin,
                                      SLOT(OnClicked(QListWidgetItem*)));

    // -- hotkeys

    // Ctrl + Break - Run
    QShortcut* shortcut_Break = new QShortcut(QKeySequence("Shift+F5"), mConsoleInput);
    QObject::connect(shortcut_Break, SIGNAL(activated()), mConsoleInput, SLOT(OnButtonStopPressed()));

    // F5 - Run
    QShortcut* shortcut_Run = new QShortcut(QKeySequence("F5"), mButtonRun);
    QObject::connect(shortcut_Run, SIGNAL(activated()), mConsoleInput, SLOT(OnButtonRunPressed()));

    // F10 - Step
    QShortcut* shortcut_Step = new QShortcut(QKeySequence("F10"), mButtonStep);
    QObject::connect(shortcut_Step, SIGNAL(activated()), mConsoleInput, SLOT(OnButtonStepPressed()));

    // F11 - Step In
    QShortcut* shortcut_StepIn = new QShortcut(QKeySequence("F11"), mButtonStepIn);
    QObject::connect(shortcut_StepIn, SIGNAL(activated()), mConsoleInput, SLOT(OnButtonStepInPressed()));

    // Shift + F11 - Step Out
    QShortcut* shortcut_StepOut = new QShortcut(QKeySequence("Shift+F11"), mButtonStepIn);
    QObject::connect(shortcut_StepOut, SIGNAL(activated()), mConsoleInput, SLOT(OnButtonStepOutPressed()));

    // Ctrl + w - Add variable watch
    QShortcut* shortcut_AddVar = new QShortcut(QKeySequence("Ctrl+W"), mMainWindow);
    QObject::connect(shortcut_AddVar, SIGNAL(activated()), mMainWindow, SLOT(menuAddVariableWatch()));

    // Ctrl + Shift + w - Watch Variable
    QShortcut* shortcut_WatchVar = new QShortcut(QKeySequence("Ctrl+Shift+W"), mMainWindow);
    QObject::connect(shortcut_WatchVar, SIGNAL(activated()), mMainWindow, SLOT(menuCreateVariableWatch()));

    // Ctrl + Shift + b - Break condition
    QShortcut* shortcut_BreakCond = new QShortcut(QKeySequence("Ctrl+Shift+B"), mMainWindow);
    QObject::connect(shortcut_BreakCond, SIGNAL(activated()), mMainWindow, SLOT(menuSetBreakCondition()));

    // Ctrl + g - Go to line in the source view
    QShortcut* shortcut_GotoLine = new QShortcut(QKeySequence("Ctrl+G"), mMainWindow);
    QObject::connect(shortcut_GotoLine, SIGNAL(activated()), mMainWindow, SLOT(menuGoToLine()));

    // Ctrl + f - Search
    QShortcut* shortcut_Search = new QShortcut(QKeySequence("Ctrl+F"), mMainWindow);
    QObject::connect(shortcut_Search, SIGNAL(activated()), mConsoleInput, SLOT(OnFindEditFocus()));

    // F3 - Search again
    QShortcut* shortcut_SearchAgain = new QShortcut(QKeySequence("F3"), mMainWindow);
    QObject::connect(shortcut_SearchAgain, SIGNAL(activated()), mConsoleInput, SLOT(OnFindEditReturnPressed()));

    mMainWindow->addDockWidget(Qt::TopDockWidgetArea, sourceWinDockWidget);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, outputDockWidget);
    mMainWindow->addDockWidget(Qt::TopDockWidgetArea, callstackDockWidget);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, breakpointsDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, autosDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, watchesDockWidget);

    mMainWindow->show();

    // -- restore the last used layout
    mMainWindow->autoLoadLayout();

    // -- initialize the breakpoint members
    mBreakpointHit = false;
    mBreakpointCodeblockHash = 0;
    mBreakpointLinenumber = -1;

    // -- initialize the assert messages
    mAssertTriggered = false;
    mAssertMessage[0] = '\0';

    mBreakpointRun = false;
    mBreakpointStep = false;
    mBreakpointStepIn = false;
    mBreakpointStepOut = false;

    // -- initialize the running members
    mIsConnected = false;
}

CConsoleWindow::~CConsoleWindow()
{
}

int CConsoleWindow::Exec()
{
    return (mApp->exec());
}

// ------------------------------------------------------------------------------------------------
// -- create a handler to register, so we can receive print messages and asserts
bool PushAssertDialog(const char* assertmsg, const char* errormsg, bool& skip, bool& trace);

int ConsolePrint(const char* fmt, ...)
{
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
                    const char* file, int32 linenumber, const char* fmt, ...)
{
    if (!script_context->IsAssertStackSkipped() || script_context->IsAssertEnableTrace())
    {
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
    return (true);
}

// ------------------------------------------------------------------------------------------------
void PushBreakpointDialog(const char* breakpoint_msg)
{
    QMessageBox msgBox;
    msgBox.setModal(true);
    msgBox.setText(breakpoint_msg);
    msgBox.show();
}

// ------------------------------------------------------------------------------------------------
void CConsoleWindow::NotifyOnConnect()
{
    // -- set the bool
    mIsConnected = true;

    // -- set the connect button color and text
    mButtonConnect->setAutoFillBackground(true);
	QPalette myPalette = mButtonConnect->palette();
	myPalette.setColor(QPalette::Button, Qt::green);	
	mButtonConnect->setPalette(myPalette);
    mButtonConnect->setText("Disconnect");

    // -- set the status message
    SetStatusMessage("Connected");
    SetTargetInfoMessage("");

    // -- send the text command to identify this connection as for a debugger
    SocketManager::SendCommand("DebuggerSetConnected(true);");

    // -- resend our list of breakpoints
    GetDebugBreakpointsWin()->NotifyOnConnect();

    // -- add some details
    ConsolePrint("*******************************************************************");
    ConsolePrint("*                                 Debugger Connected                                     *");
    ConsolePrint("*  All console input will be redirected to the connected target.  *");
    ConsolePrint("*******************************************************************");
}

void CConsoleWindow::NotifyOnDisconnect()
{
    // -- set the bool
    mIsConnected = false;

    // -- set the connect button color and text
    mButtonConnect->setAutoFillBackground(true);
	QPalette myPalette = mButtonConnect->palette();
	myPalette.setColor(QPalette::Button, Qt::red);	
	mButtonConnect->setPalette(myPalette);
    mButtonConnect->setText("Connect");

    // -- set the status message
    SetStatusMessage("Not Connected");
    SetTargetInfoMessage("");
}

void CConsoleWindow::NotifyOnClose()
{
    // -- disconnect
    SocketManager::Disconnect();
}

// ====================================================================================================================
// ToggleBreakpoint():  Central location to forward the request to both the source win and the breakpoints win
// ====================================================================================================================
void CConsoleWindow::ToggleBreakpoint(uint32 codeblock_hash, int32 line_number, bool add, bool enable)
{
    const char* filename = TinScript::UnHash(codeblock_hash);
    if (!filename)
        return;

    // -- notify the Source Window
    GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, line_number, add, enable);

    // -- notify the Breakpoints Window
    GetDebugBreakpointsWin()->ToggleBreakpoint(codeblock_hash, line_number, add);
}

// ====================================================================================================================
// NotifyBreakpointHit():  A breakpoint hit was found during processing of the data packets
// ====================================================================================================================
void CConsoleWindow::NotifyBreakpointHit(int32 watch_request_id, uint32 codeblock_hash, int32 line_number)
{
    // -- set the bool
    mBreakpointHit = true;

    // -- cache the breakpoint details
    mBreakpointWatchRequestID = watch_request_id;
    mBreakpointCodeblockHash = codeblock_hash;
    mBreakpointLinenumber = line_number;
}

// ====================================================================================================================
// HasBreakpoint():  Returns true, if a breakpoint hit is pending, along with the specific codeblock / line number
// ====================================================================================================================
bool CConsoleWindow::HasBreakpoint(int32& watch_request_id, uint32& codeblock_hash, int32& line_number)
{
    // -- no breakpoint - return false
    if (!mBreakpointHit)
        return (false);

    // -- fill in the details
    watch_request_id = mBreakpointWatchRequestID;
    codeblock_hash = mBreakpointCodeblockHash;
    line_number = mBreakpointLinenumber;

    // -- we have a pending breakpoint
    return (true);
}

// ====================================================================================================================
// HandleBreakpointHit():  Enters a loop processing events, until "run" or "step" are pressed by the user
// ====================================================================================================================
void CConsoleWindow::HandleBreakpointHit(const char* breakpoint_msg)
{
    // -- clear the flag, and initialize the action bools
    mBreakpointRun = false;
    mBreakpointStep = false;
    mBreakpointStepIn = false;
    mBreakpointStepOut = false;

    // -- highlight the button in red
    mButtonRun->setAutoFillBackground(true);
	QPalette myPalette = mButtonRun->palette();
	myPalette.setColor(QPalette::Button, Qt::red);	
	mButtonRun->setPalette(myPalette);

    // -- set the status message
    if (mBreakpointWatchRequestID > 0)
        SetStatusMessage("Break on watch");
    else
        SetStatusMessage("Breakpoint");

    // -- set the currently selected breakpoint
    if (mBreakpointWatchRequestID == 0)
        GetDebugBreakpointsWin()->SetCurrentBreakpoint(mBreakpointCodeblockHash, mBreakpointLinenumber);
    else
        GetDebugBreakpointsWin()->SetCurrentVarWatch(mBreakpointWatchRequestID);

    while (SocketManager::IsConnected() && !mBreakpointRun &&
           !mBreakpointStep && !mBreakpointStepIn && !mBreakpointStepOut)
    {
        QCoreApplication::processEvents();

        // -- update our own environment, especially receiving data packetes
        GetOutput()->DebuggerUpdate();
    }

    // -- reset the "hit" flag
    mBreakpointHit = false;

    // -- set the status message
    if (SocketManager::IsConnected())
        SetStatusMessage("Connected");
    else
    {
        SetStatusMessage("Not Connected");
        SetTargetInfoMessage("");
    }

    // -- back to normal color
	myPalette.setColor(QPalette::Button, Qt::transparent);	
	mButtonRun->setPalette(myPalette);

    // -- clear the callstack and the watch window
    GetDebugCallstackWin()->ClearCallstack();

    // -- we need to notify the watch win that we're not currently broken
    GetDebugAutosWin()->NotifyEndOfBreakpoint();
}

// ====================================================================================================================
// NotifyAssertTriggered():  An assert was found during processing of the data packets
// ====================================================================================================================
void CConsoleWindow::NotifyAssertTriggered(const char* assert_msg, uint32 codeblock_hash, int32 line_number)
{
    // -- set the bool
    mAssertTriggered = true;

    strcpy_s(mAssertMessage, assert_msg);
    mBreakpointCodeblockHash = codeblock_hash;
    mBreakpointLinenumber = line_number;
}

// ====================================================================================================================
// HasAssert():  Returns true, if an assert is pending, along with the specific codeblock / line number
// ====================================================================================================================
bool CConsoleWindow::HasAssert(const char*& assert_msg, uint32& codeblock_hash, int32& line_number)
{
    // -- no assert - return false
    if (!mAssertTriggered)
        return (false);

    // -- fill in the details
    assert_msg = mAssertMessage;
    codeblock_hash = mBreakpointCodeblockHash;
    line_number = mBreakpointLinenumber;

    // -- we have a pending breakpoint
    return (true);
}

// ====================================================================================================================
// ClearAssert():  The assert is or has been handled
// ====================================================================================================================
void CConsoleWindow::ClearAssert(bool8 set_break)
{
    // -- clear the flag
    mAssertTriggered = false;

    // -- if we're supposed to proceed by setting a breakpoint, set the flag
    if (set_break)
    {
        mBreakpointHit = true;
    }
    else
    {
        // -- send the message to run
        SocketManager::SendCommand("DebuggerBreakRun();");
    }
}

// == Global Interface ================================================================================================

// ====================================================================================================================
// DebuggerBreakpointHit():  Registered function, called from the virtual machine via the SocketManager
// ====================================================================================================================
void DebuggerBreakpointHit(int32 codeblock_hash, int32 line_number)
{
    bool8 press_ignore_run = false;
    bool8 press_trace_step = false;
    char breakpoint_msg[256];
    sprintf_s(breakpoint_msg, 256, "Break on line: %d", line_number);

    // -- set the PC
    CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(static_cast<uint32>(codeblock_hash), line_number);

    // -- loop until we press either "Run" or "Step"
    CConsoleWindow::GetInstance()->HandleBreakpointHit(breakpoint_msg);

    // -- clear the PC
    CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(static_cast<uint32>(codeblock_hash), -1);

    // -- return true if we're supposed to keep running
    if (CConsoleWindow::GetInstance()->mBreakpointRun)
    {
        // -- send the message to run
        SocketManager::SendCommand("DebuggerBreakRun();");
    }

    // -- otherwise we're supposed to step
    // -- NOTE:  stepping *in* is the default... step over and step out are the exceptions
    else if (CConsoleWindow::GetInstance()->mBreakpointStepIn)
    {
        SocketManager::SendCommand("DebuggerBreakStep(false, false);");
    }
    else if (CConsoleWindow::GetInstance()->mBreakpointStepOut)
    {
        SocketManager::SendCommand("DebuggerBreakStep(false, true);");
    }
    else
    {
        SocketManager::SendCommand("DebuggerBreakStep(true, false);");
    }
}

// ====================================================================================================================
// DebuggerConfirmBreakpoint():  Corrects the requested breakpoint to the actual breakable line
// ====================================================================================================================
void DebuggerConfirmBreakpoint(int32 filename_hash, int32 line_number, int32 actual_line)
{
    // -- notify the breakpoints window
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->NotifyConfirmBreakpoint(filename_hash, line_number,
                                                                                     actual_line);
}

// ====================================================================================================================
// DebuggerConfirmVarWatch():  Confirms the object ID and var_name_hash for a requested variable watch.
// ====================================================================================================================
void DebuggerConfirmVarWatch(int32 watch_request_id, uint32 watch_object_id, uint32 var_name_hash)
{
    // -- notify the breakpoints window
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->NotifyConfirmVarWatch(watch_request_id, watch_object_id,
                                                                                   var_name_hash);
}

// ====================================================================================================================
// DebuggerNotifyCallstack():  Called directly from the SocketManager registered RecvPacket function
// ====================================================================================================================
void DebuggerNotifyCallstack(uint32* codeblock_array, uint32* objid_array, uint32* namespace_array,
                             uint32* func_array, uint32* linenumber_array, int array_size)
{
    CConsoleWindow::GetInstance()->GetDebugCallstackWin()->NotifyCallstack(codeblock_array, objid_array,
                                                                           namespace_array, func_array,
                                                                           linenumber_array, array_size);

    // -- we need to notify the watch window, we have a new callstack
    CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyUpdateCallstack(true);
}

// ====================================================================================================================
// NotifyCodeblockLoaded():  Called by the executable when a new codeblock is added, allowing breakpoints, etc...
// ====================================================================================================================
void NotifyCodeblockLoaded(const char* filename)
{
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->NotifyCodeblockLoaded(filename);

    // -- breakpoints are keyed from hash values
    uint32 codeblock_hash = TinScript::Hash(filename);
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->NotifyCodeblockLoaded(codeblock_hash);
}

// ====================================================================================================================
// NotifyCodeblockLoaded():  Called upon connection, so looking for file source text matches the target
// ====================================================================================================================
void NotifyCurrentDir(const char* cwd)
{
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->NotifyCurrentDir(cwd);

    // -- set the status message
    char msg[1024];
    sprintf_s(msg, "Target Dir: %s", cwd ? cwd : "./");
    CConsoleWindow::GetInstance()->SetTargetInfoMessage(msg);
}

// ====================================================================================================================
// NotifyWatchVarEntry():  Called when a variable is being watched (e.g. autos, during a breakpoint)
// ====================================================================================================================
void NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry)
{
    CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyWatchVarEntry(watch_var_entry);
}
        
// ====================================================================================================================
// AddText():  Adds a message to the console output window.
// ====================================================================================================================
void CConsoleWindow::AddText(char* msg)
{
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

// ====================================================================================================================
// SetStatusMessage():  Sets the messsage in the status bar, at the bottom of the application window.
// ====================================================================================================================
void CConsoleWindow::SetStatusMessage(const char* message)
{
    // -- ensure we have a message
    if (!message)
        message = "";

    if (mStatusLabel)
        mStatusLabel->setText(QString(message));
}

// ====================================================================================================================
// SetTargetInfoMessage():  Sets the messsage in the status bar, for any target info
// ====================================================================================================================
void CConsoleWindow::SetTargetInfoMessage(const char* message)
{
    // -- ensure we have a message
    if (!message)
        message = "";

    if (mTargetInfoLabel)
        mTargetInfoLabel->setText(QString(message));
}

// ------------------------------------------------------------------------------------------------
CConsoleInput::CConsoleInput(QWidget* parent) : QLineEdit(parent)
{
    // -- q&d history implementation
    mHistoryFull = false;
    mHistoryIndex = -1;
    mHistoryLastIndex = -1;
    for(int32 i = 0; i < kMaxHistory; ++i)
        *mHistory[i] = '\0';

    // -- create the label as well
    mInputLabel = new QLabel("==>", parent);
}

void CConsoleInput::OnButtonConnectPressed()
{
    // -- if we're trying to connect...
    if (!SocketManager::IsConnected())
    {
        // -- connect, same as pressing return from the connect IP
        OnConnectIPReturnPressed();
    }

    // -- otherwise, disconnect
    else
    {
        SocketManager::Disconnect();
    }
}

void CConsoleInput::OnConnectIPReturnPressed()
{
    // -- if we're not connected, try to connect
    if (!SocketManager::IsConnected())
    {
        QByteArray ip_ba = CConsoleWindow::GetInstance()->GetConnectIP()->text().toUtf8();
        const char* ip_text = ip_ba.data();
        SocketManager::Connect(ip_text);
    }

    // -- else disconnect
    else 
    {
        SocketManager::Disconnect();
    }
}

void CConsoleInput::OnReturnPressed()
{
    QByteArray input_ba = text().toUtf8();
    const char* input_text = input_ba.data();

    bool8 is_connected = CConsoleWindow::GetInstance()->IsConnected();
    if (is_connected)
        ConsolePrint("SEND> %s", input_text);
    else
        ConsolePrint("> %s", input_text);

    // -- add this to the history buf
    const char* historyptr = (mHistoryLastIndex < 0) ? NULL : mHistory[mHistoryLastIndex];
    if(input_text[0] != '\0' && (!historyptr || strcmp(historyptr, input_text) != 0)) {
        mHistoryFull = mHistoryFull || mHistoryLastIndex == kMaxHistory - 1;
        mHistoryLastIndex = (mHistoryLastIndex + 1) % kMaxHistory;
        strcpy_s(mHistory[mHistoryLastIndex], TinScript::kMaxTokenLength, input_text);
    }
    mHistoryIndex = -1;

    if (is_connected)
        SocketManager::SendCommand(input_text);
    else
        TinScript::ExecCommand(input_text);
    setText("");
}

void CConsoleInput::OnFileEditReturnPressed() {
    QLineEdit* fileedit = CConsoleWindow::GetInstance()->GetFileLineEdit();
    QString filename = fileedit->text();
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->OpenSourceFile(filename.toUtf8(), true);
}

void CConsoleInput::OnButtonStopPressed()
{
    if (!CConsoleWindow::GetInstance()->mBreakpointHit)
    {
        SocketManager::SendDebuggerBreak();
    }
}

void CConsoleInput::OnButtonRunPressed()
{
    CConsoleWindow::GetInstance()->mBreakpointRun = true;
}

void CConsoleInput::OnButtonStepPressed()
{
    CConsoleWindow::GetInstance()->mBreakpointStep = true;
}

void CConsoleInput::OnButtonStepInPressed()
{
    CConsoleWindow::GetInstance()->mBreakpointStepIn = true;
}

void CConsoleInput::OnButtonStepOutPressed()
{
    CConsoleWindow::GetInstance()->mBreakpointStepOut = true;
}

void CConsoleInput::OnFindEditFocus()
{
    QLineEdit* find_edit = CConsoleWindow::GetInstance()->GetFindLineEdit();
    find_edit->setFocus();
}

void CConsoleInput::OnFindEditReturnPressed()
{
    QLineEdit* find_edit = CConsoleWindow::GetInstance()->GetFindLineEdit();
    QString search_string = find_edit->text();
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->FindInFile(search_string.toUtf8());
}

// ------------------------------------------------------------------------------------------------
void CConsoleInput::keyPressEvent(QKeyEvent * event)
{
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

    else
    {
        QLineEdit::keyPressEvent(event);
    }
}

// ------------------------------------------------------------------------------------------------
CConsoleOutput::CConsoleOutput(QWidget* parent) : QListWidget(parent)
{
    mCurrentTime = 0;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(Update()));
    mTimer->start(kUpdateTime);
}

CConsoleOutput::~CConsoleOutput() {
    delete mTimer;
}

void CConsoleOutput::Update()
{
    // -- see if we've become either connected, or disconnected
    bool isConnected = SocketManager::IsConnected();
    if (CConsoleWindow::GetInstance()->IsConnected() != isConnected)
    {
        // -- if we've become connected, notify the main window
        if (isConnected)
            CConsoleWindow::GetInstance()->NotifyOnConnect();
        else
            CConsoleWindow::GetInstance()->NotifyOnDisconnect();
    }

    // -- process the received packets
    ProcessDataPackets();

    // -- see if we an assert was triggered
    int32 watch_request_id = 0;
    uint32 codeblock_hash = 0;
    int32 line_number = -1;
    const char* assert_msg = "";
    bool hasAssert = CConsoleWindow::GetInstance()->HasAssert(assert_msg, codeblock_hash, line_number);
    if (hasAssert)
    {
        // -- set the PC
        CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(codeblock_hash, line_number);

        // -- if the response to the assert is to break, then we'll drop down and handle a breakpoint
        bool ignore = false;
        PushDebuggerAssertDialog(assert_msg, ignore);

        // -- clear the PC
        CConsoleWindow::GetInstance()->mDebugSourceWin->SetCurrentPC(codeblock_hash, -1);

        // -- clear the assert flag
        CConsoleWindow::GetInstance()->ClearAssert(!ignore);
    }

    // -- see if we have a breakpoint
    bool hasBreakpoint = CConsoleWindow::GetInstance()->HasBreakpoint(watch_request_id, codeblock_hash, line_number);
    
    // -- this will notify all required windows, and loop until the the breakpoint has been handled
    if (hasBreakpoint)
    {
        DebuggerBreakpointHit(codeblock_hash, line_number);
    }

    // -- if we're not paused, update TinScript
    mCurrentTime += kUpdateTime;
    TinScript::UpdateContext(mCurrentTime);
}

// ====================================================================================================================
// DebuggerUpdate():  An abbreviated update to keep us alive while we're handing a breakpoint
// ====================================================================================================================
void CConsoleOutput::DebuggerUpdate()
{
    // -- process the received packets
    ProcessDataPackets();
}

// ====================================================================================================================
// ReceiveDataPackets():  Threadsafe method to queue a packet, to be process during update
// ====================================================================================================================
void CConsoleOutput::ReceiveDataPacket(SocketManager::tDataPacket* packet)
{
    // -- ensure we have something to receive
    if (!packet)
        return;

    // -- note:  the packet is received from the Socket's update thread, not the main thread
    // -- this must be thread safe
    mThreadLock.Lock();

    // -- push the packet onto the queue
    mReceivedPackets.push_back(packet);

    // -- unlock the thread
    mThreadLock.Unlock();
}

// ====================================================================================================================
// ProcessDataPackets():  During the update loop, we'll process the packets we've received
// ====================================================================================================================
void CConsoleOutput::ProcessDataPackets()
{
    // -- note:  the packet is received from the Socket's update thread, not the main thread
    // -- this is only to be called during the main thread
    mThreadLock.Lock();

    while (mReceivedPackets.size() > 0)
    {
        // -- dequeue the packet
        SocketManager::tDataPacket* packet = mReceivedPackets[0];
        mReceivedPackets.erase(mReceivedPackets.begin());

        // -- see what type of data packet we received
        int32* dataPtr = (int32*)packet->mData;
    
        // -- get the ID of the packet, as defined in the USER CONSTANTS at the top of socket.h
        int32 dataType = *dataPtr;

        // -- see if we have a handler for this packet
        switch (dataType)
        {
            case k_DebuggerCurrentWorkingDirPacketID:
                HandlePacketCurrentWorkingDir(dataPtr);
                break;

            case k_DebuggerCodeblockLoadedPacketID:
                HandlePacketCodeblockLoaded(dataPtr);
                break;

            case k_DebuggerBreakpointHitPacketID:
                HandlePacketBreakpointHit(dataPtr);
                break;

            case k_DebuggerBreakpointConfirmPacketID:
                HandlePacketBreakpointConfirm(dataPtr);
                break;

            case k_DebuggerVarWatchConfirmPacketID:
                HandlePacketVarWatchConfirm(dataPtr);
                break;

            case k_DebuggerCallstackPacketID:
                HandlePacketCallstack(dataPtr);
                break;

            case k_DebuggerWatchVarEntryPacketID:
                HandlePacketWatchVarEntry(dataPtr);
                break;

            case k_DebuggerAssertMsgPacketID:
                HandlePacketAssertMsg(dataPtr);
                break;

            case k_DebuggerPrintMsgPacketID:
                HandlePacketPrintMsg(dataPtr);
                break;

            default:
                break;
        }

        // -- the callback is required to manage the packet memory itself
        delete packet;
    }

    // -- unlock the thread
    mThreadLock.Unlock();
}
// ====================================================================================================================
// HandlePacketCurrentWorkingDir():  A callback handler for a packet of type "current working directory"
// ====================================================================================================================
void CConsoleOutput::HandlePacketCurrentWorkingDir(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- notification is of the filename, as the codeblock hash may not yet be in the dictionary
    const char* cwd = (const char*)dataPtr;

    // -- notify the debugger
    NotifyCurrentDir(cwd);
}

// ====================================================================================================================
// HandlePacketCodeblockLoaded():  A callback handler for a packet of type "codeblock loaded"
// ====================================================================================================================
void CConsoleOutput::HandlePacketCodeblockLoaded(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- notification is of the filename, as the codeblock hash may not yet be in the dictionary
    const char* filename = (const char*)dataPtr;

    // -- notify the debugger
    NotifyCodeblockLoaded(filename);
}

// ====================================================================================================================
// HandlePacketBreakpointHit():  A callback handler for a packet of type "breakpoint hit"
// ====================================================================================================================
void CConsoleOutput::HandlePacketBreakpointHit(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the watch request id
    uint32 watch_request_id = *dataPtr++;

    // -- get the codeblock has
    uint32 codeblock_hash = *dataPtr++;

    // -- get the line number
    int32 line_number = *dataPtr++;

    // -- notify the debugger
    CConsoleWindow::GetInstance()->NotifyBreakpointHit(watch_request_id, codeblock_hash, line_number);
    CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyBreakpointHit();
    CConsoleWindow::GetInstance()->GetDebugWatchesWin()->NotifyBreakpointHit();
}

// ====================================================================================================================
// HandlePacketBreakpointConfirm():  A callback handler for a packet of type "breakpoint confirm"
// ====================================================================================================================
void CConsoleOutput::HandlePacketBreakpointConfirm(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the codeblock has
    uint32 codeblock_hash = *dataPtr++;

    // -- get the line number
    int32 line_number = *dataPtr++;

    // -- get the correctedline number
    int32 actual_line = *dataPtr++;

    // -- notifiy the debugger
    DebuggerConfirmBreakpoint(codeblock_hash, line_number, actual_line);
}

// ====================================================================================================================
// HandlePacketVarWatchConfirm():  A callback handler for a packet of type "variable watch confirm"
// ====================================================================================================================
void CConsoleOutput::HandlePacketVarWatchConfirm(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the watch request ID
    int32 watch_request_id = *dataPtr++;

    // -- get the watch object id
    uint32 watch_object_id = *dataPtr++;

    // -- get the watch var name hash
    int32 var_name_hash = *dataPtr++;

    // -- notifiy the debugger
    DebuggerConfirmVarWatch(watch_request_id, watch_object_id, var_name_hash);
}

// ====================================================================================================================
// HandlePacketCallstack():  A callback handler for a packet of type "callstack"
// ====================================================================================================================
void CConsoleOutput::HandlePacketCallstack(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the array size
    int32 array_size = *dataPtr++;

    // -- get the codeblock array
    uint32* codeblock_array = (uint32*)dataPtr;
    dataPtr += array_size;

    // -- get the objectID array
    uint32* objid_array = (uint32*)dataPtr;
    dataPtr += array_size;

    // -- get the namespace array
    uint32* namespace_array = (uint32*)dataPtr;
    dataPtr += array_size;

    // -- get the function ID array
    uint32* func_array = (uint32*)dataPtr;
    dataPtr += array_size;

    // -- get the line numbers array
    uint32* linenumber_array = (uint32*)dataPtr;
    dataPtr += array_size;

    // -- notify the debugger
    DebuggerNotifyCallstack(codeblock_array, objid_array, namespace_array, func_array, linenumber_array, array_size);
}

// ====================================================================================================================
// HandlePacketWatchVarEntry():  A handler for packet type "watch var entry"
// ====================================================================================================================
void CConsoleOutput::HandlePacketWatchVarEntry(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- reconstitute the stuct
    TinScript::CDebuggerWatchVarEntry watch_var_entry;

	// -- variable watch request ID (unused for stack dumps)
	watch_var_entry.mWatchRequestID = *dataPtr++;

    // -- function namespace hash
    watch_var_entry.mFuncNamespaceHash = *dataPtr++;

    // -- function hash
    watch_var_entry.mFunctionHash = *dataPtr++;

    // -- function hash
    watch_var_entry.mFunctionObjectID = *dataPtr++;

    // -- objectID (required, if this var is a member)
    watch_var_entry.mObjectID = *dataPtr++;

    // -- namespace hash (required, if this var is a member)
    watch_var_entry.mNamespaceHash = *dataPtr++;

    // -- var type
    watch_var_entry.mType = (TinScript::eVarType)(*dataPtr++);

    // -- array size
    watch_var_entry.mArraySize = (TinScript::eVarType)(*dataPtr++);

    // -- name string length
    int32 name_str_length = *dataPtr++;

    // -- copy the name string
    strcpy_s(watch_var_entry.mVarName, (const char*)dataPtr);
    dataPtr += (name_str_length / 4);

    // -- value string length
    int32 value_str_length = *dataPtr++;

    // -- copy the value string
    strcpy_s(watch_var_entry.mValue, (const char*)dataPtr);
    dataPtr += (value_str_length / 4);

    // -- cached var name hash
    watch_var_entry.mVarHash = *dataPtr++;

    // -- cached var object ID
    watch_var_entry.mVarObjectID = *dataPtr++;

    // -- notify the debugger - if we've got a request ID > 0, it's a watch variable, not an auto var
	if (watch_var_entry.mWatchRequestID > 0)
		CConsoleWindow::GetInstance()->GetDebugWatchesWin()->NotifyVarWatchResponse(&watch_var_entry);
	else
		CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyWatchVarEntry(&watch_var_entry);
}

// ====================================================================================================================
// HandlePacketAssertMsg():  A handler for packet type "assert"
// ====================================================================================================================
void CConsoleOutput::HandlePacketAssertMsg(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the length of the message string
    int32 msg_length = *dataPtr++;

    // -- set the const char*
    const char* assert_msg = (char*)dataPtr;
    dataPtr += (msg_length / 4);

    // -- get the codeblock hash
    uint32 codeblock_hash = *dataPtr++;

    // -- get the line number
    int32 line_number = *dataPtr++;

    // -- notify the debugger
    CConsoleWindow::GetInstance()->NotifyAssertTriggered(assert_msg, codeblock_hash, line_number);
}

// ====================================================================================================================
// HandlePacketPrintMsg():  A handler for packet type "print"
// ====================================================================================================================
void CConsoleOutput::HandlePacketPrintMsg(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- get the length of the message string
    int32 msg_length = *dataPtr++;

    // -- set the const char*
    const char* msg = (char*)dataPtr;
    dataPtr += (msg_length / 4);

    // -- add the message, preceeded with some indication that it's a remote message
    ConsolePrint("RECV> %s", msg);
}

// ====================================================================================================================
// PushAssertDialog():  Handler for a ScriptAssert_(), returns ignore/trace/break input
// ====================================================================================================================
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

// ====================================================================================================================
// PushDebuggerAssertDialog():  Handler for receiving an assert from our target
// ====================================================================================================================
bool PushDebuggerAssertDialog(const char* assertmsg, bool& ignore)
{
    QMessageBox msgBox;

    QString dialog_msg = QString(assertmsg);
    msgBox.setText(dialog_msg);

    ConsolePrint("RECV> *** ASSERT ***");
    ConsolePrint("RECV> %s", assertmsg);

    QPushButton *ignore_button = msgBox.addButton("Ignore", QMessageBox::ActionRole);
    QPushButton *break_button = msgBox.addButton("Break", QMessageBox::ActionRole);

    msgBox.exec();

    bool handled = false;
    if (msgBox.clickedButton() == ignore_button)
    {
        ignore = true;
        handled = true;
    }

    else if (msgBox.clickedButton() == break_button)
    {
        ignore = false;
        handled = true;
    }

    return (handled);
}

// ====================================================================================================================
// DebuggerRecvDataCallback():  Registered with the SocketManager, to directly process packets of type DATA
// ====================================================================================================================
static void DebuggerRecvDataCallback(SocketManager::tDataPacket* packet)
{
    // -- nothing to process if we have no packet
    if (!packet || !packet->mData)
        return;

    // -- let the CConsoleOutput (which owns the update loop) deal with it
    CConsoleWindow::GetInstance()->GetOutput()->ReceiveDataPacket(packet);
}

// --------------------------------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
    // -- required to ensure registered functions from unittest.cpp are linked.
    REGISTER_FILE(unittest_cpp);
    REGISTER_FILE(mathutil_cpp);

    // -- initialize (true for MainThread context)
    TinScript::CreateContext(ConsolePrint, AssertHandler, true);

    // -- initialize the socket manager, for remote debugging
    SocketManager::Initialize();

    // -- register the callback for non-script packets
    SocketManager::RegisterProcessRecvDataCallback(DebuggerRecvDataCallback);

    // -- create the console, and start the execution
    CConsoleWindow* debugger = new CConsoleWindow();;
    int result = CConsoleWindow::GetInstance()->Exec();
    debugger->GetMainWindow()->autoSaveLayout();

    // -- shutdown
    SocketManager::Terminate();
    TinScript::DestroyContext();

    return result;
}

// --------------------------------------------------------------------------------------------------------------------
float32 GetSimTime() {
    int32 cur_time = CConsoleWindow::GetInstance()->GetOutput()->GetSimTime();
    float32 seconds = (float32)cur_time / 1000.0f;
    return (seconds);
}

REGISTER_FUNCTION_P0(GetSimTime, GetSimTime, float32);

// --------------------------------------------------------------------------------------------------------------------
#include "TinQTConsoleMoc.cpp"

// --------------------------------------------------------------------------------------------------------------------
// eof
// --------------------------------------------------------------------------------------------------------------------
