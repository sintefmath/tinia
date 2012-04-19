FILE(GLOB qtobserver_HEADERS "../../include/qtobserver/*.h" "../../include/qtobserver/*.hpp")
FILE(GLOB qtobserver_SOURCES "*.c" "*.cpp")
FILE(GLOB qtobserver_FORMS "*.ui")
FILE(GLOB qtobserver_SOURCES_TO_BE_MOCED
    "../../include/qtobserver/moc/*.hpp"
    "../../include/qtobserver/moc/*.h")

QT4_WRAP_CPP(qtobserver_HEADERS_MOC ${qtobserver_SOURCES_TO_BE_MOCED} ${qtobserver_SOURCES})
QT4_WRAP_UI(qtobserver_FORMS_HEADERS ${qtobserver_FORMS})

IF(NOT WIN32)
  add_library( QtObserver SHARED
    ${qtobserver_HEADERS}
    ${qtobserver_SOURCES}
    ${qtobserver_HEADERS_MOC}
    ${qtobserver_FORMS_HEADERS}
    )
ELSE()
  add_library( QtObserver
    ${qtobserver_HEADERS}
    ${qtobserver_SOURCES}
    ${qtobserver_HEADERS_MOC}
    ${qtobserver_FORMS_HEADERS}
    )
ENDIF()
target_link_libraries ( QtObserver
    renderlist
    renderlistgl
    policyLib
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    jobobserver
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
)

IF(NOT WIN32)
  # TODO We probably don't need to include all the [moced] headers in the final
  # libary, but keeping it simple for now.
  install( FILES ${qtobserver_HEADERS}
    DESTINATION include/qtobserver  COMPONENT qtobserver-dev)
  install( FILES ${qtobserver_HEADERS_MOC} ${qtobserver_SOURCES_TO_BE_MOCED}
    DESTINATION include/qtobserver/moc  COMPONENT qtobserver-dev)
  install( TARGETS QtObserver LIBRARY DESTINATION lib/ COMPONENT qtobserver)
ENDIF()