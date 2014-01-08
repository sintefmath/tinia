FILE( GLOB modelxmlSrc "*.cpp" )
FILE( GLOB modelxmlHeaders "../../include/tinia/impl/xml/*.hpp" )

ADD_LIBRARY( tinia_modelxml ${modelxmlSrc} ${modelxmlHeaders})

TARGET_LINK_LIBRARIES( tinia_modelxml ${Boost_LIBRARIES}
${LIBXML2_LIBRARIES} tinia_model)


install( TARGETS
         tinia_modelxml
        EXPORT TiniaTargets
    LIBRARY
     DESTINATION lib/ 
	 ARCHIVE DESTINATION lib/
     COMPONENT modelxml)
