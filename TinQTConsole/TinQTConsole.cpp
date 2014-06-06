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
#include "socket.h"

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
    mMainWindow = new CMainWindow();
    mMainWindow->resize(QSize(1200, 800));

    // -- create the output widget
    mConsoleOutput = new CConsoleOutput(this);
    mConsoleOutput->addItem("Welcome to the TinConsole!");

    // -- create the consoleinput
    mInputLayout = new QHBoxLayout();
    mInputLabel = new QLabel("==>");
    mConsoleInput = new CConsoleInput(this);
    mConsoleInput->setFixedHeight(24);
    mInputLayout->addWidget(mInputLabel);
    mInputLayout->addWidget(mConsoleInput);

    // -- create the IPConnect
    mConnectLayout = new QHBoxLayout();
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

    mConnectLayout->addWidget(mIPLabel);
    mConnectLayout->addWidget(mConnectIP);
    mConnectLayout->addWidget(mButtonConnect);

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
    mGridLayout->addLayout(mInputLayout,    4, 0, 1, Qt::AlignLeft);

    // -- column 1
    mGridLayout->addWidget(mCallstackWin,   1, 1, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mBreakpointsWin, 2, 1, 1, Qt::AlignLeft);
    mGridLayout->addWidget(mWatchWin,       3, 1, 1, Qt::AlignLeft);
    mGridLayout->addLayout(mConnectLayout,  4, 1, 1, Qt::AlignLeft);

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
    QObject::connect(mButtonConnect, SIGNAL(clicked()), mConsoleInput, SLOT(OnButtonConnectPressed()));
    QObject::connect(mConnectIP, SIGNAL(returnPressed()), mConsoleInput, SLOT(OnConnectIPReturnPressed()));

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

    // -- initialize the breakpoint members
    mBreakpointHit = false;
    mBreakpointCodeblockHash = 0;
    mBreakpointLinenumber = -1;

    mBreakpointRun = false;
    mBreakpointStep = false;

    // -- initialize the running members
    mQuit = false;
    mPaused = false;
    mIsConnected = false;
}

CConsoleWindow::~CConsoleWindow()
{
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
    delete mInputLabel;
    delete mInputLayout;
    delete mIPLabel;
    delete mConnectIP;
    delete mButtonConnect;
    delete mConnectLayout;
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

    // -- send the text command to identify this connection as for a debugger
    SocketManager::SendCommand("DebuggerSetConnected(true);");

    // -- resend our list of breakpoints
    GetDebugBreakpointsWin()->NotifyOnConnect();
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
}

void CConsoleWindow::NotifyOnClose()
{
    // -- disconnect
    SocketManager::Disconnect();

    // -- first, clear the member
    mMainWindow = NULL;
}

// ------------------------------------------------------------------------------------------------
void CConsoleWindow::ToggleBreakpoint(uint32 codeblock_hash, int32 line_number,
                                      bool add, bool enable) {
    const char* filename = TinScript::UnHash(codeblock_hash);
    if (!filename)
        return;

    // -- send the message
    if (add && enable)
        SocketManager::SendCommandf("DebuggerAddBreakpoint('%s', %d);", filename, line_number);
    else
        SocketManager::SendCommandf("DebuggerRemoveBreakpoint('%s', %d);", filename, line_number);

    // -- notify the Source Window
    GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, line_number, add, enable);

    // -- notify the Breakpoints Window
    GetDebugBreakpointsWin()->ToggleBreakpoint(codeblock_hash, line_number, add, enable);
}

// ====================================================================================================================
// NotifyBreakpointHit():  A breakpoint hit was found during processing of the data packets
// ====================================================================================================================
void CConsoleWindow::NotifyBreakpointHit(uint32 codeblock_hash, int32 line_number)
{
    // -- set the bool
    mBreakpointHit = true;

    // -- cache the breakpoint details
    mBreakpointCodeblockHash = codeblock_hash;
    mBreakpointLinenumber = line_number;
}

