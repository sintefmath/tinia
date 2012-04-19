FILE( GLOB renderlist_SRC "*.cpp" )
ADD_LIBRARY( renderlist ${renderlist_SRC} )
TARGET_LINK_LIBRARIES( renderlist )
