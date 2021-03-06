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
#include "TinQTToolsWin.h"
#include "TinQTObjectBrowserWin.h"
#include "TinQTObjectInspectWin.h"
#include "TinQTSchedulesWin.h"
#include "TinQTFunctionAssistWin.h"

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
    outputDockWidget->setMinimumHeight(200);

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
    mButtonExec = new QPushButton();
    mButtonExec->setText("Exec");
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
    toolbar->addWidget(mButtonExec);
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
    mSourceWinDockWidget = new QDockWidget();
    mSourceWinDockWidget->setObjectName("Source View");
    mSourceWinDockWidget->setWindowTitle("Source View");
    mDebugSourceWin = new CDebugSourceWin(mSourceWinDockWidget);

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
    mAutosWinDockWidget = new QDockWidget();
    mAutosWinDockWidget->setObjectName("Autos");
    mAutosWinDockWidget->setWindowTitle("Autos");
    mAutosWin = new CDebugWatchWin(mAutosWinDockWidget);

    // -- create the watches window
    QDockWidget* watchesDockWidget = new QDockWidget();
    watchesDockWidget->setObjectName("Watches");
    watchesDockWidget->setWindowTitle("Watches");
    mWatchesWin = new CDebugWatchWin(watchesDockWidget);

    // -- create the object browser window
    QDockWidget* browserDockWidget = new QDockWidget();
    browserDockWidget->setObjectName("Object Browser");
    browserDockWidget->setWindowTitle("Object Browser");
    mObjectBrowserWin = new CDebugObjectBrowserWin(browserDockWidget);

    // -- create the schedules window
    QDockWidget* schedulesDockWidget = new QDockWidget();
    schedulesDockWidget->setObjectName("Scheduler");
    schedulesDockWidget->setWindowTitle("Scheduler");
    mSchedulesWin = new CDebugSchedulesWin(schedulesDockWidget);

    // -- create the schedules window
    QDockWidget* functionAssistDockWidget = new QDockWidget();
    functionAssistDockWidget->setObjectName("Function Assist");
    functionAssistDockWidget->setWindowTitle("Function Assist");
    mFunctionAssistWin = new CDebugFunctionAssistWin(functionAssistDockWidget);

    // -- connect the widgets
    QObject::connect(mButtonConnect, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonConnectPressed()));
    QObject::connect(mConnectIP, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnConnectIPReturnPressed()));

    QObject::connect(mConsoleInput, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnReturnPressed()));

    QObject::connect(mDebugSourceWin, SIGNAL(itemDoubleClicked(QListWidgetItem*)), mDebugSourceWin,
                                      SLOT(OnDoubleClicked(QListWidgetItem*)));

    QObject::connect(mFileLineEdit, SIGNAL(returnPressed()), mConsoleInput,
                                    SLOT(OnFileEditReturnPressed()));

    QObject::connect(mButtonExec, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonExecPressed()));
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

    // Ctrl + h - Command input history
    QShortcut* shortcut_cmdHistory = new QShortcut(QKeySequence("Ctrl+H"), mMainWindow);
    QObject::connect(shortcut_cmdHistory, SIGNAL(activated()), mMainWindow, SLOT(menuCommandHistory()));

    // Ctrl + w - Add variable watch
    QShortcut* shortcut_AddVar = new QShortcut(QKeySequence("Ctrl+W"), mMainWindow);
    QObject::connect(shortcut_AddVar, SIGNAL(activated()), mMainWindow, SLOT(menuAddVariableWatch()));

    // Ctrl + Shift + w - Watch Variable
    QShortcut* shortcut_WatchVar = new QShortcut(QKeySequence("Ctrl+Shift+W"), mMainWindow);
    QObject::connect(shortcut_WatchVar, SIGNAL(activated()), mMainWindow, SLOT(menuCreateVariableWatch()));

    // Ctrl + i - Object Inspect
    QShortcut* shortcut_inspectObj = new QShortcut(QKeySequence("Ctrl+I"), mMainWindow);
    QObject::connect(shortcut_inspectObj, SIGNAL(activated()), mMainWindow, SLOT(menuCreateObjectInspector()));

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

    // F1 - Function Assist Window
    QShortcut* shortcut_FunctionAssist = new QShortcut(QKeySequence("F1"), mMainWindow);
    QObject::connect(shortcut_FunctionAssist, SIGNAL(activated()), mConsoleInput, SLOT(OnFunctionAssistPressed()));

    mMainWindow->addDockWidget(Qt::TopDockWidgetArea, mSourceWinDockWidget);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, outputDockWidget);
    mMainWindow->addDockWidget(Qt::TopDockWidgetArea, callstackDockWidget);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, breakpointsDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, mAutosWinDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, watchesDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, browserDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, schedulesDockWidget);
    mMainWindow->addDockWidget(Qt::BottomDockWidgetArea, functionAssistDockWidget);

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
    static bool initialized = false;
    static char last_msg[4096] = { '\0' };

    // -- write the string into the buffer
    va_list args;
    va_start(args, fmt);
    char buffer[2048];
    vsprintf_s(buffer, 2048, fmt, args);
    va_end(args);

    // -- if the last label exists, it means we want to append because it didn't end in a '\n'
    if (last_msg[0] != '\0')
    {
        int last_msg_length = strlen(last_msg);
        TinScript::SafeStrcpy(&last_msg[last_msg_length], buffer, 4096 - last_msg_length);
        int msg_count = CConsoleWindow::GetInstance()->GetOutput()->count();
        CConsoleWindow::GetInstance()->GetOutput()->takeItem(msg_count - 1);
    }
    else
    {
        TinScript::SafeStrcpy(last_msg, buffer, 4096);
    }

    // -- print the buffer
    char* buf_ptr = last_msg;
    while (buf_ptr && *buf_ptr != '\0')
    {
        // -- add a separate line at every newline character
        char* eol_ptr = strchr(buf_ptr, '\n');
        if (eol_ptr)
        {
            *eol_ptr = '\0';
            CConsoleWindow::GetInstance()->AddText(const_cast<char*>(buf_ptr));
            buf_ptr = eol_ptr + 1;
        }
        else
        {
            break;
        }
    }

    // -- if the buffer didn't end on a newline
    if (*buf_ptr != '\0')
    {
        // -- add the last message
        CConsoleWindow::GetInstance()->AddText(const_cast<char*>(buf_ptr));

        // -- copy the message to the beginning of the buffer
        if (buf_ptr > last_msg)
            TinScript::SafeStrcpy(last_msg, buf_ptr, 4096);
    }

    // -- otherwise, zero out the last msg so we don't append
    else
    {
        last_msg[0] = '\0';
    }

    return(0);
}

