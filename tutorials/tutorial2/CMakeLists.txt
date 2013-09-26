
SET( tutorial_HEADERS "Tutorial2_Job.hpp" )

IF(Tinia_DESKTOP)
ADD_EXECUTABLE(tutorial2_desktop
    ${tutorial_HEADERS}
    tutorial2_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial2_desktop
	      model
    qtcontroller
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}

    jobcontroller
    ${Boost_LIBRARIES}
    )
ENDIF()


IF(Tinia_SERVER)
  ADD_EXECUTABLE(tutorial2_web
    ${tutorial_HEADERS}
    tutorial2_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial2_web
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
    tutorial2_web
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
