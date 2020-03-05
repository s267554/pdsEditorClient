QT += widgets network
requires(qtConfig(filedialog), qtConfig(combobox))
qtHaveModule(printsupport): QT += printsupport

TEMPLATE        = app
TARGET          = textedit

HEADERS         = textedit.h \
    client.h
SOURCES         = textedit.cpp \
                  client.cpp \
                  main.cpp

RESOURCES += textedit.qrc
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
