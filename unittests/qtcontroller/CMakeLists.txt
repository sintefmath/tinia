FILE( GLOB qtcontrollerTestHeaders "*.hpp" )
FILE( GLOB qtcontrollerTestSrc "*.cpp" )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

ADD_EXECUTABLE( qtcontroller_unittest 
  ${qtcontrollerTestSrc} 
  ${qtcontrollerTestHeaders} )

TARGET_LINK_LIBRARIES( qtcontroller_unittest ${Boost_LIBRARIES} model qtcontroller tinia_javascript ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
