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

#ifndef __TINQTSOURCEWIN_H
#define __TINQTSOURCEWIN_H

#include <qpushbutton.h>
#include <qgridlayout.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qbytearray.h>
#include <qtablewidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

class CSourceLine : public QListWidgetItem {
    public:
        CSourceLine(QByteArray& text, int line_number);
        int mLineNumber;
        bool mBreakpointSet;
};

class CDebugSourceWin : public QListWidget {
    Q_OBJECT

    public:
        CDebugSourceWin(CConsoleWindow* owner);
        virtual ~CDebugSourceWin();

        bool OpenSourceFile(const char* filename);
        bool SetSourceView(uint32 codeblock_hash, int32 line_number);
        void SetCurrentPC(uint32 codeblock_hash, int32 line_number);
        void ToggleBreakpoint(uint32 codeblock_hash, int32 line_number, bool add, bool enable);

    public slots:
        void OnDoubleClicked(QListWidgetItem*);

    private:
        CConsoleWindow* mOwner;
        QList<CSourceLine*> mSourceText;
        uint32 mCurrentCodeblockHash;
        int32 mCurrentLineNumber;
};

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
