FILE( GLOB qtjobcontrollerTestHeaders "*.hpp" )
FILE( GLOB qtjobcontrollerTestSrc "*.cpp" )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

ADD_EXECUTABLE( qtjobcontroller_unittest
  ${qtjobcontrollerTestSrc}
  ${qtjobcontrollerTestHeaders} )

TARGET_LINK_LIBRARIES( qtjobcontroller_unittest ${Boost_LIBRARIES} )
