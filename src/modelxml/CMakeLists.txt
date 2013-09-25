FILE( GLOB modelxmlSrc "*.cpp" )
FILE( GLOB modelxmlHeaders "../../include/tinia/impl/xml/*.hpp" )

ADD_LIBRARY( modelxml ${modelxmlSrc} ${modelxmlHeaders})

TARGET_LINK_LIBRARIES( modelxml ${Boost_LIBRARIES}
${LIBXML2_LIBRARIES} model)


install( TARGETS
         modelxml
        EXPORT TiniaTargets
    LIBRARY
     DESTINATION lib/ 
	 ARCHIVE DESTINATION lib/
     COMPONENT modelxml)