// ====================================================================================================================
// HasBreakpoint():  Returns true, if a breakpoint hit is pending, along with the specific codeblock / line number
// ====================================================================================================================
bool CConsoleWindow::HasBreakpoint(uint32& codeblock_hash, int32& line_number)
{
    // -- no breakpoint - return false
    if (!mBreakpointHit)
        return (false);

    // -- fill in the details
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
    mBreakpointHit = false;
    mBreakpointRun = false;
    mBreakpointStep = false;

    // -- highlight the button in red
    mButtonRun->setAutoFillBackground(true);
	QPalette myPalette = mButtonRun->palette();
	myPalette.setColor(QPalette::Button, Qt::red);	
	mButtonRun->setPalette(myPalette);

    while (SocketManager::IsConnected() && !mBreakpointRun && !mBreakpointStep)
    {
        QCoreApplication::processEvents();

        // -- update our own environment, especially receiving data packetes
        GetOutput()->DebuggerUpdate();
    }

    // -- back to normal color
	myPalette.setColor(QPalette::Button, Qt::transparent);	
	mButtonRun->setPalette(myPalette);

    // -- clear the callstack and the watch window
    GetDebugCallstackWin()->ClearCallstack();

    // -- we need to notify the watch win that we're not currently broken
    GetDebugWatchWin()->NotifyEndOfBreakpoint();
}

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
    else
    {
        // -- send the message to run
        SocketManager::SendCommand("DebuggerBreakStep();");
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
// DebuggerNotifyCallstack():  Called directly from the SocketManager registered RecvPacket function
// ====================================================================================================================
void DebuggerNotifyCallstack(uint32* codeblock_array, uint32* objid_array, uint32* namespace_array,
                             uint32* func_array, uint32* linenumber_array, int array_size)
{
    CConsoleWindow::GetInstance()->GetDebugCallstackWin()->NotifyCallstack(codeblock_array, objid_array,
                                                                           namespace_array, func_array,
                                                                           linenumber_array, array_size);

    // -- we need to notify the watch window, we have a new callstack
    CConsoleWindow::GetInstance()->GetDebugWatchWin()->NotifyUpdateCallstack(true);
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
}

// ====================================================================================================================
// NotifyWatchVarEntry():  Called when a variable is being watched (e.g. autos, during a breakpoint)
// ====================================================================================================================
void NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry)
{
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
CMainWindow::CMainWindow() : QWidget()
{
};

void CMainWindow::closeEvent(QCloseEvent *event)
{
    SocketManager::Disconnect();
    event->accept();
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

    else
    {
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

    // -- see if we have a breakpoint
    uint32 codeblock_hash = 0;
    int32 line_number = -1;
    bool hasBreakpoint = CConsoleWindow::GetInstance()->HasBreakpoint(codeblock_hash, line_number); 
    
    // -- this will notify all required windows, and loop until the the breakpoint has been handled
    if (hasBreakpoint)
    {
        DebuggerBreakpointHit(codeblock_hash, line_number);
    }

    // -- if we're not paused, update TinScript
    if (!mOwner->IsPaused())
    {
        mCurrentTime += kUpdateTime;
        TinScript::UpdateContext(mCurrentTime);
    }

    // -- see if we're supposed to quit
    if(mOwner->IsQuit())
    {
        QApplication::closeAllWindows();
    }
}

// ====================================================================================================================
// DebuggerUpdate():  An abbreviated update to keep us alive while we're handing a breakpoint
// ====================================================================================================================
void CConsoleOutput::DebuggerUpdate()
{
    // -- process the received packets
    ProcessDataPackets();
    //TinScript::UpdateContext(mCurrentTime);
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

            case k_DebuggerCallstackPacketID:
                HandlePacketCallstack(dataPtr);
                break;

            case k_DebuggerWatchVarEntryPacketID:
                HandlePacketWatchVarEntry(dataPtr);
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

    // -- get the codeblock has
    uint32 codeblock_hash = *dataPtr++;

    // -- get the line number
    int32 line_number = *dataPtr++;

    // -- notify the debugger
    CConsoleWindow::GetInstance()->NotifyBreakpointHit(codeblock_hash, line_number);
    CConsoleWindow::GetInstance()->GetDebugWatchWin()->NotifyBreakpointHit();
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

    // -- notify the debugger
    CConsoleWindow::GetInstance()->GetDebugWatchWin()->NotifyWatchVarEntry(&watch_var_entry);
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
    new CConsoleWindow();
    int result = CConsoleWindow::GetInstance()->Exec();

    // -- shutdown
    SocketManager::Terminate();
    TinScript::DestroyContext();

    return result;
}

// --------------------------------------------------------------------------------------------------------------------
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

// --------------------------------------------------------------------------------------------------------------------
#include "TinQTConsoleMoc.cpp"

// --------------------------------------------------------------------------------------------------------------------
// eof
// --------------------------------------------------------------------------------------------------------------------
