FILE( GLOB jobcontrollerTestHeaders "*.hpp" )
FILE( GLOB jobcontrollerTestSrc "*.cpp" )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

ADD_EXECUTABLE( jobcontroller_unittest 
  ${jobcontrollerTestSrc} 
  ${jobcontrollerTestHeaders} )

TARGET_LINK_LIBRARIES( jobcontroller_unittest ${Boost_LIBRARIES} tinia_model tinia_jobcontroller)
