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
#include "qlistwidget.h"

#include "socket.h"

class CConsoleInput;
class CConsoleOutput;
class CDebugSourceWin;
class CDebugToolBar;
class QGridLayout;
class CDebugBreakpointsWin;
class CDebugCallstackWin;
class CDebugWatchWin;
class CMainWindow;

class CConsoleWindow {
    public:
        CConsoleWindow();
        virtual ~CConsoleWindow();
        static CConsoleWindow* GetInstance() { return (gConsoleWindow); }

        int Exec();

        // -- scripted methods
        void AddText(char* msg);

        // -- Qt component accessors
        CConsoleOutput* GetOutput() { return (mConsoleOutput); }
        CConsoleInput* GetInput() { return (mConsoleInput); }
        QLineEdit* GetConnectIP() { return (mConnectIP); }
        QPushButton* GetConnectButton() { return (mButtonConnect); }
        QLineEdit* GetFileLineEdit() { return (mFileLineEdit); }
        CDebugSourceWin* GetDebugSourceWin() { return (mDebugSourceWin); }
        CDebugBreakpointsWin* GetDebugBreakpointsWin() { return (mBreakpointsWin); }
        CDebugCallstackWin* GetDebugCallstackWin() { return (mCallstackWin); }
        CDebugWatchWin* GetDebugWatchWin() { return (mWatchWin); }

        // -- Qt components
        QApplication* mApp;
        CMainWindow* mMainWindow;
        QGridLayout* mGridLayout;

        CConsoleOutput* mConsoleOutput;

        QHBoxLayout* mInputLayout;
        QLabel* mInputLabel;
        CConsoleInput* mConsoleInput;

        QHBoxLayout* mConnectLayout;
        QLabel* mIPLabel;
        QLineEdit* mConnectIP;
        QPushButton* mButtonConnect;

        CDebugSourceWin* mDebugSourceWin;
        CDebugBreakpointsWin* mBreakpointsWin;
        CDebugCallstackWin* mCallstackWin;
        CDebugWatchWin* mWatchWin;

        QHBoxLayout* mToolbarLayout;
        QLabel* mFileLabel;
        QLineEdit* mFileLineEdit;
        QPushButton* mButtonRun;
        QPushButton* mButtonStep;
        QWidget* mSpacer;

        // -- notifications
        bool8 IsConnected() const
        {
            return (mIsConnected);
        }
        void NotifyOnConnect();
        void NotifyOnDisconnect();

        // -- if we close the window, we destroy the console input (our main signal/slot hub)
        void NotifyOnClose();

        // -- debugger methods
        void ToggleBreakpoint(uint32 codeblock_hash, int32 line_number, bool add, bool enable);

        // -- notify breakpoint hit - allows the next update to execute the HandleBreakpointHit()
        // -- keeps the threads separate
        void NotifyBreakpointHit(uint32 codeblock_hash, int32 line_number);
        bool HasBreakpoint(uint32& codeblock_hash, int32& line_number);
        void HandleBreakpointHit(const char* breakpoint_msg);

        // -- not dissimilar to a breakpoint
        void NotifyAssertTriggered(const char* assert_msg, uint32 codeblock_hash, int32 line_number);
        bool HasAssert(const char*& assert_msg, uint32& codeblock_hash, int32& line_number);
        void ClearAssert(bool set_break);

        // -- breakpoint members
        bool mBreakpointHit;
        uint32 mBreakpointCodeblockHash;
        int32 mBreakpointLinenumber;
        bool mBreakpointRun;
        bool mBreakpointStep;

        // -- assert members
        bool mAssertTriggered;
        char mAssertMessage[kMaxArgLength];

    private:
        static CConsoleWindow* gConsoleWindow;

        // -- store whether we're connected
        bool8 mIsConnected;
};

class CMainWindow : public QWidget
{
    Q_OBJECT;

    public:
        CMainWindow();

    protected:
        virtual void closeEvent(QCloseEvent *);
};

class CConsoleInput : public QLineEdit {
    Q_OBJECT;

    public:
        explicit CConsoleInput(CConsoleWindow* owner);
        virtual ~CConsoleInput() { }

    public slots:
        void OnButtonConnectPressed();
        void OnConnectIPReturnPressed();
        void OnReturnPressed();
        void OnFileEditReturnPressed();
        void OnButtonRunPressed();
        void OnButtonStepPressed();

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

        // -- these methods queue and process data packets, received from the socket
        // -- both must be thread safe
        void ReceiveDataPacket(SocketManager::tDataPacket* packet);
        void ProcessDataPackets();

        // -- handlers for the data packet
        void HandlePacketCurrentWorkingDir(int32* dataPtr);
        void HandlePacketCodeblockLoaded(int32* dataPtr);
        void HandlePacketBreakpointConfirm(int32* dataPtr);
        void HandlePacketBreakpointHit(int32* dataPtr);
        void HandlePacketCallstack(int32* dataPtr);
        void HandlePacketWatchVarEntry(int32* dataPtr);
        void HandlePacketAssertMsg(int32* dataPtr);
        void HandlePacketPrintMsg(int32* dataPtr);

        // -- called while handling a breakpoint, to ensure we still get to update our own context
        void DebuggerUpdate();

    public slots:
        void Update();

    private:
        // -- the console output handles the current time, and timer events to call Update()
        CConsoleWindow* mOwner;
        QTimer* mTimer;

        unsigned int mCurrentTime;

        // -- the console output also needs to receive and process data packets
        TinScript::CThreadMutex mThreadLock;
        std::vector<SocketManager::tDataPacket*> mReceivedPackets;
};

TinScript::CScriptContext* GetScriptContext();
int ConsolePrint(const char* fmt, ...);

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
