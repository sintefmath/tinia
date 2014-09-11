SET( two_qt_job_SOURCES "TwoJob.cpp" "main.cpp" )
SET( two_job_HEADERS "TwoJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(two_qt_job
    ${two_qt_job_SOURCES}
    ${two_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(two_qt_job
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

    INSTALL( TARGETS two_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT two_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( two_cloud_job_SOURCES "TwoJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(two_cloud_job
    ${two_cloud_job_SOURCES}
    ${two_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(two_cloud_job
    ${TINIA_LIBRARIES}
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
    two_cloud_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
