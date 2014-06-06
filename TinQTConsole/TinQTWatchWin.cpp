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

CWatchEntry::CWatchEntry(const TinScript::CDebuggerWatchVarEntry& debugger_entry)
    : QTreeWidgetItem()
{
    // -- copy the debugger entry
    memcpy(&mDebuggerEntry, &debugger_entry, sizeof(TinScript::CDebuggerWatchVarEntry));

    // -- the text is displayed differently...
    // -- if this is a namespace label
    if (mDebuggerEntry.mObjectID > 0 && mDebuggerEntry.mNamespaceHash > 0 &&
        mDebuggerEntry.mType == TinScript::TYPE_void)
    {
        setText(0, mDebuggerEntry.mVarName);
        setText(1, "Namespace");
        setText(2, "");
    }

    // -- otherwise, it's a real entry
    else
    {
        // -- set the text
        setText(0, mDebuggerEntry.mVarName);
        setText(1, TinScript::GetRegisteredTypeName(mDebuggerEntry.mType));

        const char* value = mDebuggerEntry.mValue;
        if (mDebuggerEntry.mType == TinScript::TYPE_object && mDebuggerEntry.mVarObjectID == 0)
        {
            value = "<invalid>";
        }
        setText(2, value);
    }

    // -- it's being added, so it's valid
    mUnconfirmed = false;
}

CWatchEntry::~CWatchEntry()
{
}

void CWatchEntry::UpdateValue(const char* new_value)
{
    if (!new_value)
        new_value = "";

    // -- if it's of type object, it's could be a variable that was uninitialized - re-cache the object ID
    if (mDebuggerEntry.mType == TinScript::TYPE_object && mDebuggerEntry.mVarObjectID == 0)
    {
        new_value = "<invalid>";
    }

    strcpy_s(mDebuggerEntry.mValue, new_value);
    setText(2, mDebuggerEntry.mValue);
}

// ------------------------------------------------------------------------------------------------
// CDebugWatchWin
CDebugWatchWin::CDebugWatchWin(CConsoleWindow* owner)
{
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
}

CDebugWatchWin::~CDebugWatchWin()
{
    delete mHeaderItem;
    mWatchList.clear();
    clear();
}

void CDebugWatchWin::AddTopLevelEntry(const TinScript::CDebuggerWatchVarEntry& watch_var_entry)
{
    // -- find out what function call is currently selected on the stack
    uint32 cur_func_ns_hash = 0;
    uint32 cur_func_hash = 0;
    uint32 cur_func_object_id = 0;
    int32 current_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->
                                                               GetSelectedStackEntry(cur_func_ns_hash, cur_func_hash,
                                                                                     cur_func_object_id);
    if (current_stack_index < 0)
    {
        return;
    }

    // -- loop through the watch entries, and every instance of matching watch entry,
    // -- update the value (if it's not an object)
    bool found_callstack_entry = false;
    int entry_index = 0;
    while (entry_index < mWatchList.size())
    {
        CWatchEntry* entry = mWatchList.at(entry_index);
        if (entry->mDebuggerEntry.mObjectID == 0 &&
            entry->mDebuggerEntry.mType == watch_var_entry.mType &&
            entry->mDebuggerEntry.mVarHash == watch_var_entry.mVarHash)

        {
            // -- update the value (if it's not a label)
            if (entry->mDebuggerEntry.mType != TinScript::TYPE_void)
            {
                // -- if the entry is for an object, update the object ID as well
                if (entry->mDebuggerEntry.mType == TinScript::TYPE_object)
                    entry->mDebuggerEntry.mVarObjectID = watch_var_entry.mVarObjectID;

                // -- update the value (text label)
                entry->UpdateValue(watch_var_entry.mValue);
            }

            // -- if this entry is *not* the one for the call stack, make sure we still find one
            if (entry->mDebuggerEntry.mFuncNamespaceHash == cur_func_ns_hash &&
                entry->mDebuggerEntry.mFunctionHash == cur_func_hash &&
                entry->mDebuggerEntry.mFunctionObjectID == cur_func_object_id)
            {
                found_callstack_entry = true;
            }
        }

        // -- increment the index
        ++entry_index;
    }

    // -- if we did not find the entry for the current callstack, but this entry is from the top of the callstack...
    if (!found_callstack_entry && watch_var_entry.mFuncNamespaceHash == cur_func_ns_hash &&
        watch_var_entry.mFunctionHash == cur_func_hash && watch_var_entry.mFunctionObjectID == cur_func_object_id)
    {
        CWatchEntry* new_entry = new CWatchEntry(watch_var_entry);
        addTopLevelItem(new_entry);
        mWatchList.append(new_entry);
    }
}

