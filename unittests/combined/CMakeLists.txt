FILE( GLOB combinedTestHeaders "../*/*.hpp" )
FILE( GLOB combinedTestSrc "../*/*.cpp" )

file( GLOB mainApps "../*/main.cpp" )
list( REMOVE_ITEM combinedTestSrc ${mainApps} )

IF( NOT Tinia_SERVER )
  #The following files are only applicable for Tinia_Server
  FILE( GLOB toRemove "../*/test_*.cpp" )
  LIST( REMOVE_ITEM combinedTestSrc ${toRemove} )
ENDIF( WIN32 )

file( GLOB combinedMain main.cpp ) # Assuming absolute path is a good thing
list( APPEND combinedTestSrc ${combinedMain} )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
ADD_EXECUTABLE( combined_unittest ${combinedTestSrc} ${combinedTestHeaders} )
TARGET_LINK_LIBRARIES( combined_unittest
                       ${Boost_LIBRARIES}
                       ${LIBXML2_LIBRARIES}
                       tinia_jobcontroller
                       tinia_model
                       tinia_modelxml
                       tinia_qtcontroller
                       tinia_renderlist
                       tinia_renderlistgl
                       ${TINIA_LIBRARIES} )
