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
CBreakpointEntry::CBreakpointEntry(uint32 codeblock_hash, int line_number, QListWidget* owner)
    : QListWidgetItem("", owner)
{
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

void CBreakpointEntry::UpdateLabel(uint32 codeblock_hash, int32 line_number)
{
    char linebuf[256];
    char spaces[6] = "     ";
    int space_count = line_number >= 1e5 ? 0 :
                        line_number >= 1e4 ? 1 :
                        line_number >= 1e3 ? 2 :
                        line_number >= 1e2 ? 3 :
                        line_number >= 1e1 ? 4 : 5;
    spaces[space_count] = '\0';
                                               
    sprintf_s(linebuf, 256, "%s : %s%d", TinScript::UnHash(codeblock_hash), spaces, line_number);

    // -- set the text in the QWidget
    setText(linebuf);
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
        breakpoint = new CBreakpointEntry(codeblock_hash, line_number, this);
        breakpoint->UpdateLabel(codeblock_hash, line_number);
        mBreakpoints.append(breakpoint);
        sortItems();
    }

    // -- else delete it
    else if(!add && breakpoint) {
        mBreakpoints.removeAt(found_index);
        delete breakpoint;
    }
}

void CDebugBreakpointsWin::NotifyCodeblockLoaded(uint32 codeblock_hash)
{
    // -- get the file name
    const char* filename = TinScript::UnHash(codeblock_hash);

    // -- loop through all the existing breakpoints, and set the breakpoints
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if (breakpoint->mCodeblockHash == codeblock_hash)
        {
            // -- see if the breakpoint is enabled
            bool breakpoint_enabled = breakpoint->checkState() == Qt::Checked;

            // -- notify the source window
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash,
                                                                                 breakpoint->mLineNumber, true,
                                                                                 breakpoint_enabled);

            // -- also notify the target, if the breakpoint is enabled
            if (breakpoint_enabled)
            {
                SocketManager::SendCommandf("DebuggerAddBreakpoint('%s', %d);", filename, breakpoint->mLineNumber);
            }
        }
    }
}

// ====================================================================================================================
// NotifyConfirmBreakpoint():  Notification of the actual breakable line for a requested breakpoint
// ====================================================================================================================
void CDebugBreakpointsWin::NotifyConfirmBreakpoint(uint32 codeblock_hash, int32 line_number, int32 actual_line)
{
    // -- ensure the numbers are different
    if (line_number == actual_line)
        return;

    int foundIndex = -1;
    CBreakpointEntry* found = NULL;
    CBreakpointEntry* alreadyExists = NULL;
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if (breakpoint->mCodeblockHash == codeblock_hash)
        {
            if (breakpoint->mLineNumber == line_number)
            {
                found = breakpoint;
                foundIndex = i;
            }

            if (breakpoint->mLineNumber == actual_line)
                alreadyExists = breakpoint;
        }
    }

    // -- if we found our breakpoint, and one doesn't already exist, simply update the line
    if (found)
    {
        // -- clear the breakpoint from the old line
        CConsoleWindow::GetInstance()->GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, line_number,
                                                                             false, false);

        bool old_enabled = found->checkState() == Qt::Checked;
        if (!alreadyExists)
        {
            found->mLineNumber = actual_line;
            found->UpdateLabel(codeblock_hash, actual_line);

            // -- update the source window with the new breakpoint location
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, actual_line,
                                                                                 true, old_enabled);
        }

        // -- otherwise they both exist - simply delete the invalid breakpoint
        else
        {
            bool new_enabled = alreadyExists->checkState() == Qt::Checked;

            CBreakpointEntry* temp = mBreakpoints.at(foundIndex);
            mBreakpoints.removeAt(foundIndex);
            delete temp;

            // -- update the source window with the new breakpoint location - if either was enabled, choose enabled
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->ToggleBreakpoint(codeblock_hash, actual_line,
                                                                                 true, old_enabled || new_enabled);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CDebugBreakpointsWin::NotifySourceFile(uint32 filehash)
{
    // -- loop through all the existing breakpoints, and set the breakpoints
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if (breakpoint->mCodeblockHash == filehash)
        {
            // -- notify the source window
            bool breakpoint_enabled = breakpoint->checkState() == Qt::Checked;
            CConsoleWindow::GetInstance()->GetDebugSourceWin()->ToggleBreakpoint(filehash, breakpoint->mLineNumber,
                                                                                 true, breakpoint_enabled);
        }
    }
}

// ====================================================================================================================
// NotifyOnConnect():  Called when the debugger is re-attached, to resend all active breakpoints
// ====================================================================================================================
void CDebugBreakpointsWin::NotifyOnConnect()
{
    // -- we want to re-broadcast all active breakpoints, upon reconnection
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        bool breakpoint_enabled = breakpoint->checkState() == Qt::Checked;

        // -- resend the breakpoint, if it's enabled
        if (breakpoint_enabled)
        {
            const char* filename = TinScript::UnHash(breakpoint->mCodeblockHash);
            SocketManager::SendCommandf("DebuggerAddBreakpoint('%s', %d);", filename, breakpoint->mLineNumber);
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
