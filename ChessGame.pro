#-------------------------------------------------
#
# Project created by QtCreator 2015-11-20T15:37:06
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChessGame
#TARGET = Console_ChessGame
TEMPLATE = app

CONFIG += c++11

#CONFIG   += console
#CONFIG   -= app_bundle


CONFIG += mobility
MOBILITY = 

RESOURCES += \
    images.qrc

#linux build
#RC_ICONS+=/home/kosmaz/Software-Development/Qt_Projects/projects/Gui_ChessGame/images/ICONS/ChessBoard.ico
#windows build
RC_ICONS+=C:\users\user\Documents\Qt\resources\ChessBoard.ico
RC_LANG+=English(US/UK)
RC_CODEPAGE+=****
VERSION+=0.1.0.0
QMAKE_TARGET_COMPANY+=-k0$m@3- Inc.
QMAKE_TARGET_DESCRIPTION+=CHESS GAME
QMAKE_TARGET_COPYRIGHT+=Licensed under the GPL License
QMAKE_TARGET_PRODUCT+=CHESS GAME

HEADERS += \
    src/chessBoard.hpp \
    src/defs.hpp \
    src/GuiDisplay.hpp \
    src/Human.hpp \
    src/Lauretta.hpp \
    src/Player.hpp \
    src/ui_GuiDisplay.h

SOURCES += \
    src/chessBoard.cpp \
    src/GuiDisplay.cpp \
    src/Human.cpp \
    src/Lauretta.cpp \
    src/main.cpp \
    src/Player.cpp

