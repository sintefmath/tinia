FILE(GLOB_RECURSE qtcontroller_HEADERS "../../include/tinia/qtcontroller/*.h" "../../include/tinia/qtcontroller/*.hpp")
FILE(GLOB_RECURSE qtcontroller_SOURCES "*.c" "*.cpp")


FILE(GLOB qtcontroller_RESOURCES "*.qrc")
FILE(GLOB qtcontroller_FORMS "*.ui")
FILE(GLOB qtcontroller_SOURCES_TO_BE_MOCED
    "../../include/tinia/qtcontroller/moc/*.hpp"
    "../../include/tinia/qtcontroller/moc/*.h"
    "../../include/tinia/qtcontroller/scripting/*.hpp")

IF(NOT ${LIBXML2_FOUND})
    FILE(GLOB_RECURSE serverToRemove "*Server*" "*Server*" "*LongPoll*")
    LIST(REMOVE_ITEM qtcontroller_SOURCES ${serverToRemove})
    LIST(REMOVE_ITEM qtcontroller_SOURCES_TO_BE_MOCED ${serverToRemove})
    LIST(REMOVE_ITEM qtcontroller_HEADERS ${serverToRemove})

ENDIF()
QT4_ADD_RESOURCES( qtcontroller_QRC_SOURCES ${qtcontroller_RESOURCES} )
QT4_WRAP_CPP(qtcontroller_HEADERS_MOC ${qtcontroller_SOURCES_TO_BE_MOCED} ${qtcontroller_SOURCES})
QT4_WRAP_UI(qtcontroller_FORMS_HEADERS ${qtcontroller_FORMS})

 add_library( qtcontroller
    ${qtcontroller_QRC_SOURCES}
    ${qtcontroller_HEADERS}
    ${qtcontroller_SOURCES}
    ${qtcontroller_HEADERS_MOC}
    ${qtcontroller_SOURCES_TO_BE_MOCED} # Was removed on Win32
    ${qtcontroller_FORMS_HEADERS}
    )

IF(${LIBXML2_FOUND})
target_link_libraries ( qtcontroller
    renderlist
    renderlistgl
    model
	modelxml
    tinia_javascript
    ${LIBXML2_LIBRARIES}
    modelxml
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    jobcontroller
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
)
ELSe()
target_link_libraries ( qtcontroller
    renderlist
    renderlistgl
    model
    tinia_javascript
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    jobcontroller
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
)
ENDIF()
# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( FILES ${qtcontroller_HEADERS}
    DESTINATION include/tinia/qtcontroller  COMPONENT qtcontroller_dev)

  install( FILES ${qtcontroller_HEADERS_MOC} ${qtcontroller_SOURCES_TO_BE_MOCED}
    DESTINATION include/tinia/qtcontroller/moc  COMPONENT qtcontroller_dev)

  install( TARGETS 
    qtcontroller LIBRARY 
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT qtcontroller)
