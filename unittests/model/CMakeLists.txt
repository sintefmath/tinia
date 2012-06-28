FILE( GLOB modelTestHeaders "*.hpp" )
FILE( GLOB modelTestSrc "*.cpp" )

ADD_EXECUTABLE( model_unittest ${modelTestSrc} ${modelTestHeaders} )
TARGET_LINK_LIBRARIES( model_unittest ${Boost_LIBRARIES} model )