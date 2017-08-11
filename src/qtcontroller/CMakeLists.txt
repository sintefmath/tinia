FILE(GLOB_RECURSE qtcontroller_HEADERS "../../include/tinia/qtcontroller/*.h" "../../include/tinia/qtcontroller/*.hpp")
FILE(GLOB_RECURSE qtcontroller_SOURCES "*.c" "*.cpp")

set(GLEW_INCLUDE_DIR  "C:/opt/glew-2.0.0/include" CACHE PATH "")
set(GLEW_LIBRARY  "C:/opt/glew-2.0.0/lib/Release/x64/glew32s.lib" CACHE PATH "")
FILE(GLOB qtcontroller_RESOURCES "*.qrc")
FILE(GLOB qtcontroller_FORMS "*.ui")
FILE(GLOB qtcontroller_SOURCES_TO_BE_MOCED
    "../../include/tinia/qtcontroller/moc/*.hpp"
    "../../include/tinia/qtcontroller/moc/*.h"
    "../../include/tinia/qtcontroller/scripting/*.hpp")
set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/5.8/msvc2015_64/lib/cmake" )
#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/5.6/msvc2015_64/lib/cmake/Qt5OpenGL" CACHE PATH "")
find_package( Qt5OpenGL REQUIRED )
find_package( Qt5Script REQUIRED )
find_package( Qt5Xml REQUIRED )
set(GLEW_DEFINITIONS -DGLEW_STATIC)
find_package( GLEW REQUIRED )
IF(NOT LIBXML2_FOUND)
    FILE(GLOB_RECURSE serverToRemove "*Server*" "*Server*" "*LongPoll*" "../../include/tinia/qtcontroller/moc/*Server*"
									"../../include/tinia/qtcontroller/moc/*LongPoll*")
    LIST(REMOVE_ITEM qtcontroller_SOURCES ${serverToRemove})
    LIST(REMOVE_ITEM qtcontroller_SOURCES_TO_BE_MOCED ${serverToRemove})
    LIST(REMOVE_ITEM qtcontroller_HEADERS ${serverToRemove})

ENDIF()
QT5_ADD_RESOURCES( qtcontroller_QRC_SOURCES ${qtcontroller_RESOURCES} )
QT5_WRAP_CPP(qtcontroller_HEADERS_MOC ${qtcontroller_SOURCES_TO_BE_MOCED})# ${qtcontroller_SOURCES})
QT5_WRAP_UI(qtcontroller_FORMS_HEADERS ${qtcontroller_FORMS})

 add_library( tinia_qtcontroller
    ${qtcontroller_QRC_SOURCES}
    ${qtcontroller_HEADERS}
    ${qtcontroller_SOURCES}
    ${qtcontroller_HEADERS_MOC}
    #${qtcontroller_SOURCES_TO_BE_MOCED} # Was removed on Win32
    ${qtcontroller_FORMS_HEADERS}
    )
qt5_use_modules (tinia_qtcontroller Widgets)
IF(LIBXML2_FOUND)
target_link_libraries ( tinia_qtcontroller
    tinia_renderlist
    tinia_renderlistgl
    tinia_model
	tinia_modelxml
    tinia_javascript
    ${LIBXML2_LIBRARIES}
    tinia_modelxml
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    tinia_jobcontroller
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${Freeglut_LIBRARY}
    ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Script_LIBRARIES} ${Qt5Xml_LIBRARIES}
)
ELSe()
target_link_libraries ( tinia_qtcontroller
    tinia_renderlist
    tinia_renderlistgl
    tinia_model
    tinia_javascript
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    tinia_jobcontroller
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${Freeglut_LIBRARY}
    ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Script_LIBRARIES} ${Qt5Xml_LIBRARIES}
)
ENDIF()
    target_include_directories( tinia_qtcontroller PUBLIC
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5GUI_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5OpenGL_INCLUDE_DIRS}
        ${Qt5Script_INCLUDE_DIRS}
        ${Qt5ScriptTools_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
    )
# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( DIRECTORY "../../include/tinia/qtcontroller" # ${qtcontroller_HEADERS}
    DESTINATION include/tinia  COMPONENT qtcontroller_dev)

  install( FILES ${qtcontroller_HEADERS_MOC} ${qtcontroller_SOURCES_TO_BE_MOCED}
    DESTINATION include/tinia/qtcontroller/moc  COMPONENT qtcontroller_dev)

  install( TARGETS 
    tinia_qtcontroller
    EXPORT TiniaTargets
    LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT qtcontroller)
