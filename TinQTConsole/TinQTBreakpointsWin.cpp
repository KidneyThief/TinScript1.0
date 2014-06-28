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

#include <QDockWidget>
#include <QKeyEvent>

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTWatchWin.h"

// ------------------------------------------------------------------------------------------------
CBreakpointCheck::CBreakpointCheck(CBreakpointEntry* breakpoint) : QCheckBox()
{
    mBreakpoint = breakpoint;
}

void CBreakpointCheck::OnCheckBoxClicked()
{
}

// ------------------------------------------------------------------------------------------------
CBreakpointEntry::CBreakpointEntry(uint32 codeblock_hash, int32 line_number, QListWidget* owner)
    : QListWidgetItem("", owner)
{
    // -- store the file/line members
    mCodeblockHash = codeblock_hash;
    mLineNumber = line_number;

    // -- clear the var watch members
    mWatchRequestID = 0;
    mWatchVarObjectID = 0;
    mWatchVarNameHash = 0;

    setFlags(flags() | Qt::ItemIsUserCheckable);
    setCheckState(Qt::Checked);
};

CBreakpointEntry::CBreakpointEntry(int32 watch_request_id, uint32 var_object_id, uint32 var_name_hash, QListWidget* owner)
    : QListWidgetItem("", owner)
{
    // -- clear the file/line members
    mCodeblockHash = 0;
    mLineNumber = 0;

    // -- set the var watch members
    mWatchRequestID = watch_request_id;
    mWatchVarObjectID = var_object_id;
    mWatchVarNameHash = var_name_hash;

    setFlags(flags() | Qt::ItemIsUserCheckable);
    setCheckState(Qt::Checked);

    // -- set the text in the QWidget
    char label_buf[TinScript::kMaxNameLength];
    if (mWatchVarObjectID > 0)
        sprintf_s(label_buf, 256, "_watch:  %d.%s", mWatchVarObjectID, TinScript::UnHash(mWatchVarNameHash));
    else
        sprintf_s(label_buf, 256, "_watch:  %s", TinScript::UnHash(mWatchVarNameHash));
    setText(label_buf);
};

CBreakpointEntry::~CBreakpointEntry()
{
}

void CBreakpointEntry::UpdateLabel(uint32 codeblock_hash, int32 line_number)
{
    // -- ensure we're not updating the label for a variable watch
    if (mWatchRequestID > 0)
        return;

    char linebuf[256];
    char spaces[6] = "     ";
    int space_count = line_number >= 1e5 ? 0 :
                        line_number >= 1e4 ? 1 :
                        line_number >= 1e3 ? 2 :
                        line_number >= 1e2 ? 3 :
                        line_number >= 1e1 ? 4 : 5;
    spaces[space_count] = '\0';

    // -- note:  all line numbers are stored accurately (0 based), but displayed +1, to match text editors
    sprintf_s(linebuf, 256, "%s : %s%d", TinScript::UnHash(codeblock_hash), spaces, line_number + 1);

    // -- set the text in the QWidget
    setText(linebuf);
}

void CDebugBreakpointsWin::OnClicked(QListWidgetItem* item)
{
    CBreakpointEntry* breakpoint = static_cast<CBreakpointEntry*>(item);

    // -- see if we're enabled
    bool enabled = breakpoint->checkState() == Qt::Checked;

    // -- if this is a file/line breakpoint, toggle it (affects the source view as well)
    if (breakpoint->mWatchRequestID == 0)
    {
        CConsoleWindow::GetInstance()->ToggleBreakpoint(breakpoint->mCodeblockHash,
                                                        breakpoint->mLineNumber, true, enabled);
    }

    // -- otherwise, send the toggle message directly
    else
    {
        SocketManager::SendCommandf("DebuggerToggleVarWatch(%d, %d, %d, %s);", breakpoint->mWatchRequestID,
                                                                               breakpoint->mWatchVarObjectID,
                                                                               breakpoint->mWatchVarNameHash,
                                                                               enabled ? "true" : "false");
    }
}

