FILE( GLOB qtcontrollerTestHeaders "*.hpp" )
FILE( GLOB qtcontrollerTestSrc "*.cpp" )
IF(NOT ${LIBXML2_FOUND})
    FILE(GLOB_RECURSE serverToRemove "*Server*")
    LIST(REMOVE_ITEM qtcontrollerTestSrc ${serverToRemove} )
    LIST(REMOVE_ITEM qtcontrollerTestHeaders ${serverToRemove} )

ENDIF()

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

ADD_EXECUTABLE( qtcontroller_unittest 
  ${qtcontrollerTestSrc} 
  ${qtcontrollerTestHeaders} )

IF(${LIBXML2_FOUND})
TARGET_LINK_LIBRARIES( qtcontroller_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} tinia_model tinia_modelxml tinia_qtcontroller tinia_javascript ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
ELSE()
TARGET_LINK_LIBRARIES( qtcontroller_unittest ${Boost_LIBRARIES} tinia_model tinia_qtcontroller tinia_javascript ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
ENDIF()
