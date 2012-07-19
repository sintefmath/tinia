
FILE( GLOB fpsviewer_HEADERS "*.hpp" )
FILE( GLOB fpsviewer_JS_SOURCES "*.js" )
FILE(GLOB fpsviewer_RESOURCES "*.qrc" )
QT4_ADD_RESOURCES( fpsviewer_QRC_SOURCES ${fpsviewer_RESOURCES} )

IF(${Tinia_DESKTOP})

ADD_EXECUTABLE(fpsviewer_desktop
    ${fpsviewer_HEADERS}
    fpsviewer_desktop.cpp
    ${fpsviewer_QRC_SOURCES}
    ${fpsviewer_JS_SOURCES}
)
  TARGET_LINK_LIBRARIES(fpsviewer_desktop
    qtcontroller
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
    model
    jobcontroller
    ${Boost_LIBRARIES}
    )
ENDIF()


IF(${Tinia_SERVER})
  ADD_EXECUTABLE(fpsviewer_web
    ${tutorial_HEADERS}
    fpsviewer_web.cpp
    ${fpsviewer_QRC_RESOURCES}
    )

  TARGET_LINK_LIBRARIES(fpsviewer_web
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Freeglut_LIBRARY}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    model 
    modelxml
    renderlist
    renderlistgl
    jobcontroller
    trell
    )


  INSTALL(TARGETS
    fpsviewer_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
