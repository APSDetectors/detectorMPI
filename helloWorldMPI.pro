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
INCLUDEPATH +=$$PWD/HelloWorld


QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = helloWorldMPI
TEMPLATE = app


SOURCES +=\
    Common/mpicalcrunner.cpp \
    Common/mpimesgrecvr.cpp \
    HelloWorld/main_mpi.cpp \
    Common/mpiengine.cpp \
    HelloWorld/imagequeueitem.cpp \
    HelloWorld/signalmessage.cpp \
    Common/mpiScatter.cpp \
    HelloWorld/mpicontrolgui.cpp \
    Common/mpiGather.cpp \
    Common/pipeReader.cpp \
    HelloWorld/mpihello.cpp \
    Common/pipewriter.cpp \
    Common/tinytiff.cpp \
    HelloWorld/mpiscatterhello.cpp \
    Common/pipebinaryformat.cpp \
    HelloWorld/mpigatherhello.cpp \
    Common/imageStreamTest.cpp

HEADERS  += \
    Common/mpicalcrunner.h \
    Common/mpimesgrecvr.h \
    Common/mpiengine.h \
    HelloWorld/imagequeueitem.h \
    HelloWorld/signalmessage.h \
    Common/mpiScatter.h \
    HelloWorld/mpicontrolgui.h \
    Common/mpiGather.h \
    Common/pipeReader.h \
    HelloWorld/mpihello.h \
    Common/pipewriter.h \
    Common/tinytiff.h \
    HelloWorld/mpiscatterhello.h \
    Common/pipebinaryformat.h \
    HelloWorld/mpigatherhello.h \
    HelloWorld/mpiscatterhello.h \
    HelloWorld/mpigatherhello.h

FORMS    += \
    HelloWorld/mpicontrolgui.ui