void CDebugWatchWin::AddObjectMemberEntry(const TinScript::CDebuggerWatchVarEntry& watch_var_entry)
{
    // -- find out what function call is currently selected on the stack
    uint32 cur_func_ns_hash = 0;
    uint32 cur_func_hash = 0;
    uint32 cur_func_object_id = 0;
    int32 current_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->
                                                               GetSelectedStackEntry(cur_func_ns_hash, cur_func_hash,
                                                                                     cur_func_object_id);
    if (current_stack_index < 0)
    {
        return;
    }

    // -- loop through the watch entries, and every instance of a watch entry for the given object ID, ensure
    // -- it has a label
    int entry_index = 0;
    while (entry_index < mWatchList.size())
    {
        // -- find the object entry
        CWatchEntry* obj_entry = NULL;
        while (entry_index < mWatchList.size())
        {
            CWatchEntry* entry = mWatchList.at(entry_index);
            if (entry->mDebuggerEntry.mObjectID == 0 &&
                entry->mDebuggerEntry.mType == TinScript::TYPE_object &&
                entry->mDebuggerEntry.mVarObjectID == watch_var_entry.mObjectID)
            {
                // -- increment the index - we want to start looking for the member/label after the object entry
                ++entry_index;

                obj_entry = entry;
                break;
            }

            // -- not yet found - increment the index
            else
                ++entry_index;
        }

        // -- if we did not find the entry, we have no parent entry to add a namespace label
        if (!obj_entry)
            break;

        // -- otherwise, now see if we have a label 
        CWatchEntry* member_entry = NULL;
        while (entry_index < mWatchList.size())
        {
            CWatchEntry* entry = mWatchList.at(entry_index);
            if (entry->mDebuggerEntry.mObjectID == watch_var_entry.mObjectID &&
                entry->mDebuggerEntry.mNamespaceHash == watch_var_entry.mNamespaceHash &&
                entry->mDebuggerEntry.mType == watch_var_entry.mType &&
                entry->mDebuggerEntry.mVarHash == watch_var_entry.mVarHash)
            {
                member_entry = entry;
                break;
            }

            // -- else if we've moved on to a different object, we're done
            else if (entry->mDebuggerEntry.mObjectID != watch_var_entry.mObjectID)
            {
                break;
            }

            // -- not yet found - increment the index
            else
                ++entry_index;
        }

        // -- if we didn't find a label, add one
        if (member_entry == NULL)
        {
            CWatchEntry* ns = new CWatchEntry(watch_var_entry);
            obj_entry->addChild(ns);

            if (entry_index == mWatchList.size())
            {
                mWatchList.append(ns);
            }
            else
            {
                mWatchList.insert(mWatchList.begin() + entry_index, ns);
            }

            // -- now see if the label should be visible
            // -- either it's not from a function call, or it's from the current callstack function call
            if (!(watch_var_entry.mFuncNamespaceHash == 0 ||
                    (watch_var_entry.mFuncNamespaceHash == cur_func_ns_hash &&
                    watch_var_entry.mFunctionHash == cur_func_hash &&
                    watch_var_entry.mFunctionObjectID == cur_func_object_id)))
            {
                ns->setHidden(true);
            }

            // -- we want to increment the index, to account for the inserted entry
            ++entry_index;
        }

        // -- otherwise, simply update it's value
        else if (member_entry->mDebuggerEntry.mType != TinScript::TYPE_void)
        {
            member_entry->UpdateValue(watch_var_entry.mValue);
        }
    }
}

// ------------------------------------------------------------------------------------------------
void CDebugWatchWin::ClearWatchWin() {
    mWatchList.clear();
    clear();
}

// ------------------------------------------------------------------------------------------------
void CDebugWatchWin::NotifyWatchVarEntry(TinScript::CDebuggerWatchVarEntry* watch_var_entry)
{
    if (!watch_var_entry)
        return;

    // -- find out what function call is currently selected on the stack
    uint32 cur_func_ns_hash = 0;
    uint32 cur_func_hash = 0;
    uint32 cur_func_object_id = 0;
    int32 current_stack_index = CConsoleWindow::GetInstance()->GetDebugCallstackWin()->
                                                               GetSelectedStackEntry(cur_func_ns_hash, cur_func_hash,
                                                                                     cur_func_object_id);
    if (current_stack_index < 0)
    {
        return;
    }

    // -- if we're adding a namespace label
    if (watch_var_entry->mObjectID > 0)
    {
        AddObjectMemberEntry(*watch_var_entry);
    }

    // -- else see if we're adding a top level entry
    else if (watch_var_entry->mObjectID == 0 && watch_var_entry->mType != TinScript::TYPE_void)
    {
        AddTopLevelEntry(*watch_var_entry);
    }
}

