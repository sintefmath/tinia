SET( restrictions_qt_job_SOURCES "RestrictionsJob.cpp" "main.cpp" )
SET( restrictions_job_HEADERS "RestrictionsJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(restrictions_qt_job
    ${restrictions_qt_job_SOURCES}
    ${restrictions_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(restrictions_qt_job
      ${LIBXML2_LIBRARIES} 
    qtcontroller
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    model
    jobcontroller
    ${Boost_LIBRARIES}
    )

    INSTALL( TARGETS restrictions_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT restrictions_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( restrictions_cloud_job_SOURCES "RestrictionsJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(restrictions_cloud_job
    ${restrictions_cloud_job_SOURCES}
    ${restrictions_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(restrictions_cloud_job
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
    restrictions_cloud_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
