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
// TinQTToolsWin.cpp : A list view of tool widgets used to conveniently submit commands to a target application.
// ====================================================================================================================

#include "stdafx.h"

#include <QListWidget>
#include <QLabel>
#include <QScroller>
#include <QScrollArea>
#include <QPushButton>

#include "TinScript.h"
#include "TinRegistration.h"

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTBreakpointsWin.h"
#include "TinQTToolsWin.h"
#include "mainwindow.h"

// --------------------------------------------------------------------------------------------------------------------
// -- statics
int32 CDebugToolEntry::gToolsWindowElementIndex = 0;

// == CDebugToolEntry =================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CDebugToolEntry::CDebugToolEntry(CDebugToolsWin* parent) : QWidget()
{
    mEntryID = 0;
    mParent = parent;
}

// ====================================================================================================================
// Deconstructor
// ====================================================================================================================
CDebugToolEntry::~CDebugToolEntry()
{
    // -- delete the elements
    delete mName;
    delete mDescription;
}

// ====================================================================================================================
// Initialize():  Populates the layout with the elements for this 
// ====================================================================================================================
int32 CDebugToolEntry::Initialize(const char* name, const char* description, QWidget* element)
{
    // -- ensure we don't initialize multiple times
    if (mEntryID > 0)
        return (mEntryID);

    mEntryID = ++gToolsWindowElementIndex;

    // -- get the current number of entries added to this window
    int count = mParent->GetEntryCount();

    QSize parentSize = mParent->size();
    int newWidth = parentSize.width();
    mParent->GetContent()->setGeometry(0, 20, newWidth, (count + 2) * 24);

    mName = new QLabel(name);
    mDescription = new QLabel(description);

    // -- add this to the window
    mParent->GetLayout()->addWidget(mName, count, 0, 1, 1);
    mParent->GetLayout()->addWidget(element, count, 1, 1, 1);
    mParent->GetLayout()->addWidget(mDescription, count, 2, 1, 1);
    mParent->AddEntry(this);

    mParent->GetContent()->updateGeometry();
    mParent->ExpandToParentSize();

    return (mEntryID);
}

// == CDebugToolMessage ===============================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CDebugToolMessage::CDebugToolMessage(const char* message, CDebugToolsWin* parent) : CDebugToolEntry(parent)
{
    mMessage = new QLabel(message);
    Initialize("", "", mMessage);
}

// ====================================================================================================================
// Deconstructor
// ====================================================================================================================
CDebugToolMessage::~CDebugToolMessage()
{
    delete mMessage;
}

// == CDebugToolButton ==================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CDebugToolButton::CDebugToolButton(const char* name, const char* description, const char* value, const char* command,
                                   CDebugToolsWin* parent)
    : CDebugToolEntry(parent)
{
    // -- copy the command
    TinScript::SafeStrcpy(mCommand, command, TinScript::kMaxTokenLength);

    // -- create the button
    mButton = new QPushButton(value);
    Initialize(name, description, mButton);

    // -- hook up the button
    QObject::connect(mButton, SIGNAL(clicked()), this, SLOT(OnButtonPressed()));
};

// ====================================================================================================================
// Deconstructor
// ====================================================================================================================
CDebugToolButton::~CDebugToolButton()
{
    delete mButton;
}

// ====================================================================================================================
// OnButtonPressed():  Slot hooked up to the button, to execute the command when pressed.
// ====================================================================================================================
void CDebugToolButton::OnButtonPressed()
{
    bool8 is_connected = CConsoleWindow::GetInstance()->IsConnected();
    if (is_connected)
    {
        ConsolePrint("%s%s\n", kConsoleSendPrefix, mCommand);
        SocketManager::SendCommandf(mCommand);
    }
    else
    {
        ConsolePrint("%s%s\n", kLocalSendPrefix, mCommand);
        TinScript::ExecCommand(mCommand);
    }
}

// == CDebugToolsWin ==================================================================================================

// ====================================================================================================================
// Constructor
// ====================================================================================================================
CDebugToolsWin::CDebugToolsWin(const char* tools_name, QWidget* parent) : QWidget(parent)
{
    TinScript::SafeStrcpy(mWindowName, tools_name, TinScript::kMaxNameLength);
    mScrollArea = new QScrollArea(this);
    mScrollContent = new QWidget(mScrollArea);
    mLayout = new QGridLayout(mScrollContent);
    mLayout->setColumnStretch(2, 1);
    mScrollArea->setWidget(mScrollContent);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ExpandToParentSize();
}

// ====================================================================================================================
// Destructor
// ====================================================================================================================
CDebugToolsWin::~CDebugToolsWin()
{
}

// ====================================================================================================================
// ClearAll():  Delete all elements,from this window - allows the window to be repopulated
// ====================================================================================================================
void CDebugToolsWin::ClearAll()
{
    for (int i = 0; i < mEntryList.size(); ++i)
    {
        CDebugToolEntry* entry = mEntryList[i];
        delete entry;
    }
    mEntryList.clear();

    // -- re-create the layout, which should clean up oall the widgets parented to it
    delete mLayout;
    mLayout = new QGridLayout(mScrollContent);
    mLayout->setColumnStretch(2, 1);
}

// ====================================================================================================================
// AddMessage():  Adds a gui entry of type "message" to the ToolPalette window
// ====================================================================================================================
int32 CDebugToolsWin::AddMessage(const char* message)
{
    // -- create the message entry
    CDebugToolMessage* new_entry = new CDebugToolMessage(message, this);
    if (new_entry)
        return (new_entry->GetEntryID());

    // -- failed to create the message
    return (0);
}

// ====================================================================================================================
// AddMessage():  Adds a gui entry of type "message" to the ToolPalette window
// ====================================================================================================================
int32 CDebugToolsWin::AddButton(const char* name, const char* description, const char* value, const char* command)
{
    // -- create the message entry
    CDebugToolButton* new_entry = new CDebugToolButton(name, description, value, command, this);
    if (new_entry)
        return (new_entry->GetEntryID());

    // -- failed to create the message
    return (0);
}

// ------------------------------------------------------------------------------------------------
#include "TinQTToolsWinMoc.cpp"

// ------------------------------------------------------------------------------------------------
// eof
// ------------------------------------------------------------------------------------------------
