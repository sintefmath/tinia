# apc = auto proxy cube

SET( apc_qt_job_SOURCES "APCJob.cpp" "main.cpp" )
SET( apc_job_HEADERS "APCJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(apc_qt_job
    ${apc_qt_job_SOURCES}
    ${apc_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(apc_qt_job
      ${LIBXML2_LIBRARIES} 
    tinia_qtcontroller
    tinia_renderlist
    tinia_renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    tinia_model
    tinia_utils
    tinia_jobcontroller
    ${Boost_LIBRARIES}
    )

    INSTALL( TARGETS apc_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT apc_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( apc_cloud_job_SOURCES "APCJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(apc_cloud_job
    ${apc_cloud_job_SOURCES}
    ${apc_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(apc_cloud_job
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    tinia_model 
    tinia_modelxml
    tinia_renderlist
    tinia_utils
    tinia_renderlistgl
    tinia_jobcontroller
    tinia_trell
    )

    
  INSTALL(TARGETS
    apc_cloud_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
