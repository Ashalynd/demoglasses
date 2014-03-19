#http://www.cmake.org/cmake/help/cmake2.6docs.html
cmake_minimum_required (VERSION 2.6)
project (demoglasses)

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED )

SET(demoglasses_SOURCES main.cpp mainwindow.cpp detector.cpp)
SET(demoglasses_MOC_HEADERS mainwindow.h)

QT4_WRAP_CPP(demoglasses_MOC_SOURCES ${demoglasses_MOC_HEADERS})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_EXECUTABLE(demoglasses ${demoglasses_SOURCES} ${demoglasses_MOC_SOURCES})

TARGET_LINK_LIBRARIES(demoglasses ${OpenCV_LIBS} ${QT_LIBRARIES} )
