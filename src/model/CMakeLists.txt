FILE( GLOB modelSrc "*.cpp" )
FILE( GLOB_RECURSE modelHeaders "../../include/tinia/model/*.hpp")


ADD_LIBRARY( model ${modelSrc} ${modelHeaders} )

TARGET_LINK_LIBRARIES( model ${Boost_LIBRARIES} )

IF(NOT WIN32)
  install( FILES 
    ${modelHeaders} 
    DESTINATION include/tinia/model  
    COMPONENT model-dev)
  install( TARGETS  
    model LIBRARY 
    DESTINATION lib/ 
    COMPONENT model)
ELSE()
 INSTALL(DIRECTORY include/tinia/model
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS model
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
