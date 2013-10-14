FILE( GLOB combinedTestHeaders "../*/*.hpp" )
FILE( GLOB combinedTestSrc "../*/*.cpp" )

file( GLOB mainApps "../*/main.cpp" )
list( REMOVE_ITEM combinedTestSrc ${mainApps} )

file( GLOB combinedMain main.cpp ) # Assuming absolute path is a good thing
list( APPEND combinedTestSrc ${combinedMain} )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
ADD_EXECUTABLE( combined_unittest ${combinedTestSrc} ${combinedTestHeaders} )
TARGET_LINK_LIBRARIES( combined_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} jobcontroller model modelxml qtcontroller renderlist renderlistgl )
