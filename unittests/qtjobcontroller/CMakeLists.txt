FILE( GLOB qtjobcontrollerTestHeaders "*.hpp" )
FILE( GLOB qtjobcontrollerTestSrc "*.cpp" )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

# We use the qrc system of Qt to embed the javascript files to avoid dependencies on where
# the .js files are stored.
QT4_ADD_RESOURCES( QRC_SOURCES qtjobcontroller.qrc )

ADD_EXECUTABLE( qtjobcontroller_unittest
  ${QRC_SOURCES}
  ${qtjobcontrollerTestSrc}
  ${qtjobcontrollerTestHeaders} )

TARGET_LINK_LIBRARIES( qtjobcontroller_unittest ${Boost_LIBRARIES}  ${QT_LIBRARIES}  )
