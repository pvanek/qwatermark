TEMPLATE = app
TARGET = QWatermark 

QT        += core gui 

HEADERS   += src/qwatermark.h \
    src/profiledialog.h \
    src/profile.h
SOURCES   += src/main.cpp \
    src/qwatermark.cpp \
    src/profiledialog.cpp \
    src/profile.cpp
FORMS     += src/qwatermark.ui \     
    src/profiledialog.ui
RESOURCES += \
    src/resources.qrc

mac {
    # Copy the custom Info.plist to the app bundle
    QMAKE_INFO_PLIST = src/Info.plist

    # Icon is mandatory for submission
    ICON = src/qwatermark.icns
}
