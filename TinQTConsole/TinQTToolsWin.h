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
// TinQTToolsWin.h
// ====================================================================================================================

#ifndef __TINQTTOOLSWIN_H
#define __TINQTTOOLSWIN_H

#include <qpushbutton.h>
#include <qgridlayout.h>
#include <qlineedit.h>
#include <qbytearray.h>
#include <qscrollarea.h>

// --------------------------------------------------------------------------------------------------------------------
// -- forward declarations
class QLabel;
class QScroller;
class QScrollArea;
class QButton;

// ====================================================================================================================
// class CDebugToolEntry:  The base class for gui elements to be added to a ToolPalette window.
// ====================================================================================================================
class CDebugToolEntry : public QWidget
{
    public:
        CDebugToolEntry(CDebugToolsWin* parent);
        virtual ~CDebugToolEntry();

        int32 Initialize(const char* name, const char* description, QWidget* element);
        int32 GetEntryID() const
        {
            return (mEntryID);
        }

    protected:
        static int32 gToolsWindowElementIndex;
        int32 mEntryID;
        CDebugToolsWin* mParent;

        QLabel* mName;
        QLabel* mDescription;
};

// ====================================================================================================================
// CDebugToolMessage:  Gui element of type "message", to be added to a ToolPalette
// ====================================================================================================================
class CDebugToolMessage : public CDebugToolEntry
{
    public:
        CDebugToolMessage(const char* message, CDebugToolsWin* parent);
        virtual ~CDebugToolMessage();

    protected:
        QLabel* mMessage;
};

// ====================================================================================================================
// CDebugToolButton:  Gui element of type "button", to be added to a ToolPalette
// ====================================================================================================================
class CDebugToolButton : public CDebugToolEntry
{
    Q_OBJECT

    public:
        CDebugToolButton(const char* name, const char* description, const char* label, const char* command,
                         CDebugToolsWin* parent);
        virtual ~CDebugToolButton();

    public slots:
        void OnButtonPressed();

    protected:
        QPushButton* mButton;
        char mCommand[TinScript::kMaxTokenLength];
};

// ====================================================================================================================
// class CDebugToolsWin:  The base class for ToolPalette windows
// ====================================================================================================================

class CDebugToolsWin : public QWidget
{
    Q_OBJECT

    public:
        CDebugToolsWin(const char* tools_name, QWidget* parent);
        virtual ~CDebugToolsWin();

        virtual void paintEvent(QPaintEvent* e)
        {
            ExpandToParentSize();
            //QListWidget::paintEvent(e);
        }

        void ExpandToParentSize()
        {
            // -- resize to be the parent widget's size, with room for the title
            QSize parentSize = parentWidget()->size();
            int newWidth = parentSize.width();
            int newHeight = parentSize.height() - 24;
            if (newHeight < 20)
                newHeight = 20;
            setGeometry(0, 20, newWidth, newHeight);
            updateGeometry();

            mScrollArea->setGeometry(0, 20, newWidth, newHeight);
            mScrollArea->updateGeometry();
        }

        // -- interface to populate with GUI elements
        void ClearAll();
        int GetEntryCount() { return (mEntryList.size()); }
        QGridLayout* GetLayout() { return (mLayout); }
        QWidget* GetContent() { return (mScrollContent); }
        QScrollArea* GetScrollArea() { return (mScrollArea); }
        void AddEntry(CDebugToolEntry* entry) { mEntryList.push_back(entry); }

        int32 AddMessage(const char* message);
        int32 AddButton(const char* name, const char* description, const char* value, const char* command);

    private:
        char mWindowName[TinScript::kMaxNameLength];
        QList<CDebugToolEntry*> mEntryList;
        QGridLayout* mLayout;
        QScrollArea* mScrollArea;
        QWidget* mScrollContent;
};

#endif

// ====================================================================================================================
// EOF
// ====================================================================================================================
