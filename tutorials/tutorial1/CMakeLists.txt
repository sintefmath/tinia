
SET( tutorial1_HEADERS "Tutorial1_Job.hpp" )

IF(${DESKTOP})
ADD_EXECUTABLE(tutorial1_desktop
    ${tutorial_HEADERS}
    tutorial1_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial1_desktop
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
  ADD_EXECUTABLE(tutorial1_web
    ${tutorial_HEADERS}
    tutorial1_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial1_web
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
    tutorial1_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