void CDebugBreakpointsWin::OnDoubleClicked(QListWidgetItem* item)
{
    CBreakpointEntry* breakpoint = static_cast<CBreakpointEntry*>(item);

    // -- open the source, to the filename
    if (breakpoint->mWatchRequestID == 0)
    {
        CConsoleWindow::GetInstance()->GetDebugSourceWin()->SetSourceView(breakpoint->mCodeblockHash,
                                                                          breakpoint->mLineNumber);
    }
}

void CDebugBreakpointsWin::keyPressEvent(QKeyEvent* event)
{
    if (!event)
        return;

    // -- delete the selected, if we have a selected
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        CBreakpointEntry* cur_item = static_cast<CBreakpointEntry*>(currentItem());
        if (cur_item)
        {
            if (cur_item->mWatchRequestID == 0)
            {
                CConsoleWindow::GetInstance()->ToggleBreakpoint(cur_item->mCodeblockHash, cur_item->mLineNumber,
                                                                false, false);
            }
            else
            {
                SocketManager::SendCommandf("DebuggerToggleVarWatch(%d, %d, %d, false);", cur_item->mWatchRequestID,
                                                                                          cur_item->mWatchVarObjectID,
                                                                                          cur_item->mWatchVarNameHash);
            }
        }
        return;
    }

    QListWidget::keyPressEvent(event);
}

CDebugBreakpointsWin::CDebugBreakpointsWin(QWidget* parent) : QListWidget(parent)
{
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

    int found_index = -1;
    CBreakpointEntry* breakpoint = NULL;
    for(int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* temp = mBreakpoints.at(i);
        if (temp->mWatchRequestID == 0 && temp->mCodeblockHash == codeblock_hash && temp->mLineNumber == line_number)
        {
            breakpoint = temp;
            found_index = i;
            break;
        }
    }

    // -- not found - create it
    if (add && !breakpoint)
    {
        breakpoint = new CBreakpointEntry(codeblock_hash, line_number, this);
        breakpoint->UpdateLabel(codeblock_hash, line_number);
        mBreakpoints.append(breakpoint);
        sortItems();
    }

    // -- else delete it
    else if (!add && breakpoint)
    {
        mBreakpoints.removeAt(found_index);
        delete breakpoint;
    }
}

// ====================================================================================================================
// SetCurrentBreakpoint():  Set the current breakpoint, when we hit a file/line break.
// ====================================================================================================================
void CDebugBreakpointsWin::SetCurrentBreakpoint(uint32 codeblock_hash, int32 line_number)
{
    int found_index = -1;
    CBreakpointEntry* breakpoint = NULL;
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* temp = mBreakpoints.at(i);
        if (temp->mWatchRequestID == 0 && temp->mCodeblockHash == codeblock_hash && temp->mLineNumber == line_number)
        {
            breakpoint = temp;
            found_index = i;
            break;
        }
    }

    // -- set the current entry (or clear it, if the entry wasn't found)
    setCurrentItem(breakpoint);
}

// ====================================================================================================================
// SetCurrentVarWatch():  Set the current breakpoint, when we hit a variable watch.
// ====================================================================================================================
void CDebugBreakpointsWin::SetCurrentVarWatch(int32 watch_request_id)
{
    int found_index = -1;
    CBreakpointEntry* breakpoint = NULL;
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* temp = mBreakpoints.at(i);
        if (temp->mWatchRequestID == watch_request_id)
        {
            breakpoint = temp;
            found_index = i;
            break;
        }
    }

    // -- set the current entry (or clear it, if the entry wasn't found)
    setCurrentItem(breakpoint);
}

