FILE( GLOB jobobserverSrc "*.cpp" )

ADD_LIBRARY( jobobserver SHARED ${jobobserverSrc} )

TARGET_LINK_LIBRARIES( jobobserver ${Boost_LIBRARIES} policyLib )