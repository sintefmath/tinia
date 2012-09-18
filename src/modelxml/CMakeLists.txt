FILE( GLOB modelxmlSrc "*.cpp" )
FILE( GLOB modelxmlHeaders "../../include/tinia/impl/xml/*.hpp" )

ADD_LIBRARY( modelxml ${modelxmlSrc} ${modelxmlHeaders})

TARGET_LINK_LIBRARIES( modelxml ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES})


install( TARGETS
	 modelxml LIBRARY 
     DESTINATION lib/ 
	 ARCHIVE DESTINATION lib/
     COMPONENT modelxml)
