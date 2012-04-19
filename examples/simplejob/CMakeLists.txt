SET( test_qt_job_SOURCES "TestJob.cpp" "main.cpp" )
SET( test_job_HEADERS "TestJob.hpp" )

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
  policyLib
  jobobserver
  ${Boost_LIBRARIES}
)

SET( test_cloud_job_SOURCES "TestJob.cpp" "cloud_main.cpp" )

#ADD_EXECUTABLE(simple_cloud_job
#  ${test_cloud_job_SOURCES}
#  ${test_job_HEADERS}
#)

#TARGET_LINK_LIBRARIES(simple_cloud_job
#  ${LIBXML2_LIBRARIES}
#  ${Boost_LIBRARIES}
#  ${LIBXML2_LIBRARIES}
#  ${LIBXML2_LIBRARY}
#  policyLib
#  policyLibXml
#  jobobserver
#  trell
#  renderlist
#  renderlistgl
#  ${OPENGL_LIBRARIES}
#  ${GLEW_LIBRARY}

#)