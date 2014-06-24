FILE( GLOB ipcTestHeaders "*.hpp" )
FILE( GLOB ipcTestSrc "*.cpp" )

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

ADD_EXECUTABLE( ipc_unittest 
  ${ipcTestSrc} 
  ${ipcTestHeaders} )

TARGET_LINK_LIBRARIES( ipc_unittest ${Boost_LIBRARIES} ${TINIA_LIBRARIES} tinia_trell )