// ====================================================================================================================
// NotifyBreakpointHit():  Notification that the callstack has been updated, all watch entries are complete
// ====================================================================================================================
void CDebugWatchWin::NotifyBreakpointHit()
{
    // -- loop through all entries, remove the children of any object variable that is invalid
    int entry_index = 0;
    while (entry_index < mWatchList.size())
    {
        CWatchEntry* entry = mWatchList.at(entry_index);
        if (entry->mDebuggerEntry.mFunctionHash != 0 && entry->mDebuggerEntry.mType == TinScript::TYPE_object &&
            entry->mDebuggerEntry.mVarObjectID == 0)
        {
            // -- retrieve the original object ID
            uint32 object_id = 0;
            if (entry->childCount() > 0)
            {
                CWatchEntry* child_entry = static_cast<CWatchEntry*>(entry->child(0));
                object_id = child_entry->mDebuggerEntry.mObjectID;
            }

            // -- now remove any children from the main watch list (starting with the next index)
            ++entry_index;

            // -- remove all children (if we had any)
            if (object_id > 0)
            {
                while (entry->childCount() > 0)
                    entry->removeChild(entry->child(0));

                while (entry_index < mWatchList.size())
                {
                    CWatchEntry* child_entry = mWatchList.at(entry_index);
                    if (entry->mDebuggerEntry.mObjectID == object_id)
                    {
                        mWatchList.removeAt(entry_index);
                        delete child_entry;
                    }

                    // -- otherwise we're done
                    else
                    {
                        break;
                    }
                }
            }
        }

        // -- else next index
        else
        {
            ++entry_index;
        }
    }
}


// ====================================================================================================================
// NotifyNewCallStack():  Called when the callstack has changed, so we can verify/purge invalid watches
// ====================================================================================================================
void CDebugWatchWin::NotifyUpdateCallstack(bool breakpointHit)
{
    // -- get the stack window
    CDebugCallstackWin* stackWindow = CConsoleWindow::GetInstance()->GetDebugCallstackWin();
    if (!stackWindow)
        return;

    uint32 cur_func_ns_hash = 0;
    uint32 cur_func_hash = 0;
    uint32 cur_func_object_id = 0;
    int32 current_stack_index = stackWindow->GetSelectedStackEntry(cur_func_ns_hash, cur_func_hash,
                                                                   cur_func_object_id);
    if (current_stack_index < 0)
    {
        return;
    }

    // -- if the reason we're updating the callstack, is because of a breakpoint, then we need to
    // -- mark all entries as "unconfirmed" until we receive a debugger watch entry confirming they
    // -- still exist.
    // -- Specifically, if an object is deleted, the object var still exists, but
    // -- the members need to be removed
    /*
    if (breakpointHit)
    {
        int entry_index = 0;
        while (entry_index < mWatchList.size())
        {
            CWatchEntry* entry = mWatchList.at(entry_index);
            entry->mUnconfirmed = true;
            ++entry_index;
        }
    }
    */

    // -- loop through all watches
    int entry_index = 0;
    while (entry_index < mWatchList.size())
    {
        // -- if the watch has a calling function, see if that calling function is still valid in the current stack
        // -- note:  only loop through top level entries, as members are automatically deleted when the object entry is
        bool removed = false;
        CWatchEntry* entry = mWatchList.at(entry_index);
        if (entry->mDebuggerEntry.mObjectID == 0 && entry->mDebuggerEntry.mFunctionHash != 0)
        {
            int32 stack_index = stackWindow->ValidateStackEntry(entry->mDebuggerEntry.mFuncNamespaceHash,
                                                                entry->mDebuggerEntry.mFunctionHash,
                                                                entry->mDebuggerEntry.mFunctionObjectID);
            if (stack_index < 0)
            {
                // -- remove this entry
                mWatchList.removeAt(entry_index);

                // -- if this is an object, we need to delete its children as well
                if (entry->mDebuggerEntry.mType == TinScript::TYPE_object && entry->childCount() > 0)
                {
                    // -- ensure we have a valid object ID
                    CWatchEntry* child_entry = static_cast<CWatchEntry*>(entry->child(0));
                    uint32 object_id = child_entry->mDebuggerEntry.mObjectID;

                    // -- clear the children
                    while (entry->childCount() > 0)
                        entry->removeChild(entry->child(0));

                    // -- now starting from the next index, delete the children of this object
                    while (entry_index < mWatchList.size())
                    {
                        CWatchEntry* child_entry = mWatchList.at(entry_index);
                        if (entry->mDebuggerEntry.mObjectID == object_id)
                        {
                            mWatchList.removeAt(entry_index);
                            delete child_entry;
                        }

                        // -- otherwise we're done
                        else
                        {
                            break;
                        }
                    }
                }

                // -- now delete the entry
                delete entry;
                removed = true;
            }

            // -- otherwise, see if we need to hide the items
            else
            {
                bool hidden = (stack_index != current_stack_index);
                entry->setHidden(hidden);

                // -- if this entry is an object, we need to hide all of its children
                for (int i = 0; i < entry->childCount(); ++i)
                {
                    QTreeWidgetItem* child = entry->child(i);
                    child->setHidden(hidden);
                }
            }
        }

        // -- if we didn't remove the entry, we need to bump the index
        if (!removed)
            ++entry_index;
    }
}

// ------------------------------------------------------------------------------------------------
#include "TinQTWatchWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
