SET( test_qt_job_SOURCES "TestJob.cpp" "main.cpp" )
SET( test_job_HEADERS "TestJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(simple_qt_job
    ${test_qt_job_SOURCES}
    ${test_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(simple_qt_job
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

    INSTALL( TARGETS simple_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT simple_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( test_cloud_job_SOURCES "TestJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(test_job
    ${test_cloud_job_SOURCES}
    ${test_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(test_job
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
    test_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/test_job/bin/
    COMPONENT mod_trell
    )
ENDIF()
