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

class CConsoleInput;
class CConsoleOutput;

class CConsoleWindow {
    public:
        CConsoleWindow();
        virtual ~CConsoleWindow();

        int Exec() {
            return mApp->exec();
        }

        // -- scripted methods
        void Quit() { mQuit = true; }
        void Pause() { mPaused = true; }
        void Unpause() { mPaused = false; }

        bool IsQuit() { return mQuit; }
        bool IsPaused() { return mPaused; }

        void AddText(char* msg);

        // -- Qt component accessors
        CConsoleOutput* GetOutput() { return mConsoleOutput; }
        CConsoleInput* GetInput() { return mConsoleInput; }

        // -- Qt components
        QApplication* mApp;
        QWidget* mMainWindow;
        QGridLayout* mGridLayout;
        CConsoleOutput* mConsoleOutput;
        CConsoleInput* mConsoleInput;

    private:
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

    public slots:
        void Update();

    private:
        // -- the console output handles the current time, and timer events to call Update()
        CConsoleWindow* mOwner;
        QTimer* mTimer;

        unsigned int mCurrentTime;
};

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
