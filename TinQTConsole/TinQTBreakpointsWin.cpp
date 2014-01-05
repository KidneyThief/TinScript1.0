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

#include <QListWidget>

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTWatchWin.h"

// ------------------------------------------------------------------------------------------------
CBreakpointCheck::CBreakpointCheck(CBreakpointEntry* breakpoint) : QCheckBox() {
    mBreakpoint = breakpoint;
}

void CBreakpointCheck::OnCheckBoxClicked() {
}

// ------------------------------------------------------------------------------------------------
CBreakpointEntry::CBreakpointEntry(uint32 codeblock_hash, int line_number, const char* label,
                                   QListWidget* owner) : QListWidgetItem(label, owner) {
    mCodeblockHash = codeblock_hash;
    mLineNumber = line_number;

    //QListWidgetItem* item = new QListWidgetItem("item", listWidget);
    setFlags(flags() | Qt::ItemIsUserCheckable);
    setCheckState(Qt::Checked);

    // -- store the actual data needed
    mCodeblockHash = codeblock_hash;
    mLineNumber = line_number;

    // -- connect the signals
    //QObject::connect(mJumpButton, SIGNAL(clicked()), this, SLOT(OnJumpButtonClicked()));
};

CBreakpointEntry::~CBreakpointEntry() {
}

void CDebugBreakpointsWin::OnClicked(QListWidgetItem* item) {
    CBreakpointEntry* breakpoint = static_cast<CBreakpointEntry*>(item);
    // -- see if we're enabled
    bool enabled = breakpoint->checkState() == Qt::Checked;
    CConsoleWindow::GetInstance()->ToggleBreakpoint(breakpoint->mCodeblockHash,
                                                    breakpoint->mLineNumber, true, enabled);
}

void CDebugBreakpointsWin::OnDoubleClicked(QListWidgetItem* item) {
    CBreakpointEntry* breakpoint = static_cast<CBreakpointEntry*>(item);
    // -- open the source, to the filename
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->SetSourceView(breakpoint->mCodeblockHash,
                                                                      breakpoint->mLineNumber);
}

CDebugBreakpointsWin::CDebugBreakpointsWin(CConsoleWindow* owner) : QListWidget() {
    mOwner = owner;
}

CDebugBreakpointsWin::~CDebugBreakpointsWin() {
    while(mBreakpoints.size() > 0) {
        CBreakpointEntry* temp = mBreakpoints.at(0);
        mBreakpoints.removeAt(0);
        delete temp;
    }
}

void CDebugBreakpointsWin::ToggleBreakpoint(uint32 codeblock_hash, int32 line_number,
                                            bool add, bool enable) {

    // -- note:  +1 since all line numbers count from 1 to match editors
    int found_index = -1;
    CBreakpointEntry* breakpoint = NULL;
    for(int i = 0; i < mBreakpoints.size(); ++i) {
        CBreakpointEntry* temp = mBreakpoints.at(i);
        if(temp->mCodeblockHash == codeblock_hash && temp->mLineNumber == line_number) {
            breakpoint = temp;
            found_index = i;
            break;
        }
    }

    // -- not found - create it
    if(add && !breakpoint) {
        char linebuf[256];
        char spaces[6] = "     ";
        int space_count = line_number >= 1e5 ? 0 :
                          line_number >= 1e4 ? 1 :
                          line_number >= 1e3 ? 2 :
                          line_number >= 1e2 ? 3 :
                          line_number >= 1e1 ? 4 : 5;
        spaces[space_count] = '\0';
                                               
        sprintf_s(linebuf, 256, "%s : %s%d", TinScript::UnHash(codeblock_hash), spaces, line_number);
        breakpoint = new CBreakpointEntry(codeblock_hash, line_number, linebuf, this);
        mBreakpoints.append(breakpoint);
        sortItems();
    }

    // -- else delete it
    else if(!add && breakpoint) {
        mBreakpoints.removeAt(found_index);
        delete breakpoint;
    }
}

