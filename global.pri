INCLUDEPATH     += ../../include

MOC_DIR          = .moc
OBJECTS_DIR      = .obj

QT              += widgets network webkitwidgets

*-g++* {
    QMAKE_CXXFLAGS += -std=c++11
}
