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

// -- includes

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include "qlistwidget.h"
#include <QDockWidget>

#include "socket.h"

// --------------------------------------------------------------------------------------------------------------------
// -- Forward declarations

class CConsoleInput;
class CConsoleOutput;
class CDebugSourceWin;
class CDebugToolBar;
class QGridLayout;
class CDebugBreakpointsWin;
class CDebugCallstackWin;
class CDebugWatchWin;

// -- new "dock widget" framework
class MainWindow;

// ====================================================================================================================
// class CConsoleWindow:  The main application class, owning instance of all other components
// ====================================================================================================================
class CConsoleWindow
{
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

        void SetStatusMessage(const char* message);

        // -- Qt components
        QApplication* mApp;
        MainWindow* mMainWindow;
        QGridLayout* mGridLayout;
        MainWindow* GetMainWindow() { return (mMainWindow); }

        CConsoleOutput* mConsoleOutput;

        QHBoxLayout* mInputLayout;
        QLabel* mInputLabel;
        CConsoleInput* mConsoleInput;

        QLabel* mStatusLabel;
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

// ====================================================================================================================
// class CConsoleInput:  Provides text input, and history, to issue commands to the debug target.
// ====================================================================================================================
class CConsoleInput : public QLineEdit
{
    Q_OBJECT;

    public:
        explicit CConsoleInput(QWidget* parent);
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
            
        virtual void paintEvent(QPaintEvent* e)
        {
            ExpandToParentSize();
            QLineEdit::paintEvent(e);
        }

        void ExpandToParentSize()
        {
            // -- leave room at the start for the input label
            QSize parentSize = parentWidget()->size();
            int newWidth = parentSize.width() - 24;
            if (newWidth < 0)
                newWidth = 0;
            int newYOffset = parentSize.height() - 24;
            if (newYOffset < 0)
                newYOffset = 0;
            setGeometry(24, newYOffset, newWidth, 24);
            updateGeometry();

            // -- update the label as well
            mInputLabel->setGeometry(0, newYOffset, 24, 24);
        }

    private:
        QLabel* mInputLabel;
        static const int32 kMaxHistory = 64;
        bool8 mHistoryFull;
        int32 mHistoryIndex;
        int32 mHistoryLastIndex;
        char mHistory[TinScript::kMaxTokenLength][kMaxHistory];
};

// ====================================================================================================================
// class CConsoleOutput:  An output window, receiving any form of output message from the debug target.
// ====================================================================================================================
class CConsoleOutput : public QListWidget {
    Q_OBJECT;

    public:
        explicit CConsoleOutput(QWidget* parent);
        virtual ~CConsoleOutput();

        virtual void paintEvent(QPaintEvent* e)
        {
            ExpandToParentSize();
            QListWidget::paintEvent(e);
        }

        void ExpandToParentSize()
        {
            // -- resize to be the parent widget's size, with room for the title,
            // -- leave room at the bottom for the console input
            QSize parentSize = parentWidget()->size();
            int newWidth = parentSize.width();
            int newHeight = parentSize.height() - 46;
            if (newHeight < 20)
                newHeight = 20;
            setGeometry(0, 20, newWidth, newHeight);
            updateGeometry();
            //update();
        }

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
        QTimer* mTimer;

        unsigned int mCurrentTime;

        // -- the console output also needs to receive and process data packets
        TinScript::CThreadMutex mThreadLock;
        std::vector<SocketManager::tDataPacket*> mReceivedPackets;
};

// ====================================================================================================================
// -- global interface
TinScript::CScriptContext* GetScriptContext();
int ConsolePrint(const char* fmt, ...);

#endif // __TINQTCONSOLE_H

// ====================================================================================================================
// eof
// ====================================================================================================================
