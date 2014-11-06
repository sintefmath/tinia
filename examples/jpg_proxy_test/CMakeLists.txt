# jpc = jpg proxy cube

SET( jpc_qt_job_SOURCES "JPCJob.cpp" "main.cpp" )
SET( jpc_job_HEADERS "JPCJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(jpc_qt_job
    ${jpc_qt_job_SOURCES}
    ${jpc_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(jpc_qt_job
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

    INSTALL( TARGETS jpc_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT jpc_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( jpc_cloud_job_SOURCES "JPCJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(jpc_cloud_job
    ${jpc_cloud_job_SOURCES}
    ${jpc_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(jpc_cloud_job
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    tinia_model 
    tinia_utils
    tinia_modelxml
    tinia_renderlist
    tinia_renderlistgl
    tinia_jobcontroller
    tinia_trell
    )

    
  INSTALL(TARGETS
    jpc_cloud_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