// -- returns false if we should break
bool8 AssertHandler(TinScript::CScriptContext* script_context, const char* condition,
                    const char* file, int32 linenumber, const char* fmt, ...)
{
    if (!script_context->IsAssertStackSkipped() || script_context->IsAssertEnableTrace())
    {
        if(!script_context->IsAssertStackSkipped())
            ConsolePrint("\n*************************************************************\n");
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

    // -- request the ObjectBrowser be repopulated
    GetDebugObjectBrowserWin()->NotifyOnConnect();

    // -- request the SchedulesWin be repopulated
    GetDebugSchedulesWin()->NotifyOnConnect();

    // -- Console Input label is colored to reflect connection status
    mConsoleOutput->NotifyConnectionStatus(true);
    mConsoleInput->NotifyConnectionStatus(true);

    // -- add some details
    ConsolePrint("*******************************************************************\n");
    ConsolePrint("*                                 Debugger Connected                                     *\n");
    ConsolePrint("*  All console input will be redirected to the connected target.  *\n");
    ConsolePrint("*******************************************************************\n");
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

    // -- update the input label to reflrect our connection status
    mConsoleOutput->NotifyConnectionStatus(false);
    mConsoleInput->NotifyConnectionStatus(false);
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
           !mBreakpointStep && !mBreakpointStepIn && !mBreakpointStepOut && !mAssertTriggered)
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

// ====================================================================================================================
// FindOrCreateToolsWindow():  Finds a tools window by name, creates if one is not found
// ====================================================================================================================
CDebugToolsWin* CConsoleWindow::FindOrCreateToolsWindow(const char* window_name)
{
    // - sanity check
    if (!window_name || !window_name[0])
        return (NULL);

    // -- see if the window already exists
    CDebugToolsWin* found = NULL;
    uint32 name_hash = TinScript::Hash(window_name);
    if (mToolsWindowMap.contains(name_hash))
    {
        found = mToolsWindowMap[name_hash];
    }

    // -- not found - create the tools window
    else
    {
        QDockWidget* new_tools_window_dock = new QDockWidget();
        new_tools_window_dock->setObjectName(window_name);
        new_tools_window_dock->setWindowTitle(window_name);
        found = new CDebugToolsWin(window_name, new_tools_window_dock);
        found->setGeometry(0, 0, 320, 240); 
        mToolsWindowMap[name_hash] = found;

        // -- see if we can find an existing tools window to join
        const QList<CDebugToolsWin*> tool_windows = mToolsWindowMap.values();

        QDockWidget* dock_parent = NULL;
        int count = tool_windows.size();
        for (int index = count - 1; index >= 0; --index)
        {
            CDebugToolsWin* last_dock = tool_windows.at(index);
            if (last_dock && last_dock->isVisible())
            {
                dock_parent = static_cast<QDockWidget*>(last_dock->parent());
                break;
            }
        }

        // -- if we still haven't found a dock parent, try the source window
        if (!dock_parent && mSourceWinDockWidget && mSourceWinDockWidget->isVisible())
            dock_parent = mSourceWinDockWidget;

        // -- dock the window initially, so it shows up
        if (dock_parent)
        {
            GetMainWindow()->tabifyDockWidget(dock_parent, new_tools_window_dock);
            new_tools_window_dock->show();
            new_tools_window_dock->raise();
        }

        // -- no parent to dock to - simply dock it to the top of the application
        else
        {
            GetMainWindow()->addDockWidget(Qt::TopDockWidgetArea, new_tools_window_dock, Qt::Horizontal);
        }
    }

    // -- return the window
    return (found);
}

// ====================================================================================================================
// ToolsWindowClear():  Find the tools window, and clears all its elements.
// ====================================================================================================================
void CConsoleWindow::ToolsWindowClear(const char* window_name)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (tools_win)
        tools_win->ClearAll();
}

