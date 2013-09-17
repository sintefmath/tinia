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

IF(${Tinia_FIND_SERVER})
  FIND_PATH( APACHE_INCLUDE_DIR httpd.h
    HINTS "/usr/include/apache2" "/usr/include/httpd/"
  )

  FIND_LIBRARY( RT
    NAMES rt
    PATHS "/usr/lib/x86_64-linux-gnu/" "/usr/lib/"
    )

  FIND_LIBRARY(LIB_APR
    NAMES apr-1
    PATHS "/usr/lib/"
    )

  FIND_PATH(APR_INCLUDE_DIR "apr.h"
HINTS   "/usr/include/apr-1.0"
        "/usr/include/apr-1"
        "apr-1/"
        "apr-1.0")

ENDIF()


FIND_PATH( TINIA_INCLUDE_DIR tinia/model/ExposedModel.hpp
  "/usr/local/include/"
  "/work/projects/tinia/include/"
  "$ENV{HOME}/include/"
  "$ENV{HOME}/install/include/"
  "$ENV{HOME}/projects/tinia/include"
  "${TINIA_ROOT}/include/"
  "../tinia/include/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/include"
  )


FIND_LIBRARY( tinia_RenderList_LIBRARY renderlist
  "/usr/local/lib/"
  "/work/projects/tinia/"
  "/work/projects/tinia/src/renderlist"
  "$ENV{HOME}/projects/tinia/build/src/renderlist"
  "$ENV{HOME}/lib/"
  "$ENV{HOME}/install/lib/"
  "${TINIA_ROOT}/lib/"
  "${TINIA_ROOT}/lib/Release/"
  "${TINIA_ROOT}/lib/Debug/"
  "../tinia/build/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )


FIND_LIBRARY( tinia_RenderListGL_LIBRARY renderlistgl
  "/usr/local/lib/"
  "/work/projects/tinia/"
  "/work/projects/tinia/src/renderlistgl/"
  "$ENV{HOME}/projects/tinia/build/src/renderlistgl/"
  "$ENV{HOME}/lib/"
  "$ENV{HOME}/install/lib/"
  "${TINIA_ROOT}/lib/"
  "${TINIA_ROOT}/lib/Release/"
  "${TINIA_ROOT}/lib/Debug/"
  "../tinia/build/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )


FIND_LIBRARY( tinia_model_LIBRARY model
  "/usr/local/lib/"
  "/work/projects/tinia/"
  "/work/projects/tinia/src/model"
  "$ENV{HOME}/projects/tinia/build/src/model"
  "$ENV{HOME}/lib/"
  "$ENV{HOME}/install/lib/"
  "${TINIA_ROOT}/lib/"
  "${TINIA_ROOT}/lib/Release/"
  "${TINIA_ROOT}/lib/Debug/"
  "../tinia/build/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )


FIND_LIBRARY( tinia_jobcontroller_LIBRARY jobcontroller
 "/usr/local/lib/"
  "/work/projects/tinia/"
  "/work/projects/tinia/src/jobcontroller"
  "$ENV{HOME}/projects/tinia/build/src/jobcontroller"
  "$ENV{HOME}/lib/"
  "$ENV{HOME}/install/lib/"
  "${TINIA_ROOT}/lib/"
  "${TINIA_ROOT}/lib/Release/"
  "${TINIA_ROOT}/lib/Debug/"
  "../tinia/build/"
  "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )


IF( ${Tinia_FIND_DESKTOP} )
	
  FIND_LIBRARY( tinia_javascript_LIBRARY tinia_javascript
    "/usr/local/lib/"
    "/work/projects/tinia/"
    "/work/projects/tinia/src/qtcontroller"
    "$ENV{HOME}/projects/tinia/build/src/qtcontroller"
    "$ENV{HOME}/lib/"
    "$ENV{HOME}/install/lib/"
    "${TINIA_ROOT}/lib/"
    "${TINIA_ROOT}/lib/Release/"
    "${TINIA_ROOT}/lib/Debug/"
    "../tinia/build/"
    "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )
  FIND_LIBRARY( tinia_qtcontroller_LIBRARY qtcontroller
    "/usr/local/lib/"
    "/work/projects/tinia/"
    "/work/projects/tinia/src/qtcontroller"
    "$ENV{HOME}/projects/tinia/build/src/qtcontroller"
    "$ENV{HOME}/lib/"
    "$ENV{HOME}/install/lib/"
    "${TINIA_ROOT}/lib/"
    "${TINIA_ROOT}/lib/Release/"
    "${TINIA_ROOT}/lib/Debug/"
    "../tinia/build/"
    "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
  )

ENDIF()

FIND_LIBRARY( tinia_modelxml_LIBRARY modelxml
    "/usr/local/lib/"
    "/work/projects/tinia/"
    "/work/projects/tinia/src/modelxml"
    "$ENV{HOME}/projects/tinia/build/src/modelxml"
    "$ENV{HOME}/lib/"
    "$ENV{HOME}/install/lib/"
    "${TINIA_ROOT}/lib/"
    "${TINIA_ROOT}/lib/Release/"
    "${TINIA_ROOT}/lib/Debug/"
    "../tinia/build/"
    "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
    )

IF( ${Tinia_FIND_SERVER} )

  FIND_LIBRARY( tinia_trell_LIBRARY trell
    "/usr/local/lib/"
    "/work/projects/tinia/"
    "/work/projects/tinia/src/trell"
    "$ENV{HOME}/projects/tinia/build/src/trell"
    "$ENV{HOME}/lib/"
    "$ENV{HOME}/install/lib/"
    "${TINIA_ROOT}/lib/"
    "${TINIA_ROOT}/lib/Release/"
    "${TINIA_ROOT}/lib/Debug/"
    "../tinia/build/"
    "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia/lib"
    )
  
ENDIF()

IF( ${Tinia_FIND_DESKTOP} )
  IF(NOT ${Tinia_FIND_SERVER})
    find_package_handle_standard_args(Tinia  DEFAULT_MSG
                                  tinia_jobcontroller_LIBRARY
                                  tinia_qtcontroller_LIBRARY
                                  tinia_model_LIBRARY
                                  TINIA_INCLUDE_DIR
                                  tinia_RenderList_LIBRARY
                                  tinia_RenderListGL_LIBRARY
				  tinia_modelxml_LIBRARY
				  tinia_javascript_LIBRARY)
  ELSE()
    find_package_handle_standard_args(Tinia  DEFAULT_MSG
                                  tinia_jobcontroller_LIBRARY
                                  tinia_qtcontroller_LIBRARY
                                  tinia_model_LIBRARY
                                  TINIA_INCLUDE_DIR
                                  tinia_RenderList_LIBRARY
                                  tinia_RenderListGL_LIBRARY
                                  tinia_trell_LIBRARY
                                  tinia_modelxml_LIBRARY)
  ENDIF()
ELSEIF(${Tinia_FIND_SERVER})

  find_package_handle_standard_args(Tinia  DEFAULT_MSG
                                  tinia_jobcontroller_LIBRARY
                                  tinia_model_LIBRARY
                                  TINIA_INCLUDE_DIR
                                  tinia_RenderList_LIBRARY
                                  tinia_RenderListGL_LIBRARY
                                  tinia_trell_LIBRARY
                                  tinia_modelxml_LIBRARY)
ENDIF()

SET(TINIA_INCLUDE_DIRS
  ${TINIA_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS} )


SET(TINIA_LIBRARIES
  ${Boost_LIBRARIES}
  ${GLEW_LIBRARY}
  ${tinia_model_LIBRARY}
  ${tinia_jobcontroller_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${tinia_RenderList_LIBRARY}
  ${tinia_RenderListGL_LIBRARY}
)

IF(${Tinia_FIND_DESKTOP} )
  SET(TINIA_LIBRARIES
    ${TINIA_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
    ${tinia_qtcontroller_LIBRARY}
    ${LIBXML2_LIBRARIES}
    ${tinia_modelxml_LIBRARY}
    #${tinia_javascript_LIBRARY}
    )

  SET(TINIA_INCLUDE_DIRS
    ${TINIA_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${QT_QTOPENGL_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    )
ENDIF()

IF(${Tinia_FIND_SERVER})
  SET(TINIA_LIBRARIES
    ${TINIA_LIBRARIES}
 #   ${tinia_modelxml_LIBRARY}
    ${tinia_trell_LIBRARY}
    ${RT}
    ${LIB_APR}
  )

  SET(TINIA_INCLUDE_DIRS
    ${TINIA_INCLUDE_DIRS}
    ${APACHE_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
  )
ENDIF()

IF( NOT LIBXML2_FOUND )
  MESSAGE( "LibXml2 not found. \nYou can still build desktop projects, but lose the ability to use job as web-application or take it with you" )
ELSE()
    ADD_DEFINITIONS(-DTINIA_HAVE_LIBXML)
ENDIF()
