/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "stdafx.h"

#include "mainwindow.h"

#include <QAction>
#include <QLayout>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <qdebug.h>

#include "TinQTConsole.h"
#include "TinQTSourceWin.h"
#include "TinQTWatchWin.h"
#include "TinQTBreakpointsWin.h"

// ====================================================================================================================
// -- class implementation for add variable watch dialog
class CreateVarWatchDialog : public QDialog
{
public:
    CreateVarWatchDialog(QWidget *parent = 0);

    void SetVariableName(const char* variable_name);
    QString GetVariableName() const;
    bool IsBreakOnWrite() const;

private:
    QLineEdit *mVariableName;
    QCheckBox* mBreakOnWrite;
};

CreateVarWatchDialog::CreateVarWatchDialog(QWidget *parent)
    : QDialog(parent)
{
	setWindowTitle("Add Variable Watch");
	setMinimumWidth(280);
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Variable:")), 0, 0);
    mVariableName = new QLineEdit;
    layout->addWidget(mVariableName, 0, 1);

    mBreakOnWrite = new QCheckBox;
    layout->addWidget(mBreakOnWrite, 1, 0);
    layout->addWidget(new QLabel(tr("Break On Write")), 1, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout, 2, 0, 1, 2);
    buttonLayout->addStretch();

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);
    QPushButton *okButton = new QPushButton(tr("Ok"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(okButton);

    okButton->setDefault(true);
}

void CreateVarWatchDialog::SetVariableName(const char* variable_name)
{
    mVariableName->setText(QString(variable_name));
}

QString CreateVarWatchDialog::GetVariableName() const
{
    return (mVariableName->text());
}

bool CreateVarWatchDialog::IsBreakOnWrite() const
{
    return (mBreakOnWrite->isChecked());
}

// ====================================================================================================================
// -- class implementation for the "go to line #" dialog
class CreateGoToLineDialog : public QDialog
{
public:
    CreateGoToLineDialog(QWidget *parent = 0);

    int GetLineNumber() const;

private:
    QLineEdit *mGoToLineEdit;
};

CreateGoToLineDialog::CreateGoToLineDialog(QWidget *parent)
    : QDialog(parent)
{
	setWindowTitle("Go To Line");
	setMinimumWidth(280);
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Line number:")), 0, 0);
    mGoToLineEdit = new QLineEdit;
    layout->addWidget(mGoToLineEdit, 0, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout, 2, 0, 1, 2);
    buttonLayout->addStretch();

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);
    QPushButton *okButton = new QPushButton(tr("Ok"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(okButton);

    okButton->setDefault(true);
}

int CreateGoToLineDialog::GetLineNumber() const
{
    return (atoi(mGoToLineEdit->text().toUtf8()));
}

// ====================================================================================================================
// -- class implementation for add variable watch dialog
class CreateBreakConditionDialog : public QDialog
{
public:
    CreateBreakConditionDialog(QWidget *parent = 0);

    void SetCondition(const char* variable_name, bool cond_enabled);
    QString GetCondition() const;
    bool IsEnabled() const;

    void SetTraceExpression(const char* trace_expr, bool trace_enabled, bool trace_on_cond);
    QString GetTraceExpression() const;
    bool IsTraceEnabled() const;

    bool IsTraceOnCondition() const;

private:
    QLineEdit *mCondition;
    QCheckBox* mIsEnabled;
    QLineEdit *mTracePoint;
    QCheckBox* mTraceEnabled;
    QCheckBox* mTraceOnCondition;
};

CreateBreakConditionDialog::CreateBreakConditionDialog(QWidget *parent)
    : QDialog(parent)
{
	setWindowTitle("Set Breakpoint Condition");
	setMinimumWidth(280);
    QGridLayout *layout = new QGridLayout(this);

    mIsEnabled = new QCheckBox;
    layout->addWidget(mIsEnabled, 0, 0);
    mIsEnabled->setCheckState(Qt::Checked);

    layout->addWidget(new QLabel(tr("Condition:")), 0, 1);
    mCondition = new QLineEdit;
    layout->addWidget(mCondition, 0, 2);

    mTraceEnabled = new QCheckBox;
    layout->addWidget(mTraceEnabled, 1, 0);
    mTraceEnabled->setCheckState(Qt::Unchecked);

    layout->addWidget(new QLabel(tr("Trace:")), 1, 1);
    mTracePoint = new QLineEdit;
    layout->addWidget(mTracePoint, 1, 2);

    QHBoxLayout *traceLayout = new QHBoxLayout;
    mTraceOnCondition = new QCheckBox;
    traceLayout->addWidget(mTraceOnCondition);
    traceLayout->addWidget(new QLabel("Trace On Condition"));
    traceLayout->addWidget(new QWidget(), 1);
    layout->addLayout(traceLayout, 2, 2, 1, 1);
    mTraceOnCondition->setCheckState(Qt::Unchecked);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout, 3, 0, 1, 3);
    buttonLayout->addStretch();

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);
    QPushButton *okButton = new QPushButton(tr("Ok"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(okButton);

    okButton->setDefault(true);
}

void CreateBreakConditionDialog::SetCondition(const char* condition, bool cond_enabled)
{
    mCondition->setText(QString(condition));
    mIsEnabled->setChecked(cond_enabled);
}

QString CreateBreakConditionDialog::GetCondition() const
{
    return (mCondition->text());
}

bool CreateBreakConditionDialog::IsEnabled() const
{
    return (mIsEnabled->isChecked());
}

void CreateBreakConditionDialog::SetTraceExpression(const char* expression, bool trace_enabled, bool trace_on_cond)
{
    mTracePoint->setText(QString(expression));
    mTraceEnabled->setChecked(trace_enabled);
    mTraceOnCondition->setChecked(trace_on_cond);
}

QString CreateBreakConditionDialog::GetTraceExpression() const
{
    return (mTracePoint->text());
}

bool CreateBreakConditionDialog::IsTraceEnabled() const
{
    return (mTraceEnabled->isChecked());
}

bool CreateBreakConditionDialog::IsTraceOnCondition() const
{
    return (mTraceOnCondition->isChecked());
}

// ====================================================================================================================
void MainWindow::menuAddVariableWatch()
{
    CreateVarWatchDialog dialog(this);
    int ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

	CConsoleWindow::GetInstance()->GetDebugWatchesWin()->
                                  AddVariableWatch(dialog.GetVariableName().toUtf8(),
                                                   dialog.IsBreakOnWrite());
}

void MainWindow::menuCreateVariableWatch()
{
    if (CConsoleWindow::GetInstance()->GetDebugWatchesWin()->hasFocus())
	    CConsoleWindow::GetInstance()->GetDebugWatchesWin()->CreateSelectedWatch();
    else if (CConsoleWindow::GetInstance()->GetDebugAutosWin()->hasFocus())
	    CConsoleWindow::GetInstance()->GetDebugAutosWin()->CreateSelectedWatch();
}

void MainWindow::menuSetBreakCondition()
{
    // -- we need to have a currently selected break condition, for this to be viable
    bool8 enabled = false;
    const char* condition = CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->GetBreakCondition(enabled);
    if (!condition)
        return;

    bool8 trace_enabled = false;
    bool8 trace_on_expr = false;
    const char* trace_expr = CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->
                                                            GetTraceExpression(trace_enabled, trace_on_expr);

    CreateBreakConditionDialog dialog(this);
    dialog.SetCondition(condition, enabled);
    dialog.SetTraceExpression(trace_expr, trace_enabled, trace_on_expr);
    int ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

    enabled = dialog.IsEnabled();
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->SetBreakCondition(dialog.GetCondition().toUtf8(),
                                                                               enabled);

    trace_enabled = dialog.IsTraceEnabled();
    trace_on_expr = dialog.IsTraceOnCondition();
    CConsoleWindow::GetInstance()->GetDebugBreakpointsWin()->SetTraceExpression(dialog.GetTraceExpression().toUtf8(),
                                                                                trace_enabled, trace_on_expr);
}

void MainWindow::menuGoToLine()
{
    CreateGoToLineDialog dialog(this);
    int ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

	CConsoleWindow::GetInstance()->GetDebugSourceWin()->GoToLineNumber(dialog.GetLineNumber());
}

// ====================================================================================================================

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

MainWindow::MainWindow(const QMap<QString, QSize> &customSizeHints,
                        QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");
    setWindowTitle("TinScript Remote Debugger");

    // -- default dock options...
    DockOptions opts;
    opts |= AnimatedDocks;
    opts |= AllowNestedDocks;
    opts |= AllowTabbedDocks;
    QMainWindow::setDockOptions(opts);

    setupMenuBar();
    setupDockWidgets(customSizeHints);
}

// -- destructor
MainWindow::~MainWindow()
{
    for (int i = 0; i < mScriptOpenActionList.size(); ++i)
    {
        delete mScriptOpenActionList[i];
    }
    mScriptOpenActionList.clear();
}

void MainWindow::AddScriptOpenAction(const char* fullPath)
{
    if (!fullPath || !fullPath[0])
        return;

    const char* fileName = CDebugSourceWin::GetFileName(fullPath);
    uint32 fileHash = TinScript::Hash(fileName);

    // -- ensure we haven't already added this action
    bool found = false;
    for (int i = 0; i < mScriptOpenActionList.size(); ++i)
    {
        CScriptOpenAction* scriptOpenAction = mScriptOpenActionList[i];
        if (scriptOpenAction->mFileHash == fileHash)
        {
            found = true;
            break;
        }
    }

    // -- if this entry already exists, we're done
    if (found)
        return;

    // -- create the script action
    QAction* action = mScriptsMenu->addAction(tr(fileName));
    CScriptOpenWidget *actionWidget = new CScriptOpenWidget(action, this);
    CScriptOpenAction* scriptOpenAction = new CScriptOpenAction(fullPath, fileHash, actionWidget);
    mScriptOpenActionList.push_back(scriptOpenAction);
    connect(action, SIGNAL(triggered()), actionWidget, SLOT(menuOpenScriptAction()));
}

// ====================================================================================================================
// CreateVariableWatch():  Called when using the dialog to create a watch, but with an initial string
// ====================================================================================================================
void MainWindow::CreateVariableWatch(const char* watch_string)
{
    CreateVarWatchDialog dialog(this);
    dialog.SetVariableName(watch_string);
    int ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

	CConsoleWindow::GetInstance()->GetDebugWatchesWin()->AddVariableWatch(dialog.GetVariableName().toUtf8(),
                                                                          dialog.IsBreakOnWrite());
}

// ====================================================================================================================
// menuOpenScriptAction():  Slot called when the menu option is selected for a dynamically added script open action.
// ====================================================================================================================
void MainWindow::menuOpenScriptAction(QAction *action)
{
    // -- loop through all the actions - find the one matching this action
    CScriptOpenAction* found = NULL;
    for (int i = 0; i < mScriptOpenActionList.size(); ++i)
    {
        CScriptOpenAction* scriptOpenAction = mScriptOpenActionList[i];
        if (scriptOpenAction->mActionWidget->mAction == action)
        {
            found = scriptOpenAction;
            break;
        }
    }

    // -- if we found our entry, open the file
    if (found)
    {
        CConsoleWindow::GetInstance()->GetDebugSourceWin()->OpenFullPathFile(found->mFullPath, true);
    }
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    QAction *action = menu->addAction(tr("Save layout..."));
    connect(action, SIGNAL(triggered()), this, SLOT(menuSaveLayout()));

    action = menu->addAction(tr("Load layout..."));
    connect(action, SIGNAL(triggered()), this, SLOT(menuLoadLayout()));

    action = menu->addAction(tr("Default layout"));
    connect(action, SIGNAL(triggered()), this, SLOT(defaultLoadLayout()));

    menu->addSeparator();

    menu->addAction(tr("&Quit"), this, SLOT(close()));

    // -- Dock Options menu
    mainWindowMenu = menuBar()->addMenu(tr("Main window"));

    action = mainWindowMenu->addAction(tr("Animated docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AnimatedDocks);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));

    action = mainWindowMenu->addAction(tr("Allow nested docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowNestedDocks);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));

    action = mainWindowMenu->addAction(tr("Allow tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowTabbedDocks);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));

    action = mainWindowMenu->addAction(tr("Force tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & ForceTabbedDocks);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));

    action = mainWindowMenu->addAction(tr("Vertical tabs"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & VerticalTabs);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));

    // -- Dock Widgets menu
    dockWidgetMenu = menuBar()->addMenu(tr("&Dock Options"));

    menuBar()->addSeparator();

    // -- Scripts menu
    QMenu* debug_menu = menuBar()->addMenu(tr("&Debug"));

    action = debug_menu->addAction(tr("Stop  [Shift + F5]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuDebugStop()));

    action = debug_menu->addAction(tr("Run  [F5]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuDebugRun()));

    action = debug_menu->addAction(tr("Step Over  [F10]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuDebugStepOver()));

    action = debug_menu->addAction(tr("Step In  [F11]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuDebugStepIn()));

    action = debug_menu->addAction(tr("Step Out  [Shift + F11]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuDebugStepOut()));

    debug_menu->addSeparator();

    action = debug_menu->addAction(tr("Add Watch  [Ctrl + W]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuAddVariableWatch()));

    action = debug_menu->addAction(tr("Watch Var  [Ctrl + Shift + W]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuCreateVariableWatch()));

    action = debug_menu->addAction(tr("Break Condition  [Ctrl + Shift + B]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuSetBreakCondition()));

    // -- Scripts menu
    mScriptsMenu = menuBar()->addMenu(tr("&Scripts"));

    action = mScriptsMenu->addAction(tr("Open Script..."));
    connect(action, SIGNAL(triggered()), this, SLOT(menuOpenScript()));

    mScriptsMenu->addSeparator();

    action = mScriptsMenu->addAction(tr("Goto Line  [Ctrl + G]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuGoToLine()));

    action = mScriptsMenu->addAction(tr("Search  [Ctrl + F]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuSearch()));

    action = mScriptsMenu->addAction(tr("Search Again  [F3]"));
    connect(action, SIGNAL(triggered()), this, SLOT(menuSearchAgain()));

    mScriptsMenu->addSeparator();

}

void MainWindow::setDockOptions()
{
    DockOptions opts;
    QList<QAction*> actions = mainWindowMenu->actions();

    if (actions.at(0)->isChecked())
        opts |= AnimatedDocks;
    if (actions.at(1)->isChecked())
        opts |= AllowNestedDocks;
    if (actions.at(2)->isChecked())
        opts |= AllowTabbedDocks;
    if (actions.at(3)->isChecked())
        opts |= ForceTabbedDocks;
    if (actions.at(4)->isChecked())
        opts |= VerticalTabs;

    QMainWindow::setDockOptions(opts);
}

// ====================================================================================================================
// menuSaveLayout():  The slot called by selecting the menu option to save the layout
// ====================================================================================================================
void MainWindow::menuSaveLayout()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    // -- write the layout to the opened file
    writeLayout(file);
}

// ====================================================================================================================
// autoSaveLayout():  Called automatically upon exiting, using a hardcoded file name
// ====================================================================================================================
void MainWindow::autoSaveLayout()
{
    QString fileName = "TinScript Debugger Layout";
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        return;

    writeLayout(file);
}

// ====================================================================================================================
// writeLayout():  The method to write the binary layout details to the given *open* file
// ====================================================================================================================
void MainWindow::writeLayout(QFile& file)
{
    QByteArray geo_data = saveGeometry();
    QByteArray layout_data = saveState();

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok)
        ok = file.write(geo_data) == geo_data.size();
    if (ok)
        ok = file.write(layout_data) == layout_data.size();

    if (!ok) {
        QString msg = tr("Error writing to %1\n%2")
                        .arg(file.fileName())
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

// ====================================================================================================================
// menuLoadLayout():  The slot called by selecting the menu option to load the layout
// ====================================================================================================================
void MainWindow::menuLoadLayout()
{
    QString fileName
        = QFileDialog::getOpenFileName(this, tr("Load layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    readLayout(file);
}

// ====================================================================================================================
// autoLoadLayout():  Automatically called when the application starts, to load the last (or default) layout.
// ====================================================================================================================
void MainWindow::autoLoadLayout()
{
    QString fileName = "TinScript Debugger Layout";
    QString defaultFileName = "TinScript Default Layout";
    QFile file(fileName);
    QFile defaultFile(defaultFileName);
    QFile* activeFile = &file;
    bool result = file.open(QFile::ReadOnly);
    if (!result)
    {
        activeFile = &defaultFile;
        fileName = defaultFileName;
        result = defaultFile.open(QFile::ReadOnly);
    }
    if (!result)
        return;

    readLayout(*activeFile);
}

// ====================================================================================================================
// defaultLoadLayout():  Slot called to specifically load the default layout.
// ====================================================================================================================
void MainWindow::defaultLoadLayout()
{
    QString defaultFileName = "TinScript Default Layout";
    QFile defaultFile(defaultFileName);
    bool result = defaultFile.open(QFile::ReadOnly);
    if (!result)
        return;

    readLayout(defaultFile);
}

// ====================================================================================================================
// readLayout():  Loads the binary layout from the given *open* file.
// ====================================================================================================================
void MainWindow::readLayout(QFile& file)
{
    uchar geo_size;
    QByteArray geo_data;
    QByteArray layout_data;

    bool ok = file.getChar((char*)&geo_size);
    if (ok)
    {
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }

    if (ok)
    {
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok)
        ok = restoreGeometry(geo_data);
    if (ok)
        ok = restoreState(layout_data);

    if (!ok)
    {
        QString msg = tr("Error reading %1").arg(file.fileName());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

// ====================================================================================================================
// menuDebugStop():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuDebugStop()
{
    CConsoleWindow::GetInstance()->GetInput()->OnButtonStopPressed();
}

// ====================================================================================================================
// menuDebugRun():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuDebugRun()
{
    CConsoleWindow::GetInstance()->GetInput()->OnButtonRunPressed();
}

// ====================================================================================================================
// menuDebugStepOver():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuDebugStepOver()
{
    CConsoleWindow::GetInstance()->GetInput()->OnButtonStepPressed();
}

// ====================================================================================================================
// menuDebugStepIn():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuDebugStepIn()
{
    CConsoleWindow::GetInstance()->GetInput()->OnButtonStepInPressed();
}

// ====================================================================================================================
// menuDebugStepOut():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuDebugStepOut()
{
    CConsoleWindow::GetInstance()->GetInput()->OnButtonStepOutPressed();
}

// ====================================================================================================================
// menuOpenScript():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuOpenScript()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("OpenScript"));
    if (fileName.isEmpty())
        return;

    CConsoleWindow::GetInstance()->GetDebugSourceWin()->OpenFullPathFile(fileName.toUtf8(), true);
}

// ====================================================================================================================
// menuSearch():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuSearch()
{
    CConsoleInput* console_input = CConsoleWindow::GetInstance()->GetInput();
    console_input->OnFindEditFocus();
}

// ====================================================================================================================
// menuSearch():  Slot called when the menu option is selected.
// ====================================================================================================================
void MainWindow::menuSearchAgain()
{
    CConsoleInput* console_input = CConsoleWindow::GetInstance()->GetInput();
    console_input->OnFindEditReturnPressed();
}

// ====================================================================================================================
QAction *addAction(QMenu *menu, const QString &text, QActionGroup *group, QSignalMapper *mapper,
                    int id)
{
    bool first = group->actions().isEmpty();
    QAction *result = menu->addAction(text);
    result->setCheckable(true);
    result->setChecked(first);
    group->addAction(result);
    QObject::connect(result, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(result, id);
    return result;
}

void MainWindow::setupDockWidgets(const QMap<QString, QSize> &customSizeHints)
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(setCorner(int)));

    QMenu *corner_menu = dockWidgetMenu->addMenu(tr("Top left corner"));
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    ::addAction(corner_menu, tr("Top dock area"), group, mapper, 0);
    ::addAction(corner_menu, tr("Left dock area"), group, mapper, 1);

    corner_menu = dockWidgetMenu->addMenu(tr("Top right corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    ::addAction(corner_menu, tr("Top dock area"), group, mapper, 2);
    ::addAction(corner_menu, tr("Right dock area"), group, mapper, 3);

    corner_menu = dockWidgetMenu->addMenu(tr("Bottom left corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    ::addAction(corner_menu, tr("Bottom dock area"), group, mapper, 4);
    ::addAction(corner_menu, tr("Left dock area"), group, mapper, 5);

    corner_menu = dockWidgetMenu->addMenu(tr("Bottom right corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    ::addAction(corner_menu, tr("Bottom dock area"), group, mapper, 6);
    ::addAction(corner_menu, tr("Right dock area"), group, mapper, 7);

    dockWidgetMenu->addSeparator();
}

void MainWindow::setCorner(int id)
{
    switch (id) {
        case 0:
            QMainWindow::setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
            break;
        case 1:
            QMainWindow::setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
            break;
        case 2:
            QMainWindow::setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
            break;
        case 3:
            QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
            break;
        case 4:
            QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
            break;
        case 5:
            QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
            break;
        case 6:
            QMainWindow::setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
            break;
        case 7:
            QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
            break;
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    CConsoleWindow::GetInstance()->NotifyOnClose();
}

#include "mainwindowMOC.cpp"