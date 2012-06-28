SET( TRELL_MASTER_SRC
    "master_main.cpp"
    "Master.cpp"
)

ADD_EXECUTABLE( trell_master ${TRELL_MASTER_SRC} )
TARGET_LINK_LIBRARIES( trell_master renderlist renderlistgl trell ${RT} jobobserver  ${LIBXML2_LIBRARIES} model modelxml )


INSTALL( TARGETS 
  trell_master
  RUNTIME DESTINATION "/var/trell/bin"
    COMPONENT mod_trell
    PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

