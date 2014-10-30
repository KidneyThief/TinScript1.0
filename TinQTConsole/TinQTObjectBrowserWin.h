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
// TinQTObjectBrowser.h

#ifndef __TINQTOBJECTBROWSERWIN_H
#define __TINQTOBJECTBROWSERWIN_H

#include <qgridlayout.h>
#include <qtreewidget.h>
#include <qlabel.h>
#include <QLineEdit>

#include "../source/TinScript.h"

// --------------------------------------------------------------------------------------------------------------------
// -- forward declaration
class CConsoleWindow;

// ====================================================================================================================
// class CBrowserEntry:  Defines an entry in the tree hierarchy of objects.
// ====================================================================================================================
class CBrowserEntry : public QTreeWidgetItem
{
    public:
        CBrowserEntry(uint32 parent_id, uint32 object_id, const char* object_name, const char* derivation);
        virtual ~CBrowserEntry();

        uint32 mObjectID;
        uint32 mParentID;
        char mName[TinScript::kMaxNameLength];
        char mDerivation[TinScript::kMaxNameLength];

        bool mExpanded;
};

// ------------------------------------------------------------------------------------------------
class CDebugObjectBrowserWin : public QTreeWidget
{
    Q_OBJECT

    public:
        CDebugObjectBrowserWin(QWidget* parent);
        virtual ~CDebugObjectBrowserWin();

        void NotifyCreateObject(uint32 object_id, const char* object_name, const char* derivation);
        void NotifyDestroyObject(uint32 object_id);
        void NotifySetAddObject(uint32 set_id, uint32 object_id);
        void NotifySetRemoveObject(uint32 set_id, uint32 object_id);
        void RemoveAll();

        virtual void paintEvent(QPaintEvent* e)
        {
            ExpandToParentSize();
            QTreeWidget::paintEvent(e);
        }

        virtual void resizeEvent(QResizeEvent* e)
        {
            ExpandToParentSize();
            QTreeWidget::resizeEvent(e);
        }

        void ExpandToParentSize()
        {
            // -- resize to be the parent widget's size, with room for the title
            QSize parentSize = parentWidget()->size();
            int newWidth = parentSize.width();
            int newHeight = parentSize.height();
            if (newHeight < 20)
                newHeight = 20;
            setGeometry(0, 20, newWidth, newHeight);
            updateGeometry();
        }

    protected:
        virtual void keyPressEvent(QKeyEvent * event);

    private:
        // -- the dictionary of objects, each list is another instance of the same entry, with a different parent instance
        QMap<uint32, QList<CBrowserEntry*>* > mObjectDictionary;

        // -- the parentOf dictionary is a list of all the children belonging to the object
        // -- all children in the parentOf dictionary will show up in the TreeView, they must be allocated
        QMap<uint32, QMap<uint32, CBrowserEntry*>* > mParentOfDictionary;

        // -- the childOf dictionary is a list of all the sets the object belongs to
        // -- the CBrowserEntries in this list are allocated, as they show up in the TreeView, so they must be deleted
        QMap<uint32, QMap<uint32, CBrowserEntry*>* > mChildOfDictionary;
};

#endif //__TINQTOBJECTBROWSERWIN_H

// ====================================================================================================================
// EOF
// ====================================================================================================================
