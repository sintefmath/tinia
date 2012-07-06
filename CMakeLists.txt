PROJECT( tinia )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
# Set shared for linux:
IF(NOT WIN32)
    IF(NOT DEFINED ${BUILD_SHARED_LIBS})
        SET(BUILD_SHARED_LIBS ON)
    ENDIF()
ENDIF()

IF(NOT WIN32)
  OPTION( SERVER "Build mod_trell and related components?" ON)
ELSE()
  OPTION( SERVER "Build mod_trell and related components?" OFF)
ENDIF()

OPTION( DESKTOP "Build qtcontroller?" ON)


OPTION( EXTEND_CMAKE_MODULE_PATH
        "Extend the CMAKE_MODULE_PATH variable with user directories?"
        ON )
IF( EXTEND_CMAKE_MODULE_PATH )
    SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                           ${CMAKE_SOURCE_DIR}/cmake/Modules
                           "~/cmake/Modules"
                           #"$ENV{HOMEPATH}/cmake/Modules"
                           ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules
                           "C:/cmake/Modules"
                           "../siut/cmakemacros"
                           "../siut"
                           "."
    )


IF(WIN32)
  ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  SET(CMAKE_INSTALL_PREFIX CACHE INTERNAL "")
  SET(CMAKE_PREFIX_PATH 
	"$ENV{TINIA_3RDPARTY}/glm;$ENV{TINIA_3RDPARTY}/glew;$ENV{TINIA_3RDPARTY}/boost" )
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
        SET(CMAKE_CXX_FLAGS " -DDEBUG -g3 -Wall -fPIC -std=c++0x ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
ENDIF()
IF(MSVC10)
        #Enable multiprocessor compilation for speed
        #SET(CMAKE_CXX_FLAGS " /MP ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
	#CAREFUL, this will append the above string for each time CMAKE is run!
ENDIF()


# We don't need XML support on Windows (only needed for mod_trell...)
IF(NOT WIN32)
  find_package( LibXml2 REQUIRED )
ENDIF()

##### BOOST #####
set(BOOST_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE( Boost 1.46 COMPONENTS unit_test_framework prg_exec_monitor thread date_time system )

##### QT #####
IF(${DESKTOP})
  FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXML QtScript REQUIRED)
  INCLUDE(${QT_USE_FILE})
  SET(QT_USE_QTOPENGL TRUE)
  SET(QT_USE_QTXML TRUE)
  SET(QT_USE_QTSCRIPT TRUE)
  QT4_WRAP_CPP(qtcontroller_HEADERS_MOC ${qtcontroller_SOURCES_TO_BE_MOCED} ${qtcontroller_SOURCES})
  QT4_WRAP_UI(qtcontroller_FORMS_HEADERS ${qtcontroller_FORMS})
  ADD_DEFINITIONS(${QT_DEFINITIONS})
  ADD_DEFINITIONS(-DQT_SHARED)
ENDIF()


##### OPENGL #####
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( GLM REQUIRED )

IF(${SERVER})

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
  ${Boost_INCLUDE_DIRS}
)

link_directories(
  ${Boost_LIBRARY_DIRS}
)

ADD_SUBDIRECTORY( "src/model" )
IF(${SERVER})
  ADD_SUBDIRECTORY( "src/modelxml" )
ENDIF()
ADD_SUBDIRECTORY( "src/renderlist" )
ADD_SUBDIRECTORY( "src/renderlistgl" )
ADD_SUBDIRECTORY( "src/jobcontroller" )
IF(${DESKTOP})
  ADD_SUBDIRECTORY( "src/qtcontroller" )
ENDIF()
IF(${SERVER})
  ADD_SUBDIRECTORY( "src/trell" )
  ADD_SUBDIRECTORY( "src/mod_trell" )
  ADD_SUBDIRECTORY( "src/trell_master" )
ENDIF()

# Unit tests
ADD_SUBDIRECTORY( "unittests/model/" )
IF(${SERVER})
  ADD_SUBDIRECTORY( "unittests/modelxml/" )
ENDIF()
ADD_SUBDIRECTORY( "unittests/renderlist/" )
ADD_SUBDIRECTORY( "unittests/jobcontroller/" )
IF(${DESKTOP})
   ADD_SUBDIRECTORY( "unittests/qtcontroller/" )
ENDIF()

# Examples
ADD_SUBDIRECTORY( "examples/rlview/" )
ADD_SUBDIRECTORY( "examples/simplejob" )


#SET( CPACK_DEB_COMPONENT_INSTALL ON)
SET( CPACK_GENERATOR "DEB" )
SET( CPACK_MONOLITHIC_INSTALL 1)
SET( CPACK_PACKAGE_VERSION ${version_number} )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
SET( CPACK_COMPONENTS_ALL 
  model
  model-dev 
  modelxml
  modelxml-dev
  jobcontroller
  jobcontroller-dev
  renderlist
  renderlist-dev
  renderlistgl
  renderlistgl-dev
  qtcontroller
  qtcontroller-dev
  libtrell
  libtrell-dev
  mod_trell
  )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
SET( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/postrm;" )
include ( CPack )
