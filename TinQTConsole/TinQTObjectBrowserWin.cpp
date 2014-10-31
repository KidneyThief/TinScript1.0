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

// ====================================================================================================================
// TinQTObjectBrowserWin.cpp
// ====================================================================================================================

#include "stdafx.h"

#include <QLineEdit>
#include <QKeyEvent>
#include <QMessageBox>

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTObjectBrowserWin.h"
#include "TinQTConsole.h"
#include "mainwindow.h"

// == class CBrowserEntry =============================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CBrowserEntry::CBrowserEntry(uint32 parent_id, uint32 object_id, const char* object_name, const char* derivation)
    : QTreeWidgetItem()
{
    mObjectID = object_id;
    mParentID = parent_id;
    TinScript::SafeStrcpy(mName, object_name, TinScript::kMaxNameLength);
    TinScript::SafeStrcpy(mDerivation, derivation, TinScript::kMaxNameLength);

    // -- set the expanded flag
    mExpanded = false;

    // -- set the QT elements
    setText(0, mName);
    setText(1, mDerivation);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CBrowserEntry::~CBrowserEntry()
{
}

// == class CDebugObjectBrowserWin ====================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CDebugObjectBrowserWin::CDebugObjectBrowserWin(QWidget* parent) : QTreeWidget(parent)
{
    setColumnCount(2);
    setItemsExpandable(true);
    setExpandsOnDoubleClick(true);

    // -- set the headers
    QStringList headers;
    headers.append(tr("Object Hierarchy"));
    headers.append(tr("Derivation"));
    setHeaderLabels(headers);
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CDebugObjectBrowserWin::~CDebugObjectBrowserWin()
{
    RemoveAll();
}

// ====================================================================================================================
// NotifyOnConnect():  Called when the debugger's connection to the target is initially confirmed.
// ====================================================================================================================
void CDebugObjectBrowserWin::NotifyOnConnect()
{
    // -- request a fresh population of the existing objects
    SocketManager::SendCommand("DebuggerListObjects();");
}

// ====================================================================================================================
// NotifyCreateObject():  Notify a new object has been created.
// ====================================================================================================================
void CDebugObjectBrowserWin::NotifyCreateObject(uint32 object_id, const char* object_name, const char* derivation)
{
    // -- if we already have an entry for this object, we're done
    if (mObjectDictionary.contains(object_id))
        return;
    
    // -- create the list, add it to the object dictionary
    QList<CBrowserEntry*>* entry_list = new QList<CBrowserEntry*>();
    mObjectDictionary.insert(object_id, entry_list);

    // -- now create the actual entry, and add it to the list
    CBrowserEntry* new_entry = new CBrowserEntry(0, object_id, object_name, derivation);
    entry_list->append(new_entry);

    // -- until we're parented, we want to display the entry
    addTopLevelItem(new_entry);
}

// ====================================================================================================================
// NotifyDestroyObject():  Notify an object has been destroyed.
// ====================================================================================================================
void CDebugObjectBrowserWin::NotifyDestroyObject(uint32 object_id)
{
    // -- remove all entries from the dictionary
    if (!mObjectDictionary.contains(object_id))
        return;

    // -- only the first entry ever needs to be deleted, as all others are parented
    // -- and therefore deleted when the parent entry is deleted
    QList<CBrowserEntry*>* object_entry_list = mObjectDictionary[object_id];
    CBrowserEntry* object_entry = (*object_entry_list)[0];
    delete object_entry;
    object_entry_list->clear();

    // -- remove the list from the dictionary
    mObjectDictionary.remove(object_id);
    delete object_entry_list;
}

// ====================================================================================================================
// NotifySetAddObject():  Notify an object has been destroyed.
// ====================================================================================================================
void CDebugObjectBrowserWin::NotifySetAddObject(uint32 set_id, uint32 object_id)
{
    // -- ensure both objects exist
    if (!mObjectDictionary.contains(set_id) || !mObjectDictionary.contains(object_id))
        return;

    // -- get the original object entry (so we can clone the name and hierarchy)
    QList<CBrowserEntry*>* object_entry_list = mObjectDictionary[object_id];
    QList<CBrowserEntry*>* set_entry_list = mObjectDictionary[set_id];
    CBrowserEntry* object_entry = (*object_entry_list)[0];

    // -- for each entry in the set_entry_list, add a new object_entry
    for (int i = 0; i < set_entry_list->size(); ++i)
    {
        CBrowserEntry* set_entry = (*set_entry_list)[i];
        CBrowserEntry* new_entry = new CBrowserEntry(set_id, object_id, object_entry->mName,
                                                     object_entry->mDerivation);

        // -- add the new entry as a child
        set_entry->addChild(new_entry);

        // -- add the new entry to our entry list
        object_entry_list->append(new_entry);
    }

    // -- and of course, the original object entry (at the root level) is now hidden
    object_entry->setHidden(true);
}

// ====================================================================================================================
// NotifySetRemoveObject():  Notify that an object is no longer a member of a set.
// ====================================================================================================================
void CDebugObjectBrowserWin::NotifySetRemoveObject(uint32 set_id, uint32 object_id)
{
    // -- ensure the object exists (and we have a valid set_id)
    if (!mObjectDictionary.contains(object_id) || set_id == 0)
        return;

    // -- get the list of all instances of the object
    QList<CBrowserEntry*>* object_entry_list = mObjectDictionary[object_id];

    // -- find the instance belonging to the set
    for (int i = 1; i < object_entry_list->size(); ++i)
    {
        // -- get the entry, see if it's the one matching the set
        CBrowserEntry* object_entry = (*object_entry_list)[i];
        if (object_entry->mParentID == set_id)
        {
            // -- remove, delete and break
            object_entry_list->removeAt(i);
            delete object_entry;
            break;
        }
    }

    // -- if the size of the object_entry_list is now just the original, ensure it is no longer hidden
    if (object_entry_list->size() == 1)
    {
        CBrowserEntry* object_entry = (*object_entry_list)[0];
        object_entry->setHidden(false);
    }
}

// ====================================================================================================================
// RemoveAll():  Remove all browser entries.
// ====================================================================================================================
void CDebugObjectBrowserWin::RemoveAll()
{
    // -- clear the map of all object entries
    while (mObjectDictionary.size() > 0)
    {
        uint32 object_id = mObjectDictionary.begin().key();
        NotifyDestroyObject(object_id);
    }
}

// ====================================================================================================================
// GetSelectedObjectID():  Return the objectID of the currently selected entry.
// ====================================================================================================================
uint32 CDebugObjectBrowserWin::GetSelectedObjectID()
{
    CBrowserEntry* cur_item = static_cast<CBrowserEntry*>(currentItem());
    if (cur_item)
    {
        return (cur_item->mObjectID);
    }

    // -- no current objects selected
    return (0);
}

// ====================================================================================================================
// GetObjectIdentifier():  Returns the identifier (formated ID and object name) for the requested entry.
// ====================================================================================================================
const char* CDebugObjectBrowserWin::GetObjectIdentifier(uint32 object_id)
{
    if (mObjectDictionary.contains(object_id))
    {
        // -- dereference to get the List, and then again to get the first item in the list
        CBrowserEntry* entry = (*(mObjectDictionary[object_id]))[0];
        return (entry->mName);
    }

    // -- not found
    return ("");
}

// --------------------------------------------------------------------------------------------------------------------
#include "TinQTObjectBrowserWinMoc.cpp"

// ====================================================================================================================
// eof
// ====================================================================================================================
