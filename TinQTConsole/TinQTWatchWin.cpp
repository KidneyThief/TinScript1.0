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
// TinQTWatchWin.cpp
//

#include "stdafx.h"

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTWatchWin.h"

// ------------------------------------------------------------------------------------------------
// CWatchEntry

CWatchEntry::CWatchEntry(const char* varname, TinScript::eVarType type, const char* value,
                         int stack_index) : QTreeWidgetItem() {
    setText(0, varname);
    setText(1, TinScript::GetRegisteredTypeName(type));
    setText(2, value);

    strcpy_s(mName, varname ? varname : "");
    strcpy_s(mValue, value ? value : "");
    mType = type;
    mStackIndex = stack_index;
    mIsNamespace = false;
}

CWatchEntry::CWatchEntry(const char* nsname, int stack_index) : QTreeWidgetItem() {
    setText(0, nsname);
    setText(1, "Namespace");
    setText(2, "");

    strcpy_s(mName, nsname ? nsname : "");
    strcpy_s(mValue, "");
    mType = TinScript::TYPE_void;
    mStackIndex = stack_index;
    mIsNamespace = true;
}

CWatchEntry::~CWatchEntry() {

}

// ------------------------------------------------------------------------------------------------
// CDebugWatchWin
CDebugWatchWin::CDebugWatchWin(CConsoleWindow* owner) {
    mOwner = owner;

    setColumnCount(3);
    setItemsExpandable(true);
    setExpandsOnDoubleClick(true);

    // -- set the header
  	mHeaderItem = new QTreeWidgetItem();
	mHeaderItem->setText(0,QString("Variable"));
	mHeaderItem->setText(1,QString("Type"));
	mHeaderItem->setText(2,QString("Value"));
	setHeaderItem(mHeaderItem);

    mCurrentEntry = NULL;
}

CDebugWatchWin::~CDebugWatchWin() {
    delete mHeaderItem;
    mWatchList.clear();
    clear();
}

void CDebugWatchWin::BeginVariable(const char* varname, TinScript::eVarType type, const char* value, int stack_index) {
    if(!varname)
        return;

    mCurrentEntry = new CWatchEntry(varname, type, value ? value : "", stack_index);
    addTopLevelItem(mCurrentEntry);

    // -- if we're not on the right stack frame, set the item to hidden
    int cur_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->GetSelectedStackIndex();
    if(cur_stack_index != stack_index)
        mCurrentEntry->setHidden(true);

    // -- add to the global list
    mWatchList.append(mCurrentEntry);
}

void CDebugWatchWin::AddVariable(const char* varname, TinScript::eVarType type, const char* value, int stack_index) {
    if(!varname)
        return;

    CWatchEntry* new_var = new CWatchEntry(varname, type, value ? value : "", stack_index);
    mCurrentEntry->addChild(new_var);

    // -- if we're not on the right stack frame, set the item to hidden
    int cur_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->GetSelectedStackIndex();
    if(cur_stack_index != stack_index)
        new_var->setHidden(true);

    // -- add to the global list
    mWatchList.append(new_var);
}

void CDebugWatchWin::AddNamespace(const char* nsname, int stack_index) {
    if(!mCurrentEntry)
        return;
    CWatchEntry* ns = new CWatchEntry(nsname, stack_index);
    mCurrentEntry->addChild(ns);
    mCurrentEntry = ns;

    // -- if we're not on the right stack frame, set the item to hidden
    int cur_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->GetSelectedStackIndex();
    if(cur_stack_index != stack_index)
        ns->setHidden(true);

    // -- add to the global list
    mWatchList.append(ns);
}

// ------------------------------------------------------------------------------------------------
void CDebugWatchWin::ClearWatchWin() {
    mWatchList.clear();
    clear();
}

// ------------------------------------------------------------------------------------------------
void CDebugWatchWin::NotifyCallstackIndex(int callstack_index) {
    // -- remove without deleting, all items in the treeview
    while(true) {
        if(!takeTopLevelItem(0))
            break;
    }

    // -- loop through the entries, removing all with a different index from the window
    int entry_count = mWatchList.size();
    for(int i = 0; i < entry_count; ++i) {
        CWatchEntry* entry = mWatchList.at(i);
        if(entry->mStackIndex == callstack_index) {
            addTopLevelItem(entry);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CDebugWatchWin::NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry) {
    if(!watch_var_entry)
        return;
    // -- get the current callstack index, see if this variable belongs to it
    int stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->GetSelectedStackIndex();
    if(watch_var_entry->mIsNamespace) {
        AddNamespace(watch_var_entry->mName, watch_var_entry->mStackIndex);
    }
    else if(watch_var_entry->mIsMember) {
        AddVariable(watch_var_entry->mName, watch_var_entry->mType, watch_var_entry->mValue,
                    watch_var_entry->mStackIndex);
    }
    else {
        BeginVariable(watch_var_entry->mName, watch_var_entry->mType, watch_var_entry->mValue,
                      watch_var_entry->mStackIndex);
    }
}

// ------------------------------------------------------------------------------------------------
#include "TinQTWatchWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
