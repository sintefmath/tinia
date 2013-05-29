SET( restrictions_qt_job_SOURCES "RestrictionsJob.cpp" "main.cpp" )
SET( restrictions_job_HEADERS "RestrictionsJob.hpp" "utils.hpp" )
IF(${Tinia_DESKTOP})
  ADD_EXECUTABLE(restrictions_qt_job
    ${restrictions_qt_job_SOURCES}
    ${restrictions_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(restrictions_qt_job
      ${LIBXML2_LIBRARIES} 
    tinia_qtcontroller
    tinia_renderlist
    tinia_renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    tinia_model
    tinia_jobcontroller
    ${Boost_LIBRARIES}
    )

    INSTALL( TARGETS restrictions_qt_job
        RUNTIME DESTINATION bin
        COMPONENT restrictions_qt_job
    )
                
   

ENDIF()


IF(${Tinia_SERVER})
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
    tinia_model
    tinia_modelxml
    tinia_renderlist
    tinia_renderlistgl
    tinia_jobcontroller
    tinia_trell
    )

    
  INSTALL(TARGETS
    restrictions_cloud_job RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
