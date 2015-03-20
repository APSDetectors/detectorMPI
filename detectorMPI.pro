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




QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = detectorMPI
TEMPLATE = app


SOURCES +=\
    mpicalcrunner.cpp \
    mpimesgrecvr.cpp \
    main_mpi.cpp \
    mpiengine.cpp \
    imagequeueitem.cpp \
    signalmessage.cpp \
    mpiScatter.cpp \
    mpicontrolgui.cpp \
    mpiGather.cpp \
    pipeReader.cpp \
    mpidarksubtract.cpp \
    pipewriter.cpp \
    tinytiff.cpp \
    mpiscatterdark.cpp \
    pipebinaryformat.cpp \
    imageStreamTest.cpp \
    mpigatherdark.cpp

HEADERS  += \
    mpicalcrunner.h \
    mpimesgrecvr.h \
    mpiengine.h \
    imagequeueitem.h \
    signalmessage.h \
    mpiScatter.h \
    mpicontrolgui.h \
    mpiGather.h \
    pipeReader.h \
    mpidarksubtract.h \
    pipewriter.h \
    tinytiff.h \
    mpiscatterdark.h \
    pipebinaryformat.h \
    mpigatherdark.h

FORMS    += \
    mpicontrolgui.ui







