

# -----------------------------------------
# Project target (executable name)
# -----------------------------------------
TARGET = PROJETVOLANGTBL

# -----------------------------------------
# Qt modules
# -----------------------------------------
QT += widgets openglwidgets core gui 
QT += 3dcore 3drender 3dextras 3dinput 
# -----------------------------------------
# Compiler configuration
# -----------------------------------------
CONFIG += c++17

# -----------------------------------------
# Source files
# -----------------------------------------
SOURCES += \
    main.cpp \
    mainWindow.cpp \
    PROJETVOLANGTBL.cpp \
    map2d.cpp \
    Track.cpp \
    trackBuilder3D.cpp \
    trackViewer.cpp \
    trackPieces3D.cpp \
    track3DViewer.cpp

# -----------------------------------------
# Header files
# -----------------------------------------
HEADERS += \
    PROJETVOLANGTBL.h \
    mainWindow.h \
    map2d.h \
    Track.h \
    trackBuilder3D.h \
    trackViewer.h \
    trackPieces3D.h \
    track3DViewer.h

FORMS += PROJETVOLANGTBL.ui
# -----------------------------------------
# Optional: specify the Visual Studio version
# -----------------------------------------
# When you run qmake, you can use -tp vc
# Example: qmake -tp vc projetvolangtbl.pro
# This will generate projetvolangtbl.vcxproj automatically
