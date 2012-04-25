PROJECT( tinia )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

IF(NOT WIN32)
  OPTION( SERVER "Build mod_trell and related components?" ON)
ELSE()
  OPTION( SERVER "Build mod_trell and related components?" OFF)
ENDIF()

OPTION( DESKTOP "Build qtobserver?" ON)


OPTION( EXTEND_CMAKE_MODULE_PATH
        "Extend the CMAKE_MODULE_PATH variable with user directories?"
        ON )
IF( EXTEND_CMAKE_MODULE_PATH )
    SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                           "./cmake/Modules"
                           "~/cmake/Modules"
                           #"$ENV{HOMEPATH}/cmake/Modules"
                           ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules
                           "C:/cmake/Modules"
                           "../siut/cmakemacros"
                           "../siut"
                           "."
    )


IF(WIN32)
  SET(CMAKE_INSTALL_PREFIX CACHE INTERNAL "")
  SET(tinia_INSTALL_PREFIX
    "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia"
    CACHE PATH "Path to install tinia")
ENDIF()
ENDIF( EXTEND_CMAKE_MODULE_PATH )

SET( revision $ENV{GIT_REVISION} )
IF ( NOT DEFINED revision )
    SET( revision 0 )
ENDIF( NOT  DEFINED revision )

SET( build_number $ENV{BUILD_NUMBER} )
IF ( NOT DEFINED build_number )
    SET( build_number 0 )
ENDIF( NOT  DEFINED build_number )

SET( version_number "0.${revision}.${build_number}" )

IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET(CMAKE_CXX_FLAGS " -DDEBUG -g3 -Wall -fPIC -std=gnu++0x ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
ENDIF()
IF(MSVC10)
        #Enable multiprocessor compilation for speed
        SET(CMAKE_CXX_FLAGS " /MP ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
	#CAREFUL, this will append the above string for each time CMAKE is run!
ENDIF()


# We don't need XML support on Windows (only needed for mod_trell...)
IF(NOT WIN32)
  find_package( LibXml2 REQUIRED )
ENDIF()

##### BOOST #####
FIND_PACKAGE( Boost 1.46 COMPONENTS unit_test_framework prg_exec_monitor thread  date_time test_exec_monitor)

##### QT #####
IF(${DESKTOP})
  FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXML REQUIRED)
  INCLUDE(${QT_USE_FILE})
  SET(QT_USE_QTOPENGL TRUE)
  SET(QT_USE_QTXML TRUE)
  QT4_WRAP_CPP(qtobserver_HEADERS_MOC ${qtobserver_SOURCES_TO_BE_MOCED} ${qtobserver_SOURCES})
  QT4_WRAP_UI(qtobserver_FORMS_HEADERS ${qtobserver_FORMS})
  ADD_DEFINITIONS(${QT_DEFINITIONS})
  ADD_DEFINITIONS(-DQT_SHARED)
ENDIF()


##### OPENGL #####
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( GLM REQUIRED )

IF(${SERVER})
  FIND_LIBRARY( RT
    NAMES rt
    PATHS "/usr/lib/x86_64-linux-gnu/" "/usr/lib/"
    )

  FIND_LIBRARY(LIB_APR
    NAMES apr-1
    PATHS "/usr/lib/"
    )
  SET(APACHE_INCLUDE_DIR "/usr/include/apache2")
  SET(APR_INCLUDE_DIR   "/usr/include/apr-1.0"
    "/usr/include/apr-1")
ENDIF()

include_directories( "./include" 
  ${LIBXML2_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${APACHE_INCLUDE_DIR}
  ${APR_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${QT_QTOPENGL_INCLUDE_DIR}
  ${BOOST_INCLUDE_DIRS}
)

ADD_SUBDIRECTORY( "src/policy" )
IF(${SERVER})
  ADD_SUBDIRECTORY( "src/policyxml" )
ENDIF()
ADD_SUBDIRECTORY( "src/renderlist" )
ADD_SUBDIRECTORY( "src/renderlistgl" )
ADD_SUBDIRECTORY( "src/jobobserver" )
IF(${DESKTOP})
  ADD_SUBDIRECTORY( "src/qtobserver" )
ENDIF()
IF(${SERVER})
  ADD_SUBDIRECTORY( "src/trell" )
  ADD_SUBDIRECTORY( "src/mod_trell" )
  ADD_SUBDIRECTORY( "src/trell_master" )
ENDIF()

# Unit tests
ADD_SUBDIRECTORY( "unittests/policy/" )
IF(${SERVER})
  ADD_SUBDIRECTORY( "unittests/policyxml/" )
ENDIF()
ADD_SUBDIRECTORY( "unittests/renderlist/" )
ADD_SUBDIRECTORY( "unittests/jobobserver/" )

# Examples
ADD_SUBDIRECTORY( "examples/rlview/" )
ADD_SUBDIRECTORY( "examples/simplejob" )


#SET( CPACK_DEB_COMPONENT_INSTALL ON)
SET( CPACK_GENERATOR "DEB" )
SET( CPACK_MONOLITHIC_INSTALL 1)
SET( CPACK_PACKAGE_VERSION ${version_number} )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
SET( CPACK_COMPONENTS_ALL 
  policylib
  policylib-dev 
  policylibxml
  policylibxml-dev
  jobobserver
  jobobserver-dev
  renderlist
  renderlist-dev
  renderlistgl
  renderlistgl-dev
  qtobserver
  qtobserver-dev
  libtrell
  libtrell-dev
  mod_trell
  )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
SET( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/postrm;" )
include ( CPack )