// ====================================================================================================================
// ToolsWindowAddMessage():  Find the tools window, and adds a text message (separtor) to it.
// ====================================================================================================================
int32 CConsoleWindow::ToolsWindowAddMessage(const char* window_name, const char* message)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (!tools_win)
        return (0);

    // -- add the entry
    int32 entry_id = tools_win->AddMessage(message);

    return (entry_id);
}

// ====================================================================================================================
// ToolsWindowAddButton():  Find the tools window, and adds a button to it.
// ====================================================================================================================
int32 CConsoleWindow::ToolsWindowAddButton(const char* window_name, const char* name, const char* description,
                                           const char* value, const char* command)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (!tools_win)
        return (0);

    // -- add the entry
    int32 entry_id = tools_win->AddButton(name, description, value, command);

    return (entry_id);
}

// ====================================================================================================================
// ToolsWindowAddSlider():  Find the tools window, and adds a slider to it.
// ====================================================================================================================
int32 CConsoleWindow::ToolsWindowAddSlider(const char* window_name, const char* name, const char* description,
                                           int32 min_value, int32 max_value, int32 cur_value, const char* command)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (!tools_win)
        return (0);

    // -- add the entry
    int32 entry_id = tools_win->AddSlider(name, description, min_value, max_value, cur_value, command);

    return (entry_id);
}

// ====================================================================================================================
// ToolsWindowAddTextEdit():  Find the tools window, and adds a text edit to it.
// ====================================================================================================================
int32 CConsoleWindow::ToolsWindowAddTextEdit(const char* window_name, const char* name, const char* description,
                                             const char* cur_value, const char* command)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (!tools_win)
        return (0);

    // -- add the entry
    int32 entry_id = tools_win->AddTextEdit(name, description, cur_value, command);

    return (entry_id);
}

// ====================================================================================================================
// ToolsWindowAddCheckBox():  Find the tools window, and adds a check box to it.
// ====================================================================================================================
int32 CConsoleWindow::ToolsWindowAddCheckBox(const char* window_name, const char* name, const char* description,
                                             bool cur_value, const char* command)
{
    // -- find the window - return 0 (invalid index) if we are unable to find or create
    CDebugToolsWin* tools_win = FindOrCreateToolsWindow(window_name);
    if (!tools_win)
        return (0);

    // -- add the entry
    int32 entry_id = tools_win->AddCheckBox(name, description, cur_value, command);

    return (entry_id);
}

