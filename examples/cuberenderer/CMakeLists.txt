SET( cube_qt_job_SOURCES "CubeJob.cpp" "main.cpp" )
SET( cube_job_HEADERS "CubeJob.hpp" "utils.hpp" )
IF(Tinia_DESKTOP)
  ADD_EXECUTABLE(cube_qt_job
    ${cube_qt_job_SOURCES}
    ${cube_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(cube_qt_job
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

    INSTALL( TARGETS cube_qt_job
        EXPORT TiniaTargets
        RUNTIME DESTINATION bin
        COMPONENT cube_qt_job
    )
                
   

ENDIF()


IF(Tinia_SERVER)
  SET( cube_cloud_job_SOURCES "CubeJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(cube_cloud_job
    ${cube_cloud_job_SOURCES}
    ${cube_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(cube_cloud_job
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
    cube_cloud_job
    EXPORT TiniaTargets
    RUNTIME
    DESTINATION var/trell/apps/
    COMPONENT mod_trell
    )
ENDIF()
