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
INCLUDEPATH +=$$PWD/DarkSub


QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = detectorMPI
TEMPLATE = app


SOURCES +=\
    Common/mpicalcrunner.cpp \
    Common/mpimesgrecvr.cpp \
    DarkSub/main_mpi.cpp \
    Common/mpiengine.cpp \
    DarkSub/imagequeueitem.cpp \
    DarkSub/signalmessage.cpp \
    Common/mpiScatter.cpp \
    DarkSub/mpicontrolgui.cpp \
    Common/mpiGather.cpp \
    Common/pipeReader.cpp \
    DarkSub/mpidarksubtract.cpp \
    Common/pipewriter.cpp \
    Common/tinytiff.cpp \
    DarkSub/mpiscatterdark.cpp \
    Common/pipebinaryformat.cpp \
    DarkSub/mpigatherdark.cpp \
    Common/imageStreamTest.cpp

HEADERS  += \
    Common/mpicalcrunner.h \
    Common/mpimesgrecvr.h \
    Common/mpiengine.h \
    DarkSub/imagequeueitem.h \
    DarkSub/signalmessage.h \
    Common/mpiScatter.h \
    DarkSub/mpicontrolgui.h \
    Common/mpiGather.h \
    Common/pipeReader.h \
    DarkSub/mpidarksubtract.h \
    Common/pipewriter.h \
    Common/tinytiff.h \
    DarkSub/mpiscatterdark.h \
    Common/pipebinaryformat.h \
    DarkSub/mpigatherdark.h

FORMS    += \
    DarkSub/mpicontrolgui.ui