// ====================================================================================================================
// FindOrCreateObjectInspectWindow():  Finds a tools window by name, creates if one is not found
// ====================================================================================================================
CDebugObjectInspectWin* CConsoleWindow::FindOrCreateObjectInspectWin(uint32 object_id, const char* object_identifier)
{
    // - sanity check
    if (!object_identifier || !object_identifier[0])
        return (NULL);

    // -- see if the window already exists
    CDebugObjectInspectWin* found = NULL;
    if (mObjectInspectWindowMap.contains(object_id))
    {
        found = mObjectInspectWindowMap[object_id];
    }

    // -- not found - create the tools window
    else
    {
        QDockWidget* new_inspect_window_dock = new QDockWidget();
        new_inspect_window_dock->setObjectName(object_identifier);
        new_inspect_window_dock->setWindowTitle(object_identifier);
        found = new CDebugObjectInspectWin(object_id, object_identifier, new_inspect_window_dock);
        found->setGeometry(0, 0, 320, 240); 
        mObjectInspectWindowMap[object_id] = found;

        // -- see if we can find an existing inspect window to join
        const QList<CDebugObjectInspectWin*> inspect_windows = mObjectInspectWindowMap.values();

        QDockWidget* dock_parent = NULL;
        int count = inspect_windows.size();
        for (int index = count - 1; index >= 0; --index)
        {
            CDebugObjectInspectWin* last_dock = inspect_windows.at(index);
            if (last_dock && last_dock->isVisible())
            {
                dock_parent = static_cast<QDockWidget*>(last_dock->parent());
                break;
            }
        }

        // -- if we still haven't found a dock parent, try the source window
        if (!dock_parent && mAutosWinDockWidget && mAutosWinDockWidget->isVisible())
            dock_parent = mAutosWinDockWidget;

        // -- dock the window initially, so it shows up
        if (dock_parent)
        {
            GetMainWindow()->tabifyDockWidget(dock_parent, new_inspect_window_dock);
            new_inspect_window_dock->show();
            new_inspect_window_dock->raise();
        }

        // -- no parent to dock to - simply dock it to the top of the application
        else
        {
            GetMainWindow()->addDockWidget(Qt::TopDockWidgetArea, new_inspect_window_dock, Qt::Horizontal);
        }

        // -- send the request to populate the inspector
        if (SocketManager::IsConnected())
        {
            SocketManager::SendCommandf("DebuggerInspectObject(%d);", object_id);
        }
    }

    // -- return the window
    return (found);
}

// ====================================================================================================================
// NotifyDestroyObject():  Notification that an object has been destroyed.
// ====================================================================================================================
void CConsoleWindow::NotifyDestroyObject(uint32 object_id)
{
    // -- delete the associated object inspector, if it exists
    if (mObjectInspectWindowMap.contains(object_id))
    {
        CDebugObjectInspectWin* found = mObjectInspectWindowMap[object_id];
        mObjectInspectWindowMap.remove(object_id);
        QDockWidget* dock_window = static_cast<QDockWidget*>(found->parentWidget());
        delete found;
        delete dock_window;
    }
}

