SET( Tinia_FIND_DESKTOP FALSE )
SET( Tinia_FIND_SERVER FALSE)

IF( NOT "${Tinia_FIND_COMPONENTS}" STREQUAL "" )
  FOREACH(component ${Tinia_FIND_COMPONENTS})
    IF( ${component} STREQUAL "Desktop" )
      SET( Tinia_FIND_DESKTOP TRUE )
    ELSEIF(${component} STREQUAL "Server" )
      SET( Tinia_FIND_SERVER TRUE )
    ELSE()
      MESSAGE( "Unknown component ${component}" )
    ENDIF()
  ENDFOREACH()
ELSE()
    #MESSAGE( "No components" )
ENDIF()


IF(WIN32)
  SET( Tinia_3RDPARTY_LOC $ENV{TINIA_3RDPARTY} CACHE PATH "Location of Tinia 3rdparty" )
  SET( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${Tinia_3RDPARTY_LOC}" )
ENDIF()


include(FindPackageHandleStandardArgs)
# Yes, we really DO want to find the libraries Tinia links against, in
# order to ensure that the user doesn't need to worry about which libraries
# are required
set(BOOST_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE( Boost 1.46 COMPONENTS filesystem unit_test_framework prg_exec_monitor thread date_time system )

IF(${Tinia_FIND_DESKTOP})
  FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXML QtScript QtNetwork REQUIRED)
  INCLUDE(${QT_USE_FILE})
  SET(QT_USE_QTOPENGL TRUE)
      SET(QT_USE_QTNETWORK TRUE)
  SET(QT_USE_QTXML TRUE)
  SET(QT_USE_QTSCRIPT TRUE)
  ADD_DEFINITIONS(${QT_DEFINITIONS})
  ADD_DEFINITIONS(-DQT_SHARED)
ENDIF()

FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( GLM REQUIRED )
FIND_PACKAGE( LibXml2 )


IF( ${Tinia_FIND_SERVER} )

  FIND_PATH( APACHE_INCLUDE_DIR httpd.h
    HINTS "/usr/include/apache2" "/usr/include/httpd/"
  )

  FIND_LIBRARY( RT
    NAMES rt
    PATHS "/usr/lib/x86_64-linux-gnu/" "/usr/lib/"
  )

  FIND_LIBRARY( LIB_APR
    NAMES apr-1
    PATHS "/usr/lib/"
  )

  FIND_PATH(APR_INCLUDE_DIR "apr.h"
    HINTS   "/usr/include/apr-1.0"
    "/usr/include/apr-1"
    "apr-1/"
    "apr-1.0"
  )

ENDIF()


FIND_PATH( Tinia_INCLUDE_DIR tinia/model/ExposedModel.hpp
  "/usr/local/include/"
  "/work/projects/tinia/include/"
  "$ENV{HOME}/include/"
  "$ENV{HOME}/install/include/"
  "$ENV{HOME}/projects/tinia/include"
  "${Tinia_ROOT}/include/"
  "../tinia/include/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/include"
)


set( Tinia_LIB_LOCATIONS
  "/usr/local/lib/"
  "/work/projects/tinia/"
  "$ENV{HOME}/lib/"
  "$ENV{HOME}/install/lib/"
  "${Tinia_ROOT}/lib/"
  "${Tinia_ROOT}/lib/Release/"
  "${Tinia_ROOT}/lib/Debug/"
  "../tinia/build/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
)
MARK_AS_ADVANCED( Tinia_LIB_LOCATIONS )


foreach ( lib renderlist renderlistgl model jobcontroller modelxml )
  FIND_LIBRARY( Tinia_${lib}_LIBRARY NAMES tinia_${lib} tinia_${lib}d
    ${Tinia_LIB_LOCATIONS}
    "/work/projects/tinia/src/${lib}"
    "$ENV{HOME}/projects/tinia/build/src/${lib}"
  )
  MARK_AS_ADVANCED( Tinia_$lib_LIBRARY )
endforeach( lib )


IF( ${Tinia_FIND_DESKTOP} )
  foreach ( lib javascript qtcontroller )
    FIND_LIBRARY( Tinia_${lib}_LIBRARY NAMES tinia_${lib} tinia_${lib}d
      ${Tinia_LIB_LOCATIONS}
      "/work/projects/tinia/src/${lib}"
      "$ENV{HOME}/projects/tinia/build/src/${lib}"
    )
    MARK_AS_ADVANCED( Tinia_$lib_LIBRARY )
  endforeach( lib )
ENDIF()


IF( ${Tinia_FIND_SERVER} )
  foreach ( lib trell )
    FIND_LIBRARY( Tinia_${lib}_LIBRARY NAMES tinia_${lib} tinia_${lib}d
      ${Tinia_LIB_LOCATIONS}
      "/work/projects/tinia/src/${lib}"
      "$ENV{HOME}/projects/tinia/build/src/${lib}"
    )
    MARK_AS_ADVANCED( Tinia_$lib_LIBRARY )
  endforeach( lib )
ENDIF()




IF( ${Tinia_FIND_DESKTOP} )
  IF( NOT ${Tinia_FIND_SERVER} )
    find_package_handle_standard_args( Tinia DEFAULT_MSG
      Tinia_jobcontroller_LIBRARY
      Tinia_qtcontroller_LIBRARY
      Tinia_model_LIBRARY
      Tinia_INCLUDE_DIR
      Tinia_renderlist_LIBRARY
      Tinia_renderlistgl_LIBRARY
      Tinia_modelxml_LIBRARY
      Tinia_javascript_LIBRARY
      # With javascript
    )
  ELSE()
    find_package_handle_standard_args( Tinia DEFAULT_MSG
      Tinia_jobcontroller_LIBRARY
      Tinia_qtcontroller_LIBRARY
      Tinia_model_LIBRARY
      Tinia_INCLUDE_DIR
      Tinia_renderlist_LIBRARY
      Tinia_renderlistgl_LIBRARY
      Tinia_trell_LIBRARY
      Tinia_modelxml_LIBRARY
      # With trell
    )
  ENDIF()
ELSEIF( ${Tinia_FIND_SERVER} )
  find_package_handle_standard_args( Tinia DEFAULT_MSG
    Tinia_jobcontroller_LIBRARY
    Tinia_model_LIBRARY
    Tinia_INCLUDE_DIR
    Tinia_renderlist_LIBRARY
    Tinia_renderlistgl_LIBRARY
    Tinia_trell_LIBRARY
    Tinia_modelxml_LIBRARY
    # Without qtcontroller and javascript
  )
ENDIF()


SET( Tinia_INCLUDE_DIRS
  ${Tinia_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)
SET( Tinia_LIBRARIES
  ${Boost_LIBRARIES}
  ${GLEW_LIBRARY}
  ${Tinia_model_LIBRARY}
  ${Tinia_jobcontroller_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${Tinia_renderlist_LIBRARY}
  ${Tinia_renderlistgl_LIBRARY}
)


IF( ${Tinia_FIND_DESKTOP} )
  SET( Tinia_INCLUDE_DIRS
    ${Tinia_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${QT_QTOPENGL_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
  )
  SET( Tinia_LIBRARIES
    ${Tinia_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${Tinia_qtcontroller_LIBRARY}
    ${LIBXML2_LIBRARIES}
    ${Tinia_modelxml_LIBRARY}
    ${Tinia_javascript_LIBRARY}
  )
ENDIF()


IF(${Tinia_FIND_SERVER})
  SET(Tinia_INCLUDE_DIRS
    ${Tinia_INCLUDE_DIRS}
    ${APACHE_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
  )
  SET(Tinia_LIBRARIES
    ${Tinia_LIBRARIES}
    #   ${Tinia_modelxml_LIBRARY}
    ${Tinia_trell_LIBRARY}
    ${RT}
    ${LIB_APR}
  )
ENDIF()


IF( NOT (${LIBXML2_FOUND}) )
  MESSAGE( "LibXml2 not found. \nYou can still build desktop projects, but loose the ability to use job as web-application or take it with you" )
ELSE()
  ADD_DEFINITIONS(-DTINIA_HAVE_LIBXML)
ENDIF()
