FILE(GLOB qtobserver_HEADERS "../../include/tinia/qtobserver/*.h" "../../include/tinia/qtobserver/*.hpp")
FILE(GLOB qtobserver_SOURCES "*.c" "*.cpp")
FILE(GLOB qtobserver_FORMS "*.ui")
FILE(GLOB qtobserver_SOURCES_TO_BE_MOCED
    "../../include/tinia/qtobserver/moc/*.hpp"
    "../../include/tinia/qtobserver/moc/*.h")

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
    policy
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
    DESTINATION include/tinia/qtobserver  COMPONENT qtobserver-dev)
  install( FILES ${qtobserver_HEADERS_MOC} ${qtobserver_SOURCES_TO_BE_MOCED}
    DESTINATION include/tinia/qtobserver/moc  COMPONENT qtobserver-dev)
  install( TARGETS QtObserver LIBRARY DESTINATION lib/ COMPONENT qtobserver)
ELSE()
 INSTALL(DIRECTORY include/tinia/qtobserver
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS QtObserver
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()