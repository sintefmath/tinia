FILE( GLOB renderlistTestHeaders "*.hpp" )
FILE( GLOB renderlistTestSrc "*.cpp" )

ADD_EXECUTABLE( renderlist_unittest ${renderlistTestSrc} ${renderlistTestHeaders} )
TARGET_LINK_LIBRARIES( renderlist_unittest ${Boost_LIBRARIES} renderlist renderlistgl )