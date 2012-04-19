FILE( GLOB librenderlistTestHeaders "*.hpp" )
FILE( GLOB librenderlistTestSrc "*.cpp" )

ADD_EXECUTABLE( librenderlist_unittest ${librenderlistTestSrc} ${librenderlistTestHeaders} )
TARGET_LINK_LIBRARIES( librenderlist_unittest ${Boost_LIBRARIES} renderlist renderlistgl )