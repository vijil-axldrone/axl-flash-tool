/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/mainwindow.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[34];
    char stringdata0[11];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[13];
    char stringdata5[9];
    char stringdata6[9];
    char stringdata7[10];
    char stringdata8[16];
    char stringdata9[10];
    char stringdata10[8];
    char stringdata11[20];
    char stringdata12[19];
    char stringdata13[22];
    char stringdata14[9];
    char stringdata15[21];
    char stringdata16[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 15),  // "selectDirectory"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 12),  // "confirmSetup"
        QT_MOC_LITERAL(41, 12),  // "refreshPorts"
        QT_MOC_LITERAL(54, 8),  // "flashEsp"
        QT_MOC_LITERAL(63, 8),  // "flashStm"
        QT_MOC_LITERAL(72, 9),  // "updateOta"
        QT_MOC_LITERAL(82, 15),  // "cancelOperation"
        QT_MOC_LITERAL(98, 9),  // "appendLog"
        QT_MOC_LITERAL(108, 7),  // "message"
        QT_MOC_LITERAL(116, 19),  // "handleProcessOutput"
        QT_MOC_LITERAL(136, 18),  // "handleProcessError"
        QT_MOC_LITERAL(155, 21),  // "handleProcessFinished"
        QT_MOC_LITERAL(177, 8),  // "exitCode"
        QT_MOC_LITERAL(186, 20),  // "QProcess::ExitStatus"
        QT_MOC_LITERAL(207, 10)   // "exitStatus"
    },
    "MainWindow",
    "selectDirectory",
    "",
    "confirmSetup",
    "refreshPorts",
    "flashEsp",
    "flashStm",
    "updateOta",
    "cancelOperation",
    "appendLog",
    "message",
    "handleProcessOutput",
    "handleProcessError",
    "handleProcessFinished",
    "exitCode",
    "QProcess::ExitStatus",
    "exitStatus"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x08,    1 /* Private */,
       3,    0,   81,    2, 0x08,    2 /* Private */,
       4,    0,   82,    2, 0x08,    3 /* Private */,
       5,    0,   83,    2, 0x08,    4 /* Private */,
       6,    0,   84,    2, 0x08,    5 /* Private */,
       7,    0,   85,    2, 0x08,    6 /* Private */,
       8,    0,   86,    2, 0x08,    7 /* Private */,
       9,    1,   87,    2, 0x08,    8 /* Private */,
      11,    0,   90,    2, 0x08,   10 /* Private */,
      12,    0,   91,    2, 0x08,   11 /* Private */,
      13,    2,   92,    2, 0x08,   12 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 15,   14,   16,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'selectDirectory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'confirmSetup'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'flashEsp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'flashStm'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateOta'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cancelOperation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'appendLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleProcessOutput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleProcessError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleProcessFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ExitStatus, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->selectDirectory(); break;
        case 1: _t->confirmSetup(); break;
        case 2: _t->refreshPorts(); break;
        case 3: _t->flashEsp(); break;
        case 4: _t->flashStm(); break;
        case 5: _t->updateOta(); break;
        case 6: _t->cancelOperation(); break;
        case 7: _t->appendLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->handleProcessOutput(); break;
        case 9: _t->handleProcessError(); break;
        case 10: _t->handleProcessFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
