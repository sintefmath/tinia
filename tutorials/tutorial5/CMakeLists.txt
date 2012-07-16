
SET( tutorial_HEADERS "Tutorial5_Job.hpp" )

IF(${DESKTOP})
ADD_EXECUTABLE(tutorial5_desktop
    ${tutorial_HEADERS}
    tutorial5_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial5_desktop
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
  ADD_EXECUTABLE(tutorial5_web
    ${tutorial_HEADERS}
    tutorial5_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial5_web
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
    tutorial5_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
