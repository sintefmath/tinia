FILE(GLOB_RECURSE qtcontroller_HEADERS "../../include/tinia/qtcontroller/*.h" "../../include/tinia/qtcontroller/*.hpp")
FILE(GLOB qtcontroller_SOURCES "*.c" "*.cpp")
FILE(GLOB qtcontroller_FORMS "*.ui")
FILE(GLOB qtcontroller_SOURCES_TO_BE_MOCED
    "../../include/tinia/qtcontroller/moc/*.hpp"
    "../../include/tinia/qtcontroller/moc/*.h")

QT4_WRAP_CPP(qtcontroller_HEADERS_MOC ${qtcontroller_SOURCES_TO_BE_MOCED} ${qtcontroller_SOURCES})
QT4_WRAP_UI(qtcontroller_FORMS_HEADERS ${qtcontroller_FORMS})

IF(NOT WIN32)
  add_library( qtcontroller 
    ${qtcontroller_HEADERS}
    ${qtcontroller_SOURCES}
    ${qtcontroller_HEADERS_MOC}
    ${qtcontroller_SOURCES_TO_BE_MOCED}
    ${qtcontroller_FORMS_HEADERS}
    )
ELSE()
  add_library( qtcontroller
    ${qtcontroller_HEADERS}
    ${qtcontroller_SOURCES}
    ${qtcontroller_HEADERS_MOC}
    ${qtcontroller_FORMS_HEADERS}
    )
ENDIF()
target_link_libraries ( qtcontroller
    renderlist
    renderlistgl
    model
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    jobcontroller
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Freeglut_LIBRARY}
)

IF(NOT WIN32)
  # TODO We probably don't need to include all the [moced] headers in the final
  # libary, but keeping it simple for now.
  install( FILES ${qtcontroller_HEADERS}
    DESTINATION include/tinia/qtcontroller  COMPONENT qtcontroller-dev)
  install( FILES ${qtcontroller_HEADERS_MOC} ${qtcontroller_SOURCES_TO_BE_MOCED}
    DESTINATION include/tinia/qtcontroller/moc  COMPONENT qtcontroller-dev)
  install( TARGETS qtcontroller LIBRARY DESTINATION lib/ COMPONENT qtcontroller)
ELSE()
 INSTALL(DIRECTORY include/tinia/qtcontroller
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS qtcontroller
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
