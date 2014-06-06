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

#ifndef __TINQTWATCHWIN_H
#define __TINQTWATCHWIN_H

#include <qgridlayout.h>
#include <qtreewidget.h>
#include <qlabel.h>

// ------------------------------------------------------------------------------------------------
// forward declares
namespace TinScript {
    class CDebuggerWatchVarEntry;
}

class CConsoleWindow;

// ------------------------------------------------------------------------------------------------
class CWatchEntry : public QTreeWidgetItem {
    public:
        CWatchEntry(const TinScript::CDebuggerWatchVarEntry& debugger_entry);

        virtual ~CWatchEntry();

        void UpdateValue(const char* newValue);

        TinScript::CDebuggerWatchVarEntry mDebuggerEntry;

        // -- need a "confirmed" flag, to ensure each time the stack is updated that the
        // -- entry still exists
        bool mUnconfirmed;
};

// ------------------------------------------------------------------------------------------------
class CDebugWatchWin : public QTreeWidget {
    Q_OBJECT

    public:
        CDebugWatchWin(CConsoleWindow* owner);
        virtual ~CDebugWatchWin();

        CWatchEntry* FindWatchEntry(uint32 funcHash, uint32 objectID, uint32 nsHash, bool& foundNamespaceLabel);
        CWatchEntry* FindWatchEntry(uint32 funcHash, uint32 objectID, uint32 nsHash, uint32 memberHash, bool isMember);

        void AddTopLevelEntry(const TinScript::CDebuggerWatchVarEntry& watch_var_entry);
        void AddObjectMemberEntry(const TinScript::CDebuggerWatchVarEntry& watch_var_entry);

        void ClearWatchWin();
        void NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry);

        void NotifyBreakpointHit();
        void NotifyEndOfBreakpoint() { }
        void NotifyUpdateCallstack(bool breakpointHit);

    public slots:

    private:
        CConsoleWindow* mOwner;
        QTreeWidgetItem* mHeaderItem;
        QList<CWatchEntry*> mWatchList;
};

#endif //__TINQTWATCHWIN_H

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
