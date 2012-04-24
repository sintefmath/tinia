SET( test_qt_job_SOURCES "TestJob.cpp" "main.cpp" )
SET( test_job_HEADERS "TestJob.hpp" )
IF(${DESKTOP})
  ADD_EXECUTABLE(simple_qt_job
    ${test_qt_job_SOURCES}
    ${test_job_HEADERS}
    )
  TARGET_LINK_LIBRARIES(simple_qt_job
    QtObserver
    renderlist
    renderlistgl
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
    policy
    jobobserver
    ${Boost_LIBRARIES}
    )
ENDIF()

IF(${SERVER})
  SET( test_cloud_job_SOURCES "TestJob.cpp" "cloud_main.cpp" )

  ADD_EXECUTABLE(test_job
    ${test_cloud_job_SOURCES}
    ${test_job_HEADERS}
    )

  TARGET_LINK_LIBRARIES(test_job
    ${RT}
    ${LIB_APR}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${Freeglut_LIBRARY}
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    policy 
    policyXml
    renderlist
    renderlistgl
    jobobserver
    trell
    )

  INSTALL(TARGETS
    test_job RUNTIME
    DESTINATION /var/trell/apps/test_job/bin/
    COMPONENT mod_trell
    )
ENDIF()