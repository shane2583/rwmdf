# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = rwmdf
DESTDIR = ../x64/Debug
QT += core
CONFIG += debug console
DEFINES += QT_DLL _CONSOLE
#DEFINES -= WIN32 WIN64
INCLUDEPATH += . \
    ./../asam_mdf4_lib \
    ./GeneratedFiles/Debug
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/debug
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(rwmdf.pri)


#contains(DEFINES, WIN32){
#        message(hello WIN32)
#}