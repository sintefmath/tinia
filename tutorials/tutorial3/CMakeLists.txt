
SET( tutorial_HEADERS "Tutorial3_Job.hpp" )

IF(Tinia_DESKTOP)
ADD_EXECUTABLE(tutorial3_desktop
    ${tutorial_HEADERS}
    tutorial3_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial3_desktop
      model
    qtcontroller
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
    jobcontroller
    ${Boost_LIBRARIES}
    )
ENDIF()


IF(Tinia_SERVER)
  ADD_EXECUTABLE(tutorial3_web
    ${tutorial_HEADERS}
    tutorial3_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial3_web
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
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
