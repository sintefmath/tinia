
SET( tutorial_HEADERS "Tutorial2_Job.hpp" )

IF(${DESKTOP})
ADD_EXECUTABLE(tutorial2_desktop
    ${tutorial_HEADERS}
    tutorial2_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial2_desktop
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


IF(${SERVER})
  ADD_EXECUTABLE(tutorial2_web
    ${tutorial_HEADERS}
    tutorial2_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial2_web
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
    tutorial2_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
