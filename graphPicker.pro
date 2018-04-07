HEADERS       = GraphPicker.h
SOURCES       = GraphPicker.cc \
                main.cc

RESOURCES     = GraphPicker.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/imageviewer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS imageviewer.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/widgets/imageviewer
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)

wince*: {
   DEPLOYMENT_PLUGIN += qjpeg qmng qgif
}
