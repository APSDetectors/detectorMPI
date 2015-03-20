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

TARGET = xpcsMPI
TEMPLATE = app


SOURCES +=\
    mpicalcrunner.cpp \
    mpimesgrecvr.cpp \
    mpiengine.cpp \
    imagequeueitem.cpp \
    signalmessage.cpp \
    mpiScatter.cpp \
    mpiGather.cpp \
    pipeReader.cpp \
    pipewriter.cpp \
    tinytiff.cpp \
    pipebinaryformat.cpp \
    imageStreamTest.cpp \
    xpcsscatter.cpp \
    xpcsgather.cpp \
    mpixpcs.cpp \
    main_xpcs.cpp \
    imm.cpp \
    xpcsgui.cpp

HEADERS  += \
    mpicalcrunner.h \
    mpimesgrecvr.h \
    mpiengine.h \
    imagequeueitem.h \
    signalmessage.h \
    mpiScatter.h \
    mpiGather.h \
    pipeReader.h \
    pipewriter.h \
    tinytiff.h \
    pipebinaryformat.h \
    mpixpcs.h \
    xpcsgather.h \
    xpcsscatter.h \
    imm.h \
    imm_header.h \
    xpcsgui.h

FORMS    += \
    xpcsgui.ui







