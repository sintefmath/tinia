FILE( GLOB modelxmlTestHeaders "*.hpp" )
FILE( GLOB modelxmlTestSrc "*.cpp" )

ADD_EXECUTABLE( modelxml_unittest ${modelxmlTestSrc} ${modelxmlTestHeaders} )
TARGET_LINK_LIBRARIES( modelxml_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} model modelxml )