

# -----------------------------------------
# Project target (executable name)
# -----------------------------------------
TARGET = PROJETVOLANGTBL

# -----------------------------------------
# Qt modules
# -----------------------------------------
QT += widgets opengl core gui 

# -----------------------------------------
# Compiler configuration
# -----------------------------------------
CONFIG += c++17

# -----------------------------------------
# Source files
# -----------------------------------------
SOURCES += \
    main.cpp \
    PROJETVOLANGTBL.cpp \
    map.cpp
# -----------------------------------------
# Header files
# -----------------------------------------
HEADERS += \
    PROJETVOLANGTBL.h \
    map.h
FORMS += PROJETVOLANGTBL.ui
# -----------------------------------------
# Optional: specify the Visual Studio version
# -----------------------------------------
# When you run qmake, you can use -tp vc
# Example: qmake -tp vc projetvolangtbl.pro
# This will generate projetvolangtbl.vcxproj automatically