void CDebugBreakpointsWin::NotifyCodeblockLoaded(uint32 codeblock_hash)
{
    // -- get the file name
    const char* filename = TinScript::UnHash(codeblock_hash);

    // -- loop through all the existing breakpoints, and set the breakpoints
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if (breakpoint->mWatchRequestID == 0 && breakpoint->mCodeblockHash == codeblock_hash)
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
            if (breakpoint->mWatchRequestID == 0 && breakpoint->mLineNumber == line_number)
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

// ====================================================================================================================
// NotifyConfirmVarWatch():  Received in response to a variable watch request to break on write.
// ====================================================================================================================
void CDebugBreakpointsWin::NotifyConfirmVarWatch(int32 watch_request_id, uint32 watch_object_id, uint32 var_name_hash)
{
    CBreakpointEntry* found = NULL;
    CBreakpointEntry* alreadyExists = NULL;
    for (int i = 0; i < mBreakpoints.size(); ++i)
    {
        CBreakpointEntry* breakpoint = mBreakpoints.at(i);
        if (breakpoint->mWatchRequestID == watch_request_id && breakpoint->mWatchVarObjectID == watch_object_id &&
            breakpoint->mWatchVarNameHash == var_name_hash)
        {
            found = breakpoint;
            break;
        }
    }

    // -- if we found our breakpoint, it means we've had a duplicate watch request, simply enable it
    if (found)
    {
        found->setCheckState(Qt::Checked);
    }
    else
    {
        CBreakpointEntry* breakpoint = new CBreakpointEntry(watch_request_id, watch_object_id, var_name_hash, this);
        mBreakpoints.append(breakpoint);
        sortItems();
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

        // -- resend the breakpoint, if it's enabled, and a file/line breakpoint
        if (breakpoint_enabled && breakpoint->mWatchRequestID == 0)
        {
            const char* filename = TinScript::UnHash(breakpoint->mCodeblockHash);
            SocketManager::SendCommandf("DebuggerAddBreakpoint('%s', %d);", filename, breakpoint->mLineNumber);
        }

        // -- otherwise, variable watches are disabled
        else if (breakpoint->mWatchRequestID > 0)
        {
            breakpoint->setCheckState(Qt::Unchecked);
        }
    }
}

// ------------------------------------------------------------------------------------------------
CCallstackEntry::CCallstackEntry(uint32 codeblock_hash, int32 line_number, uint32 object_id,
                                 uint32 namespace_hash, uint32 function_hash) : QListWidgetItem()
{
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

CDebugCallstackWin::CDebugCallstackWin(QWidget* parent) : QListWidget(parent)
{
    setWindowTitle(QString("Call Stack"));
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

void CDebugCallstackWin::OnDoubleClicked(QListWidgetItem* item)
{
    CCallstackEntry* stack_entry = static_cast<CCallstackEntry*>(item);
    CConsoleWindow::GetInstance()->GetDebugSourceWin()->SetSourceView(stack_entry->mCodeblockHash,
                                                                      stack_entry->mLineNumber);

    // -- find out which stack index this entry is, and notify the watchvar window
    for (int i = 0; i < mCallstack.size(); ++i)
    {
        if (mCallstack.at(i) == stack_entry)
        {
            CConsoleWindow::GetInstance()->GetDebugAutosWin()->NotifyUpdateCallstack(false);
            break;
        }
    }
}

int CDebugCallstackWin::GetSelectedStackEntry(uint32& func_ns_hash, uint32& func_hash, uint32& func_obj_id)
{
    if (mCallstack.size() <= 0)
        return (-1);

    QListWidgetItem* cur_item = currentItem();
    int stack_index = 0;
    while (stack_index < mCallstack.size() && mCallstack.at(stack_index) != cur_item)
        ++stack_index;

    // -- success
    if (stack_index < mCallstack.size())
    {
        CCallstackEntry* stack_entry = static_cast<CCallstackEntry*>(cur_item);
        func_ns_hash = stack_entry->mNamespaceHash;
        func_hash = stack_entry->mFunctionHash;
        func_obj_id = stack_entry->mObjectID;

        return (stack_index);
    }

    // -- fail
    return (-1);
}

// ====================================================================================================================
// ValidateStackEntry():  Returns true if there's a current stack entry matching the given function call attributes
// ====================================================================================================================
int CDebugCallstackWin::ValidateStackEntry(uint32 func_ns_hash, uint32 func_hash, uint32 func_obj_id)
{
    if (mCallstack.size() <= 0)
        return (-1);

    int stack_index = 0;
    while (stack_index < mCallstack.size())
    {
        const CCallstackEntry* callstack_entry = mCallstack.at(stack_index);
        if (callstack_entry && callstack_entry->mNamespaceHash == func_ns_hash &&
            callstack_entry->mFunctionHash == func_hash && callstack_entry->mObjectID == func_obj_id)
        {
            return (stack_index);
        }

        // -- not yet found
        ++stack_index;
    }

    // -- there are no callstack entries matching the watch entry
    return (-1);
}

// ------------------------------------------------------------------------------------------------
#include "TinQTBreakpointsWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
