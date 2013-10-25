FILE( GLOB renderlistTestHeaders "*.hpp" )
FILE( GLOB renderlistTestSrc "*.cpp" )

ADD_EXECUTABLE( renderlist_unittest ${renderlistTestSrc} ${renderlistTestHeaders} )
ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
TARGET_LINK_LIBRARIES( renderlist_unittest ${Boost_LIBRARIES} tinia_renderlist tinia_renderlistgl )
