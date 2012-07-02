FILE( GLOB modelTestHeaders "*.hpp" )
FILE( GLOB modelTestSrc "*.cpp" )

ADD_EXECUTABLE( model_unittest ${modelTestSrc} ${modelTestHeaders} )
ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )
TARGET_LINK_LIBRARIES( model_unittest ${Boost_LIBRARIES} model )