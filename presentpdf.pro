TEMPLATE = app
TARGET = presentpdf
DEPENDPATH += .
INCLUDEPATH += .
LIBS+= 
QMAKE_CXXFLAGS += -std=c++14
QT += widgets

# Input
HEADERS +=				\
	ScreenInfo.hpp			\
	Renderer.hpp			\
	Presenter.hpp			\
	View.hpp
SOURCES +=				\
	main.cpp			\
	ScreenInfo.cpp			\
	Renderer.cpp			\
	Presenter.cpp			\
	View.cpp			\
	Scribble.cpp
LIBS += -lpoppler-qt5

# Output directories
MOC_DIR=bin
UI_DIR=bin
RCC_DIR=bin
OBJECTS_DIR=bin
DESTDIR=bin