void CDebugBreakpointsWin::NotifyCodeblockLoaded(uint32 codeblock_hash) {
    // -- get the file name
    const char* filename = TinScript::UnHash(codeblock_hash);

    // -- loop through all the existing breakpoints, and set the breakpoints
    for(int i = 0; i < mBreakpoints.size(); ++i) {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if(breakpoint->mCodeblockHash == codeblock_hash) {
            bool breakpoint_enabled = breakpoint->checkState() == Qt::Checked;
            int actual_line = breakpoint->mLineNumber;

            // -- notify the script context to add a breakpoint (and adjust the line number if needed)
            if(breakpoint_enabled) {
                actual_line = GetScriptContext()->AddBreakpoint("", filename, breakpoint->mLineNumber);
                if(actual_line < 0)
                    actual_line = breakpoint->mLineNumber;
            }

            // -- if we did adjust the line number, update this breakpoint to reflect it
            if(actual_line != breakpoint->mLineNumber)
                breakpoint->mLineNumber = actual_line;

            // -- notify the source window
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->
                ToggleBreakpoint(codeblock_hash, actual_line, true, breakpoint_enabled);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CDebugBreakpointsWin::NotifySourceFile(uint32 filehash) {
    // -- loop through all the existing breakpoints, and set the breakpoints
    for(int i = 0; i < mBreakpoints.size(); ++i) {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if(breakpoint->mCodeblockHash == filehash) {
            bool breakpoint_enabled = breakpoint->checkState() == Qt::Checked;
            int actual_line = breakpoint->mLineNumber;

            // -- if we did adjust the line number, update this breakpoint to reflect it
            if(actual_line != breakpoint->mLineNumber)
                breakpoint->mLineNumber = actual_line;

            // -- notify the source window
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->
                ToggleBreakpoint(filehash, breakpoint->mLineNumber, true, breakpoint_enabled);
        }
    }
}

// ------------------------------------------------------------------------------------------------
CCallstackEntry::CCallstackEntry(uint32 codeblock_hash, int32 line_number, uint32 object_id,
                                 uint32 namespace_hash, uint32 function_hash) : QListWidgetItem() {
    mCodeblockHash = codeblock_hash;
    mLineNumber = line_number;

    mObjectID = object_id;

    mNamespaceHash = namespace_hash;
    mFunctionHash = function_hash;

    char buf[2048];
    sprintf_s(buf, 2048, "[ %d ] %s::%s   %s @ %d", object_id, TinScript::UnHash(namespace_hash),
              TinScript::UnHash(function_hash), TinScript::UnHash(codeblock_hash), line_number);
    setText(buf);
};

CDebugCallstackWin::CDebugCallstackWin(CConsoleWindow* owner) : QListWidget() {
}

CDebugCallstackWin::~CDebugCallstackWin() {
    ClearCallstack();
}

void CDebugCallstackWin::ClearCallstack() {
    while(mCallstack.size()) {
        CCallstackEntry* entry = mCallstack.at(0);
        mCallstack.removeAt(0);
        delete entry;
    }
}

void CDebugCallstackWin::NotifyCallstack(uint32* codeblock_array, uint32* objid_array,
                                         uint32* namespace_array, uint32* func_array,
                                         uint32* linenumber_array, int array_size) {
    // -- clear the callstack
    ClearCallstack();

    // -- add each entry in the callstack
    for(int i = 0; i < array_size; ++i) {
        CCallstackEntry* list_item =
            new CCallstackEntry(codeblock_array[i], linenumber_array[i], objid_array[i],
                                namespace_array[i], func_array[i]);
        addItem(list_item);
        mCallstack.append(list_item);
    }

    // -- if our array is non-empty set the selected to be the top of the stack
    if(mCallstack.size() > 0)
        setCurrentItem(mCallstack.at(0));
}

void CDebugCallstackWin::OnDoubleClicked(QListWidgetItem* item) {
    CCallstackEntry* stack_entry = static_cast<CCallstackEntry*>(item);
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->
        SetSourceView(stack_entry->mCodeblockHash, stack_entry->mLineNumber);

    // -- find out which stack index this entry is, and notify the watchvar window
    for(int i = 0; i < mCallstack.size(); ++i) {
        if(mCallstack.at(i) == stack_entry) {
            CConsoleWindow::GetInstance()->GetDebugWatchWin()->NotifyCallstackIndex(i);
            break;
        }
    }
}

int CDebugCallstackWin::GetSelectedStackIndex() {
    if(mCallstack.size() <= 0)
        return -1;

    QListWidgetItem* cur_item = currentItem();
    int stack_index = 0;
    while(stack_index < mCallstack.size() && mCallstack.at(stack_index) != cur_item)
        ++stack_index;

    // -- found
    if(stack_index < mCallstack.size())
        return stack_index;

    // -- failed
    else
        return -1;
}

// ------------------------------------------------------------------------------------------------
#include "TinQTBreakpointsWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
