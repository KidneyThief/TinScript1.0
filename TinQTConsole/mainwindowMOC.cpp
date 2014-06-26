/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.0.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata[263];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MainWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10),
QT_MOC_LITERAL(1, 11, 14),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 14),
QT_MOC_LITERAL(4, 42, 14),
QT_MOC_LITERAL(5, 57, 14),
QT_MOC_LITERAL(6, 72, 17),
QT_MOC_LITERAL(7, 90, 9),
QT_MOC_LITERAL(8, 100, 2),
QT_MOC_LITERAL(9, 103, 14),
QT_MOC_LITERAL(10, 118, 13),
QT_MOC_LITERAL(11, 132, 12),
QT_MOC_LITERAL(12, 145, 17),
QT_MOC_LITERAL(13, 163, 15),
QT_MOC_LITERAL(14, 179, 16),
QT_MOC_LITERAL(15, 196, 14),
QT_MOC_LITERAL(16, 211, 20),
QT_MOC_LITERAL(17, 232, 8),
QT_MOC_LITERAL(18, 241, 20)
    },
    "MainWindow\0menuSaveLayout\0\0menuLoadLayout\0"
    "autoSaveLayout\0autoLoadLayout\0"
    "defaultLoadLayout\0setCorner\0id\0"
    "setDockOptions\0menuDebugStop\0menuDebugRun\0"
    "menuDebugStepOver\0menuDebugStepIn\0"
    "menuDebugStepOut\0menuOpenScript\0"
    "menuOpenScriptAction\0QAction*\0"
    "menuAddVariableWatch\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x0a,
       3,    0,   90,    2, 0x0a,
       4,    0,   91,    2, 0x0a,
       5,    0,   92,    2, 0x0a,
       6,    0,   93,    2, 0x0a,
       7,    1,   94,    2, 0x0a,
       9,    0,   97,    2, 0x0a,
      10,    0,   98,    2, 0x0a,
      11,    0,   99,    2, 0x0a,
      12,    0,  100,    2, 0x0a,
      13,    0,  101,    2, 0x0a,
      14,    0,  102,    2, 0x0a,
      15,    0,  103,    2, 0x0a,
      16,    1,  104,    2, 0x0a,
      18,    0,  107,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 17,    2,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->menuSaveLayout(); break;
        case 1: _t->menuLoadLayout(); break;
        case 2: _t->autoSaveLayout(); break;
        case 3: _t->autoLoadLayout(); break;
        case 4: _t->defaultLoadLayout(); break;
        case 5: _t->setCorner((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->setDockOptions(); break;
        case 7: _t->menuDebugStop(); break;
        case 8: _t->menuDebugRun(); break;
        case 9: _t->menuDebugStepOver(); break;
        case 10: _t->menuDebugStepIn(); break;
        case 11: _t->menuDebugStepOut(); break;
        case 12: _t->menuOpenScript(); break;
        case 13: _t->menuOpenScriptAction((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 14: _t->menuAddVariableWatch(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, 0, 0}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
    return _id;
}
struct qt_meta_stringdata_CScriptOpenWidget_t {
    QByteArrayData data[3];
    char stringdata[41];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CScriptOpenWidget_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CScriptOpenWidget_t qt_meta_stringdata_CScriptOpenWidget = {
    {
QT_MOC_LITERAL(0, 0, 17),
QT_MOC_LITERAL(1, 18, 20),
QT_MOC_LITERAL(2, 39, 0)
    },
    "CScriptOpenWidget\0menuOpenScriptAction\0"
    "\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CScriptOpenWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CScriptOpenWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CScriptOpenWidget *_t = static_cast<CScriptOpenWidget *>(_o);
        switch (_id) {
        case 0: _t->menuOpenScriptAction(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CScriptOpenWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CScriptOpenWidget.data,
      qt_meta_data_CScriptOpenWidget,  qt_static_metacall, 0, 0}
};


const QMetaObject *CScriptOpenWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CScriptOpenWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CScriptOpenWidget.stringdata))
        return static_cast<void*>(const_cast< CScriptOpenWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int CScriptOpenWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
