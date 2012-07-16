
SET( tutorial_HEADERS "Tutorial3_Job.hpp" )

IF(${DESKTOP})
ADD_EXECUTABLE(tutorial3_desktop
    ${tutorial_HEADERS}
    tutorial3_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial3_desktop
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
  ADD_EXECUTABLE(tutorial3_web
    ${tutorial_HEADERS}
    tutorial3_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial3_web
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
    tutorial3_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
