FILE( GLOB jobobserverTestHeaders "*.hpp" )
FILE( GLOB jobobserverTestSrc "*.cpp" )

ADD_EXECUTABLE( jobobserver_unittest 
  ${jobobserverTestSrc} 
  ${jobobserverTestHeaders} )

TARGET_LINK_LIBRARIES( jobobserver_unittest ${Boost_LIBRARIES} model jobobserver)