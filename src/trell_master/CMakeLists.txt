SET( TRELL_MASTER_SRC
    "master_main.cpp"
    "Master.cpp"
)

ADD_EXECUTABLE( trell_master ${TRELL_MASTER_SRC} )
TARGET_LINK_LIBRARIES( trell_master renderlist renderlistgl trell ${RT} jobobserver  ${LIBXML2_LIBRARIES} policyLib policyLibXml )