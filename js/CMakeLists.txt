FILE(GLOB javascript_RESOURCES "*.qrc")
FILE(GLOB_RECURSE javascript_SOURCES "." "*.js" "*.html" "*.css")
QT4_ADD_RESOURCES( javascript_QRC_SOURCES ${javascript_RESOURCES} )

add_library( tinia_javascript
    ${javascript_QRC_SOURCES}
    ${javascript_SOURCES}
    )
target_link_libraries ( tinia_javascript
    ${QT_LIBRARIES}
)


install( TARGETS 
    tinia_javascript LIBRARY 
    DESTINATION lib/ 
    ARCHIVE DESTINATION /lib
    COMPONENT qtcontroller)
