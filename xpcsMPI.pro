#-------------------------------------------------
#
# Project created by QtCreator 2011-12-08T16:30:52
#
#-------------------------------------------------
QMAKE_CXX = mpic++
QMAKE_CXX_RELEASE = $$QMAKE_CXX
QMAKE_CXX_DEBUG = $$QMAKE_CXX
QMAKE_LINK = $$QMAKE_CXX
QMAKE_CC = mpicc
 
QMAKE_CFLAGS += $$system(mpicc --showme:compile)
QMAKE_LFLAGS += $$system(mpicxx --showme:link)
QMAKE_CXXFLAGS += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK -DUSE_MPI
QMAKE_CXXFLAGS_RELEASE += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK


INCLUDEPATH +=$$PWD/Common
INCLUDEPATH +=$$PWD/XPCS



QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xpcsMPI
TEMPLATE = app


SOURCES +=\
    Common/mpicalcrunner.cpp \
    Common/mpimesgrecvr.cpp \
    Common/mpiengine.cpp \
    XPCS/imagequeueitem.cpp \
    XPCS/signalmessage.cpp \
    Common/mpiScatter.cpp \
    Common/mpiGather.cpp \
    Common/pipeReader.cpp \
    Common/pipewriter.cpp \
    Common/tinytiff.cpp \
    Common/pipebinaryformat.cpp \
    Common/imageStreamTest.cpp \
    XPCS/xpcsscatter.cpp \
    XPCS/xpcsgather.cpp \
    XPCS/mpixpcs.cpp \
    XPCS/main_xpcs.cpp \
    XPCS/imm.cpp \
    XPCS/xpcsgui.cpp

HEADERS  += \
    Common/mpicalcrunner.h \
    Common/mpimesgrecvr.h \
    Common/mpiengine.h \
    XPCS/imagequeueitem.h \
    XPCS/signalmessage.h \
    Common/mpiScatter.h \
    Common/mpiGather.h \
    Common/pipeReader.h \
    Common/pipewriter.h \
    Common/tinytiff.h \
    Common/pipebinaryformat.h \
    XPCS/mpixpcs.h \
    XPCS/xpcsgather.h \
    XPCS/xpcsscatter.h \
    XPCS/imm.h \
    XPCS/imm_header.h \
    XPCS/xpcsgui.h

FORMS    += \
    XPCS/xpcsgui.ui