// ====================================================================================================================
// NotifyWatchVarEntry():  Notification that an object value has been updated.
// ====================================================================================================================
void CConsoleWindow::NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry)
{
    // -- see if this entry is for an object that we're inspecting
    if (watch_var_entry->mObjectID > 0 && mObjectInspectWindowMap.contains(watch_var_entry->mObjectID))
    {
        CDebugObjectInspectWin* found = mObjectInspectWindowMap[watch_var_entry->mObjectID];
        found->SetEntryValue(*watch_var_entry);
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
    CConsoleWindow::GetInstance()->GetDebugFunctionAssistWin()->NotifyCodeblockLoaded(codeblock_hash);
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
    CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyWatchVarEntry(watch_var_entry, false);
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

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CConsoleInput::CConsoleInput(QWidget* parent) : QLineEdit(parent)
{
    // -- q&d history implementation
    mHistoryFull = false;
    mHistoryIndex = -1;
    mHistoryLastIndex = -1;
    for(int32 i = 0; i < kMaxHistory; ++i)
        mHistory[i].text[0] = '\0';

    // -- create the label as well
    mInputLabel = new QLabel("==>", parent);

    // -- initialize the connection status
    NotifyConnectionStatus(false);
}

// ====================================================================================================================
// SetText():  From an external source, set the input string.
// ====================================================================================================================
void CConsoleInput::SetText(const char* text, int cursor_pos)
{
    if (!text)
        text = "";
    setText(text);
    int length = strlen(text);
    if (cursor_pos > length)
        cursor_pos = length;

    if (cursor_pos >= 0)
    {
        setCursorPosition(cursor_pos);
    }
}

// ====================================================================================================================
// GetHistory():  Returns the array of command input strings stored in the ConsoleInput history.
// ====================================================================================================================
void CConsoleInput::GetHistory(QStringList& history) const
{
    // -- ensure we actually have some history
    if (mHistoryLastIndex < 0)
        return;

    // -- fill in the history string list
    for (int i = mHistoryLastIndex; i >= 0; --i)
    {
        QString* new_history = new QString(mHistory[i].text);
        history.push_back(*new_history);
    }

    // -- if the history buffer was full, we wrapped
    if (mHistoryFull)
    {
        for (int i = kMaxHistory - 1; i > mHistoryLastIndex; --i)
        {
            //QString* new_history = new QString(mHistory[i].text);
            history.push_back(QString(mHistory[i].text));
        }
    }
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
        ConsolePrint("%s%s\n", kConsoleSendPrefix, input_text);
    else
        ConsolePrint("%s%s\n", kLocalSendPrefix, input_text);

    // -- add this to the history buf
    const char* historyptr = (mHistoryLastIndex < 0) ? NULL : mHistory[mHistoryLastIndex].text;
    if(input_text[0] != '\0' && (!historyptr || strcmp(historyptr, input_text) != 0)) {
        mHistoryFull = mHistoryFull || mHistoryLastIndex == kMaxHistory - 1;
        mHistoryLastIndex = (mHistoryLastIndex + 1) % kMaxHistory;
        strcpy_s(mHistory[mHistoryLastIndex].text, TinScript::kMaxTokenLength, input_text);
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

void CConsoleInput::OnButtonExecPressed()
{
    QLineEdit* fileedit = CConsoleWindow::GetInstance()->GetFileLineEdit();
    QString filename_string = fileedit->text();

    QByteArray filename_bytearray = filename_string.toUtf8();
    const char* filename = filename_bytearray.data();

    if (filename && filename[0])
    {
        char cmd_buf[TinScript::kMaxNameLength];
        sprintf_s(cmd_buf, "Exec('%s');", filename);
        ConsolePrint("%s%s\n", kConsoleSendPrefix, cmd_buf);
        SocketManager::SendCommand(cmd_buf);
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

void CConsoleInput::OnFunctionAssistPressed()
{
    // -- ensure the function assist window is focused, and populate it with the currently selected object
    uint32 object_id = 0;
    if (CConsoleWindow::GetInstance()->GetDebugWatchesWin()->hasFocus())
	    object_id = CConsoleWindow::GetInstance()->GetDebugWatchesWin()->GetSelectedObjectID();
    else if (CConsoleWindow::GetInstance()->GetDebugAutosWin()->hasFocus())
	    object_id = CConsoleWindow::GetInstance()->GetDebugAutosWin()->GetSelectedObjectID();
    else if (CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->hasFocus())
        object_id = CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->GetSelectedObjectID();

    CConsoleWindow::GetInstance()->GetDebugFunctionAssistWin()->SetAssistObjectID(object_id);
}

// ------------------------------------------------------------------------------------------------
void CConsoleInput::keyPressEvent(QKeyEvent * event)
{
    if (!event)
        return;

    // -- up arrow
    if (event->key() == Qt::Key_Up)
    {
        int32 oldhistory = mHistoryIndex;
        if (mHistoryIndex < 0)
            mHistoryIndex = mHistoryLastIndex;
        else if (mHistoryLastIndex > 0)
        {
            if (mHistoryFull)
                mHistoryIndex = (mHistoryIndex + kMaxHistory - 1) % kMaxHistory;
            else
                mHistoryIndex = (mHistoryIndex + mHistoryLastIndex) % (mHistoryLastIndex + 1);
        }

        // -- see if we actually changed
        if (mHistoryIndex != oldhistory && mHistoryIndex >= 0)
        {
            setText(mHistory[mHistoryIndex].text);
        }
    }

    // -- down arrow
    else if (event->key() == Qt::Key_Down)
    {
        int32 oldhistory = mHistoryIndex;
        if (mHistoryIndex < 0)
            mHistoryIndex = mHistoryLastIndex;
        else if (mHistoryLastIndex > 0)
        {
            if (mHistoryFull)
                mHistoryIndex = (mHistoryIndex + 1) % kMaxHistory;
            else
                mHistoryIndex = (mHistoryIndex + 1) % (mHistoryLastIndex + 1);
        }

        // -- see if we actually changed
        if (mHistoryIndex != oldhistory && mHistoryIndex >= 0)
        {
            setText(mHistory[mHistoryIndex].text);
        }
    }

    // -- esc
    else if (event->key() == Qt::Key_Escape)
    {
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

    // -- initialize the connection status (sets the color)
    NotifyConnectionStatus(false);
}

CConsoleOutput::~CConsoleOutput()
{
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

    // -- we'll use GetTickCount(), to try to use a more accurate representation of time
    static DWORD gSystemTickCount = 0;
    DWORD current_tick = GetTickCount();
    if (gSystemTickCount == 0)
    {
        gSystemTickCount = current_tick;
    }
    int delta_ms = current_tick - gSystemTickCount;

    mCurrentTime += delta_ms;
    gSystemTickCount = current_tick;
    TinScript::UpdateContext(mCurrentTime);

    // -- this is a bit unusual, but we're going to update the schedules window using the same update time
    CConsoleWindow::GetInstance()->GetDebugSchedulesWin()->Update(delta_ms);
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

    // -- copy the received packets, so we can unlock the thread
    // -- which allows responses to send to the socket without it already being locked
    std::vector<SocketManager::tDataPacket*> process_packets;
    while (mReceivedPackets.size() > 0)
    {
        process_packets.push_back(mReceivedPackets[0]);
        mReceivedPackets.erase(mReceivedPackets.begin());
    }

    // -- unlock the thread
    mThreadLock.Unlock();

    while (process_packets.size() > 0)
    {
        // -- dequeue the packet
        SocketManager::tDataPacket* packet = process_packets[0];
        process_packets.erase(process_packets.begin());

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

            case k_DebuggerFunctionAssistPacketID:
                HandlePacketFunctionAssist(dataPtr);
                break;

            default:
                break;
        }

        // -- the callback is required to manage the packet memory itself
        delete packet;
    }
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

	// -- variable watch request ID (unused for stack dumps)
	watch_var_entry.mStackLevel = *dataPtr++;

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

    // -- notify the debugger - see if we can find matching entries, and update them all, both autos and watches
	CConsoleWindow::GetInstance()->GetDebugWatchesWin()->NotifyVarWatchResponse(&watch_var_entry);
	CConsoleWindow::GetInstance()->GetDebugWatchesWin()->NotifyWatchVarEntry(&watch_var_entry, true);
	CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyWatchVarEntry(&watch_var_entry, false);
    CConsoleWindow::GetInstance()->NotifyWatchVarEntry(&watch_var_entry);
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
    ConsolePrint("%s%s", kConsoleRecvPrefix, msg);
}

// ====================================================================================================================
// HandlePacketFunctionAssist():  A handler for packet type "function assist entry"
// ====================================================================================================================
void CConsoleOutput::HandlePacketFunctionAssist(int32* dataPtr)
{
    // -- skip past the packet ID
    ++dataPtr;

    // -- reconstitute the stuct
    TinScript::CDebuggerFunctionAssistEntry function_assist_entry;

    // -- the only packets we receive from the target are actual functions, not object entries
    function_assist_entry.mIsObjectEntry = false;

	// -- object ID
	function_assist_entry.mObjectID = *dataPtr++;;

	// -- namespace hash
	function_assist_entry.mNamespaceHash = *dataPtr++;

	// -- function hash
	function_assist_entry.mFunctionHash = *dataPtr++;

    // -- value string length
    int32 name_length = *dataPtr++;

    // -- copy the function name string
    strcpy_s(function_assist_entry.mFunctionName, (const char*)dataPtr);
    dataPtr += (name_length / 4);

    // -- parameter count
    function_assist_entry.mParameterCount = *dataPtr++;

    // -- loop through, and send each parameter
    for (int i = 0; i < function_assist_entry.mParameterCount; ++i)
    {
        // -- type
        function_assist_entry.mType[i] = (TinScript::eVarType)(*dataPtr++);

        // -- is array
        function_assist_entry.mIsArray[i] = (*dataPtr++ != 0);

        // -- name hash
        function_assist_entry.mNameHash[i] = *dataPtr++;
    }

    // -- notify the function assist window
    CConsoleWindow::GetInstance()->GetDebugFunctionAssistWin()->NotifyFunctionAssistEntry(function_assist_entry);
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

    ConsolePrint("*** ASSERT ***\n");
    ConsolePrint("%s\n", assertmsg);

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

// ====================================================================================================================
// ToolPaletteClear():  Finds or creates a tools window, and clears all current entries.
// ====================================================================================================================
static void ToolPaletteClear(const char* win_name)
{
    CConsoleWindow::GetInstance()->ToolsWindowClear(win_name);
}

// ====================================================================================================================
// ToolPaletteAddMessage():  Finds or creates a tools window, and adds a message to it.
// ====================================================================================================================
static int32 ToolPaletteAddMessage(const char* win_name, const char* message)
{
    int32 index = CConsoleWindow::GetInstance()->ToolsWindowAddMessage(win_name, message);
    return (index);
}

// ====================================================================================================================
// ToolPaletteAddButton():  Finds or creates a tools window, and adds a button to it.
// ====================================================================================================================
static int32 ToolPaletteAddButton(const char* win_name, const char* name, const char* description, const char* value,
                                  const char* command)
{
    int32 index = CConsoleWindow::GetInstance()->ToolsWindowAddButton(win_name, name, description, value, command);
    return (index);
}

// ====================================================================================================================
// ToolPaletteAddSlider():  Finds or creates a tools window, and adds a slider to it.
// ====================================================================================================================
static int32 ToolPaletteAddSlider(const char* win_name, const char* name, const char* description, int32 min_value,
                                  int32 max_value, int32 cur_value, const char* command)
{
    int32 index = CConsoleWindow::GetInstance()->ToolsWindowAddSlider(win_name, name, description, min_value,
                                                                      max_value, cur_value, command);
    return (index);
}

// ====================================================================================================================
// ToolPaletteAddTextEdit():  Finds or creates a tools window, and adds a message to it.
// ====================================================================================================================
static int32 ToolPaletteAddTextEdit(const char* win_name, const char* name, const char* description,
                                    const char* cur_value, const char* command)
{
    int32 index = CConsoleWindow::GetInstance()->ToolsWindowAddTextEdit(win_name, name, description, cur_value,
                                                                        command);
    return (index);
}

// ====================================================================================================================
// ToolPaletteAddTextEdit():  Finds or creates a tools window, and adds a message to it.
// ====================================================================================================================
static int32 ToolPaletteAddCheckBox(const char* win_name, const char* name, const char* description, bool8 cur_value,
                                    const char* command)
{
    int32 index = CConsoleWindow::GetInstance()->ToolsWindowAddCheckBox(win_name, name, description, cur_value,
                                                                        command);
    return (index);
}

// ====================================================================================================================
// ToolPaletteSetName():  Given a DebugEntry ID, update the name.
// ====================================================================================================================
static void ToolPaletteSetName(int32 entry_id, const char* new_name)
{
    CDebugToolsWin::SetEntryName(entry_id, new_name);
}

// ====================================================================================================================
// ToolPaletteSetDescription():  Given a DebugEntry ID, update the name.
// ====================================================================================================================
static void ToolPaletteSetDescription(int32 entry_id, const char* new_description)
{
    CDebugToolsWin::SetEntryDescription(entry_id, new_description);
}

// ====================================================================================================================
// ToolPaletteSetValue():  Given a DebugEntry ID, update the value.
// ====================================================================================================================
static void ToolPaletteSetValue(int32 entry_id, const char* new_value)
{
    CDebugToolsWin::SetEntryValue(entry_id, new_value);
}

// == ToolPalette Registration ========================================================================================

REGISTER_FUNCTION_P1(ToolPaletteClear, ToolPaletteClear, void, const char*);
REGISTER_FUNCTION_P2(ToolPaletteAddMessage, ToolPaletteAddMessage, int32, const char*, const char*);
REGISTER_FUNCTION_P5(ToolPaletteAddButton, ToolPaletteAddButton, int32, const char*, const char*, const char*, const char*, const char*);
REGISTER_FUNCTION_P7(ToolPaletteAddSlider, ToolPaletteAddSlider, int32, const char*, const char*, const char*, int32, int32, int32, const char*);
REGISTER_FUNCTION_P5(ToolPaletteAddTextEdit, ToolPaletteAddTextEdit, int32, const char*, const char*, const char*, const char*, const char*);
REGISTER_FUNCTION_P5(ToolPaletteAddCheckBox, ToolPaletteAddCheckBox, int32, const char*, const char*, const char*, bool8, const char*);

REGISTER_FUNCTION_P2(ToolPaletteSetName, ToolPaletteSetName, void, int32, const char*);
REGISTER_FUNCTION_P2(ToolPaletteSetDescription, ToolPaletteSetDescription, void, int32, const char*);
REGISTER_FUNCTION_P2(ToolPaletteSetValue, ToolPaletteSetValue, void, int32, const char*);

// ====================================================================================================================
// DebuggerClearObjectBrowser():  Method to clear the ObjectBrowser
// ====================================================================================================================
void DebuggerClearObjectBrowser()
{
    CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->RemoveAll();
}

// ====================================================================================================================
// DebuggerNotifyAddObject():  Add an entry for an object to the ObjectBrowser.
// ====================================================================================================================
void DebuggerNotifyCreateObject(int32 object_id, const char* object_name, const char* derivation)
{
    CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->NotifyCreateObject(object_id, object_name, derivation);
}

// ====================================================================================================================
// DebuggerNotifyDestroyObject():  Remove the entry for an object from the ObjectBrowser.
// ====================================================================================================================
void DebuggerNotifyDestroyObject(int32 object_id)
{
    CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->NotifyDestroyObject(object_id);
    CConsoleWindow::GetInstance()->NotifyDestroyObject(object_id);
}

// ====================================================================================================================
// DebuggerSetAddObject():  Add the object to a set, mirrored in the ObjectBrowser's tree view.
// ====================================================================================================================
void DebuggerNotifySetAddObject(int32 set_id, int32 object_id, bool8 owned)
{
    CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->NotifySetAddObject(set_id, object_id, owned);
}

// ====================================================================================================================
// DebuggerSetRemoveObject():  Add the object to a set, mirrored in the ObjectBrowser's tree view.
// ====================================================================================================================
void DebuggerNotifySetRemoveObject(int32 set_id, int32 object_id)
{
    CConsoleWindow::GetInstance()->GetDebugObjectBrowserWin()->NotifySetRemoveObject(set_id, object_id);
}

// ====================================================================================================================
// DebuggerNotifyTimeScale():  Receive notification of a canceled or completed schedule.
// ====================================================================================================================
void DebuggerNotifyTimeScale(float time_scale)
{
    CConsoleWindow::GetInstance()->GetDebugSchedulesWin()->NotifyTargetTimeScale(time_scale);
}

// ====================================================================================================================
// DebuggerAddSchedule():  Receive notification of a target's pending schedule.
// ====================================================================================================================
void DebuggerAddSchedule(int32 schedule_id, bool8 repeat, int32 time_remaining_ms, int32 object_id,
                         const char* command)
{
    CConsoleWindow::GetInstance()->GetDebugSchedulesWin()->AddSchedule(schedule_id, repeat, time_remaining_ms,
                                                                       object_id, command);
}

// ====================================================================================================================
// DebuggerRemoveSchedule():  Receive notification of a canceled or completed schedule.
// ====================================================================================================================
void DebuggerRemoveSchedule(int32 schedule_id)
{
    CConsoleWindow::GetInstance()->GetDebugSchedulesWin()->RemoveSchedule(schedule_id);
}

// == ObjectBrowser Registration ======================================================================================

REGISTER_FUNCTION_P0(DebuggerClearObjectBrowser, DebuggerClearObjectBrowser, void);
REGISTER_FUNCTION_P3(DebuggerNotifyCreateObject, DebuggerNotifyCreateObject, void, int32, const char*, const char*);
REGISTER_FUNCTION_P1(DebuggerNotifyDestroyObject, DebuggerNotifyDestroyObject, void, int32);
REGISTER_FUNCTION_P3(DebuggerNotifySetAddObject, DebuggerNotifySetAddObject, void, int32, int32, bool8);
REGISTER_FUNCTION_P2(DebuggerNotifySetRemoveObject, DebuggerNotifySetRemoveObject, void, int32, int32);

REGISTER_FUNCTION_P1(DebuggerNotifyTimeScale, DebuggerNotifyTimeScale, void, float);
REGISTER_FUNCTION_P5(DebuggerAddSchedule, DebuggerAddSchedule, void, int32, bool8, int32, int32, const char*);
REGISTER_FUNCTION_P1(DebuggerRemoveSchedule, DebuggerRemoveSchedule, void, int32);

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
