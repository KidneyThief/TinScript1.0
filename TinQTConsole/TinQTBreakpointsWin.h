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

#ifndef __TINQTBREAKPOINTSWIN_H
#define __TINQTBREAKPOINTSWIN_H

#include <qgridlayout.h>
#include <qlist.h>
#include <qcheckbox.h>
#include <qlabel.h>

class CBreakpointEntry;

class CBreakpointCheck : public QCheckBox {
    Q_OBJECT

    public:

        CBreakpointCheck(CBreakpointEntry* breakpoint);
        virtual ~CBreakpointCheck() { }

    public slots:
        void OnCheckBoxClicked();

    private:
        CBreakpointEntry* mBreakpoint;
};

class CBreakpointEntry : public QListWidgetItem {
    public:
        CBreakpointEntry(uint32 codeblock_hash, int line_number, const char* label,
                         QListWidget* owner);
        virtual ~CBreakpointEntry();

        uint32 mCodeblockHash;
        int32 mLineNumber;

        // $$$TZA FIXME
        //QListWidgetItem* item = new QListWidgetItem("item", listWidget);
        //item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        //item->setCheckState(Qt::Unchecked); // AND initialize check state
};

class CDebugBreakpointsWin : public QListWidget {
    Q_OBJECT

    public:
        CDebugBreakpointsWin(CConsoleWindow* owner);
        virtual ~CDebugBreakpointsWin();

        void ToggleBreakpoint(uint32 codeblock_hash, int32 line_number, bool add, bool enable);
        void NotifyCodeblockLoaded(uint32 codeblock_hash);
        void NotifySourceFile(uint32 filehash);

    public slots:
        void OnClicked(QListWidgetItem*);
        void OnDoubleClicked(QListWidgetItem*);

    private:
        CConsoleWindow* mOwner;
        QList<CBreakpointEntry*> mBreakpoints;
        //QVBoxLayout* mVBoxLayout;
        //QWidget* mSpacer;
};

class CCallstackEntry : public QListWidgetItem {
    public:
        CCallstackEntry(uint32 codeblock_hash, int line_number, uint32 object_id,
                        uint32 namespace_hash, uint32 function_hash);
        uint32 mCodeblockHash;
        int mLineNumber;
        uint32 mObjectID;
        uint32 mNamespaceHash;
        uint32 mFunctionHash;
};

class CDebugCallstackWin : public QListWidget {
    Q_OBJECT

    public:
        CDebugCallstackWin(CConsoleWindow* owner);
        virtual ~CDebugCallstackWin();

        void NotifyCallstack(uint32* codeblock_array, uint32* objid_array, uint32* namespace_array,
                             uint32* func_array, uint32* linenumber_array, int array_size);
        void ClearCallstack();

    public slots:
        void OnDoubleClicked(QListWidgetItem*);

    private:
        CConsoleWindow* mOwner;
        QList<CCallstackEntry*> mCallstack;
};

#endif

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
