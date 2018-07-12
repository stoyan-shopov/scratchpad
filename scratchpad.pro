#-------------------------------------------------
#
# Project created by QtCreator 2018-06-11T14:57:13
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scratchpad
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS CORE_CELLS_COUNT=131072 STACK_DEPTH=16

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        scratchpad.cxx \
    main.cxx \
    sforth.cxx \
    sforth/dictionary.c \
    sforth/engine.c \
    sforth/sf-opt-prog-tools.c \
    sforth/sf-opt-string.c \
    scratchpad-widget.cxx

HEADERS += \
        scratchpad.hxx \
    sforth.hxx \
    scratchpad-widget.hxx

FORMS += \
        scratchpad.ui \
    scratchpadwidget.ui

INCLUDEPATH += ./sforth/

RESOURCES += \
    resources.qrc

DISTFILES += \
    stylesheet.txt
