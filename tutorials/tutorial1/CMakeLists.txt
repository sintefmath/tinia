
SET( tutorial_HEADERS "Tutorial1_Job.hpp" )
SET( tutorial_DOC "tutorial1.md" )

IF(${Tinia_DESKTOP})
ADD_EXECUTABLE(tutorial1_desktop
    ${tutorial_HEADERS}
    ${tutorial_DOC}
    tutorial1_desktop.cpp
)
  TARGET_LINK_LIBRARIES(tutorial1_desktop
      
          tinia_model
    tinia_qtcontroller
    tinia_renderlist
    tinia_renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    tinia_jobcontroller
    ${Boost_LIBRARIES}
    )
ENDIF()


IF(${Tinia_SERVER})
  ADD_EXECUTABLE(tutorial1_web
    ${tutorial_HEADERS}
    tutorial1_web.cpp
    )

  TARGET_LINK_LIBRARIES(tutorial1_web
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    tinia_model
    tinia_modelxml
    tinia_renderlist
    tinia_renderlistgl
    tinia_jobcontroller
    tinia_trell
    )


  INSTALL(TARGETS
    tutorial1_web RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
