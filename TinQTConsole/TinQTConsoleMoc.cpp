/****************************************************************************
** Meta object code from reading C++ file 'TinQTConsole.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.0.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "TinQTConsole.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TinQTConsole.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CConsoleInput_t {
    QByteArrayData data[7];
    char stringdata[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CConsoleInput_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CConsoleInput_t qt_meta_stringdata_CConsoleInput = {
    {
QT_MOC_LITERAL(0, 0, 13),
QT_MOC_LITERAL(1, 14, 15),
QT_MOC_LITERAL(2, 30, 0),
QT_MOC_LITERAL(3, 31, 23),
QT_MOC_LITERAL(4, 55, 18),
QT_MOC_LITERAL(5, 74, 19),
QT_MOC_LITERAL(6, 94, 20)
    },
    "CConsoleInput\0OnReturnPressed\0\0"
    "OnFileEditReturnPressed\0OnButtonRunPressed\0"
    "OnButtonStepPressed\0OnButtonPausePressed\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CConsoleInput[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a,
       3,    0,   40,    2, 0x0a,
       4,    0,   41,    2, 0x0a,
       5,    0,   42,    2, 0x0a,
       6,    0,   43,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CConsoleInput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CConsoleInput *_t = static_cast<CConsoleInput *>(_o);
        switch (_id) {
        case 0: _t->OnReturnPressed(); break;
        case 1: _t->OnFileEditReturnPressed(); break;
        case 2: _t->OnButtonRunPressed(); break;
        case 3: _t->OnButtonStepPressed(); break;
        case 4: _t->OnButtonPausePressed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CConsoleInput::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_CConsoleInput.data,
      qt_meta_data_CConsoleInput,  qt_static_metacall, 0, 0}
};


const QMetaObject *CConsoleInput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CConsoleInput::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CConsoleInput.stringdata))
        return static_cast<void*>(const_cast< CConsoleInput*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int CConsoleInput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_CConsoleOutput_t {
    QByteArrayData data[3];
    char stringdata[24];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CConsoleOutput_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CConsoleOutput_t qt_meta_stringdata_CConsoleOutput = {
    {
QT_MOC_LITERAL(0, 0, 14),
QT_MOC_LITERAL(1, 15, 6),
QT_MOC_LITERAL(2, 22, 0)
    },
    "CConsoleOutput\0Update\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CConsoleOutput[] = {

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

void CConsoleOutput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CConsoleOutput *_t = static_cast<CConsoleOutput *>(_o);
        switch (_id) {
        case 0: _t->Update(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CConsoleOutput::staticMetaObject = {
    { &QListWidget::staticMetaObject, qt_meta_stringdata_CConsoleOutput.data,
      qt_meta_data_CConsoleOutput,  qt_static_metacall, 0, 0}
};


const QMetaObject *CConsoleOutput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CConsoleOutput::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CConsoleOutput.stringdata))
        return static_cast<void*>(const_cast< CConsoleOutput*>(this));
    return QListWidget::qt_metacast(_clname);
}

int CConsoleOutput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListWidget::qt_metacall(_c, _id, _a);
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
