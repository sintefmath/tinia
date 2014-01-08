FILE( GLOB combinedTestHeaders "../*/*.hpp" )
FILE( GLOB combinedTestSrc "../*/*.cpp" )

file( GLOB mainApps "../*/main.cpp" "../*/test_main.cpp" )
list( REMOVE_ITEM combinedTestSrc ${mainApps} )

if(NOT Tinia_SERVER)
    # We don't want serverstuff if we're not building server
    file( GLOB serverToRemove "../ipc/*" )
    list( REMOVE_ITEM combinedTestSrc ${serverToRemove} )
endif()

file( GLOB combinedMain main.cpp ) # Assuming absolute path is a good thing
list( APPEND combinedTestSrc ${combinedMain} )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
ADD_EXECUTABLE( combined_unittest ${combinedTestSrc} ${combinedTestHeaders} )
TARGET_LINK_LIBRARIES( combined_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} tinia_jobcontroller tinia_model tinia_modelxml tinia_qtcontroller tinia_renderlist tinia_renderlistgl tinia_trell )
