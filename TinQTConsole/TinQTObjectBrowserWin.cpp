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
CBrowserEntry::CBrowserEntry(CBrowserEntry* parent_entry, uint32 object_id, const char* object_name,
                             const char* derivation)
    : QTreeWidgetItem()
{
    mParent = parent_entry;
    mObjectID = object_id;
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
    // -- notify our parent
    if (mParent)
    {
        mParent->RemoveChild(this);
    }

    // -- remove all children
    while (mChildList.size() > 0)
    {
        CBrowserEntry* child = mChildList[0];
        RemoveChild(child);
        delete child;
    }
}

// ====================================================================================================================
// AddChild():  Add a child to this entry.
// ====================================================================================================================
void CBrowserEntry::AddChild(CBrowserEntry* child)
{
    // -- add the child entry, if it is not already in the list
    if (!mChildList.contains(child))
    {
        mChildList.append(child);
        child->mParent = this;
    }
}

// ====================================================================================================================
// RemoveChild():  Remove a child from this entry.
// ====================================================================================================================
void CBrowserEntry::RemoveChild(CBrowserEntry* child)
{
    // -- remove the child, if it is in the list
    if (mChildList.contains(child))
    {
        mChildList.removeOne(child);
        child->mParent = NULL;
    }
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
// AddObject():  Given the parent ID and object attributes, add a BrowserEntry to every instance of the parent entry.
// ====================================================================================================================
void CDebugObjectBrowserWin::AddObject(uint32 parent_id, uint32 object_id, const char* object_name,
                                       const char* derivation)
{
    // -- see if we have a parent
    if (parent_id != 0)
    {
        if (!mObjectDictionary.contains(parent_id))
        {
            char error_msg[64];
            sprintf_s(error_msg, 64, "Error - parent object %d not found", parent_id);
            QMessageBox::warning(this, tr("Error"), QString(error_msg));
            return;
        }

        // -- ensure we have an entry for the object
        QList<CBrowserEntry*>* object_entry_list = mObjectDictionary[object_id];
        if (!object_entry_list)
        {
            object_entry_list = new QList<CBrowserEntry*>();
            mObjectDictionary.insert(object_id, object_entry_list);
        }

        // -- loop through all entries in the parent entry list, and add a child entry for this object
        QList<CBrowserEntry*>* parent_entry_list = mObjectDictionary[parent_id];
        for (int i = 0; i < (*parent_entry_list).size(); ++i)
        {
            CBrowserEntry* parent_entry = (*parent_entry_list)[i];
            CBrowserEntry* new_entry = new CBrowserEntry(parent_entry, object_id, object_name, derivation);
            object_entry_list->append(new_entry);
            parent_entry->addChild(new_entry);
        }
    }

    // -- otherwise, no parent - simply add this top level entry
    else
    {
        CBrowserEntry* new_entry = new CBrowserEntry(NULL, object_id, object_name, derivation);
        QList<CBrowserEntry*>* new_list = new QList<CBrowserEntry*>();
        mObjectDictionary.insert(object_id, new_list);
        new_list->append(new_entry);
        addTopLevelItem(new_entry);
    }
}

// ====================================================================================================================
// RemoveObject():  Remove all browser entries refering to the given object ID.
// ====================================================================================================================
void CDebugObjectBrowserWin::RemoveObject(uint32 object_id)
{
}

// ====================================================================================================================
// RemoveAll():  Remove all browser entries.
// ====================================================================================================================
void CDebugObjectBrowserWin::RemoveAll()
{
    while (mRootObjectList.size() > 0)
    {
        CBrowserEntry* entry = mRootObjectList[0];
        mRootObjectList.removeOne(entry);
        delete entry;
    }

    // -- clear the dictionary
    while (mObjectDictionary.size() > 0)
    {
        uint32 object_id = mObjectDictionary.begin().key();
        QList<CBrowserEntry*>* entry_list = mObjectDictionary[object_id];
        mObjectDictionary.remove(object_id);
        delete entry_list;
    }

    // -- clear the tree items
    clear();
}

// ====================================================================================================================
// keyPressEvent():  Handler for key presses, when a watch window is in focus
// ====================================================================================================================
void CDebugObjectBrowserWin::keyPressEvent(QKeyEvent* event)
{
    if (!event)
        return;

    // -- delete the selected, if we have a selected
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        CBrowserEntry* cur_item = static_cast<CBrowserEntry*>(currentItem());
        if (cur_item)
        {
        }
        return;
    }

    // -- pass to the base class for handling
    QTreeWidget::keyPressEvent(event);
}

// --------------------------------------------------------------------------------------------------------------------
#include "TinQTObjectBrowserWinMoc.cpp"

// ====================================================================================================================
// eof
// ====================================================================================================================
