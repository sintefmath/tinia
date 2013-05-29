SET( clock_qt_job_SOURCES "ClockJob.cpp" "main.cpp" )
SET( clock_job_HEADERS "ClockJob.hpp" "utils.hpp" )
IF(${Tinia_DESKTOP})
  ADD_EXECUTABLE(clock_qt_job
    ${clock_qt_job_SOURCES}
    ${clock_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(clock_qt_job
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

    INSTALL( TARGETS clock_qt_job
        RUNTIME DESTINATION bin
        COMPONENT clock_qt_job
    )
                
   

ENDIF()


IF(${Tinia_SERVER})
  SET( clock_cloud_job_SOURCES "ClockJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(clock_cloud_job
    ${clock_cloud_job_SOURCES}
    ${clock_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(clock_cloud_job
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
    clock_cloud_job RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
