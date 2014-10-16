/****************************************************************************
** Meta object code from reading C++ file 'TinQTToolsWin.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.0.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "TinQTToolsWin.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TinQTToolsWin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CDebugToolButton_t {
    QByteArrayData data[3];
    char stringdata[35];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CDebugToolButton_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CDebugToolButton_t qt_meta_stringdata_CDebugToolButton = {
    {
QT_MOC_LITERAL(0, 0, 16),
QT_MOC_LITERAL(1, 17, 15),
QT_MOC_LITERAL(2, 33, 0)
    },
    "CDebugToolButton\0OnButtonPressed\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDebugToolButton[] = {

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

void CDebugToolButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDebugToolButton *_t = static_cast<CDebugToolButton *>(_o);
        switch (_id) {
        case 0: _t->OnButtonPressed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CDebugToolButton::staticMetaObject = {
    { &CDebugToolEntry::staticMetaObject, qt_meta_stringdata_CDebugToolButton.data,
      qt_meta_data_CDebugToolButton,  qt_static_metacall, 0, 0}
};


const QMetaObject *CDebugToolButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDebugToolButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CDebugToolButton.stringdata))
        return static_cast<void*>(const_cast< CDebugToolButton*>(this));
    return CDebugToolEntry::qt_metacast(_clname);
}

int CDebugToolButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CDebugToolEntry::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_CDebugToolSlider_t {
    QByteArrayData data[3];
    char stringdata[36];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CDebugToolSlider_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CDebugToolSlider_t qt_meta_stringdata_CDebugToolSlider = {
    {
QT_MOC_LITERAL(0, 0, 16),
QT_MOC_LITERAL(1, 17, 16),
QT_MOC_LITERAL(2, 34, 0)
    },
    "CDebugToolSlider\0OnSliderReleased\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDebugToolSlider[] = {

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

void CDebugToolSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDebugToolSlider *_t = static_cast<CDebugToolSlider *>(_o);
        switch (_id) {
        case 0: _t->OnSliderReleased(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CDebugToolSlider::staticMetaObject = {
    { &CDebugToolEntry::staticMetaObject, qt_meta_stringdata_CDebugToolSlider.data,
      qt_meta_data_CDebugToolSlider,  qt_static_metacall, 0, 0}
};


const QMetaObject *CDebugToolSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDebugToolSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CDebugToolSlider.stringdata))
        return static_cast<void*>(const_cast< CDebugToolSlider*>(this));
    return CDebugToolEntry::qt_metacast(_clname);
}

int CDebugToolSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CDebugToolEntry::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_CDebugToolTextEdit_t {
    QByteArrayData data[3];
    char stringdata[37];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CDebugToolTextEdit_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CDebugToolTextEdit_t qt_meta_stringdata_CDebugToolTextEdit = {
    {
QT_MOC_LITERAL(0, 0, 18),
QT_MOC_LITERAL(1, 19, 15),
QT_MOC_LITERAL(2, 35, 0)
    },
    "CDebugToolTextEdit\0OnReturnPressed\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDebugToolTextEdit[] = {

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

void CDebugToolTextEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDebugToolTextEdit *_t = static_cast<CDebugToolTextEdit *>(_o);
        switch (_id) {
        case 0: _t->OnReturnPressed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CDebugToolTextEdit::staticMetaObject = {
    { &CDebugToolEntry::staticMetaObject, qt_meta_stringdata_CDebugToolTextEdit.data,
      qt_meta_data_CDebugToolTextEdit,  qt_static_metacall, 0, 0}
};


const QMetaObject *CDebugToolTextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDebugToolTextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CDebugToolTextEdit.stringdata))
        return static_cast<void*>(const_cast< CDebugToolTextEdit*>(this));
    return CDebugToolEntry::qt_metacast(_clname);
}

int CDebugToolTextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CDebugToolEntry::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_CDebugToolCheckBox_t {
    QByteArrayData data[3];
    char stringdata[31];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CDebugToolCheckBox_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CDebugToolCheckBox_t qt_meta_stringdata_CDebugToolCheckBox = {
    {
QT_MOC_LITERAL(0, 0, 18),
QT_MOC_LITERAL(1, 19, 9),
QT_MOC_LITERAL(2, 29, 0)
    },
    "CDebugToolCheckBox\0OnClicked\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDebugToolCheckBox[] = {

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

void CDebugToolCheckBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDebugToolCheckBox *_t = static_cast<CDebugToolCheckBox *>(_o);
        switch (_id) {
        case 0: _t->OnClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CDebugToolCheckBox::staticMetaObject = {
    { &CDebugToolEntry::staticMetaObject, qt_meta_stringdata_CDebugToolCheckBox.data,
      qt_meta_data_CDebugToolCheckBox,  qt_static_metacall, 0, 0}
};


const QMetaObject *CDebugToolCheckBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDebugToolCheckBox::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CDebugToolCheckBox.stringdata))
        return static_cast<void*>(const_cast< CDebugToolCheckBox*>(this));
    return CDebugToolEntry::qt_metacast(_clname);
}

int CDebugToolCheckBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CDebugToolEntry::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_CDebugToolsWin_t {
    QByteArrayData data[1];
    char stringdata[16];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_CDebugToolsWin_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_CDebugToolsWin_t qt_meta_stringdata_CDebugToolsWin = {
    {
QT_MOC_LITERAL(0, 0, 14)
    },
    "CDebugToolsWin\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDebugToolsWin[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void CDebugToolsWin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject CDebugToolsWin::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CDebugToolsWin.data,
      qt_meta_data_CDebugToolsWin,  qt_static_metacall, 0, 0}
};


const QMetaObject *CDebugToolsWin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDebugToolsWin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CDebugToolsWin.stringdata))
        return static_cast<void*>(const_cast< CDebugToolsWin*>(this));
    return QWidget::qt_metacast(_clname);
}

int CDebugToolsWin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
