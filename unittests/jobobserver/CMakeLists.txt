FILE( GLOB jobobserverTestHeaders "*.hpp" )
FILE( GLOB jobobserverTestSrc "*.cpp" )

ADD_EXECUTABLE( jobobserver_unittest 
  ${jobobserverTestSrc} 
  ${jobobserverTestHeaders} )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
TARGET_LINK_LIBRARIES( jobobserver_unittest ${Boost_LIBRARIES} model jobobserver)