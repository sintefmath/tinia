PROJECT( tinia )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
SET( version_number "0.2.5" )

# Set shared for linux:
IF(NOT WIN32)
    IF(NOT DEFINED ${BUILD_SHARED_LIBS})
        SET(BUILD_SHARED_LIBS ON)
    ENDIF()
ENDIF()

IF(NOT WIN32)
  OPTION( Tinia_SERVER "Build mod_tinia_trell and related components?" ON)
ELSE()
  OPTION( Tinia_SERVER "Build mod_tinia_trell and related components?" OFF)
ENDIF()

OPTION( Tinia_DESKTOP "Build qtcontroller?" ON )
OPTION( Tinia_BUILD_EXAMPLES "Build example jobs?" ON )
OPTION( Tinia_BUILD_TUTORIALS "Build tutorials?" ON )
OPTION( Tinia_BUILD_UNIT_TESTS "Build unit tests?" ON )

OPTION( Tinia_EXTEND_CMAKE_MODULE_PATH
        "Extend the CMAKE_MODULE_PATH variable with user directories?"
        ON )
MARK_AS_ADVANCED( Tinia_EXTEND_CMAKE_MODULE_PATH )

IF( Tinia_EXTEND_CMAKE_MODULE_PATH )
    SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                           ${CMAKE_SOURCE_DIR}/cmake/Modules )
ENDIF()



IF(WIN32)
  ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  SET( CMAKE_INSTALL_PREFIX "$ENV{PROGRAMFILES(x86)}/SINTEF/tinia" CACHE PATH "Path to install tinia")
  SET( Tinia_3RDPARTY_LOC $ENV{TINIA_3RDPARTY} CACHE PATH "Location of Tinia 3rdparty" )
  SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
	"${Tinia_3RDPARTY_LOC}" )
ENDIF()


IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET(CMAKE_CXX_FLAGS "-Wall -fPIC -std=c++0x ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
ENDIF()
IF(MSVC10)
        #Enable multiprocessor compilation for speed
        #SET(CMAKE_CXX_FLAGS " /MP ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
	#CAREFUL, this will append the above string for each time CMAKE is run!
ENDIF()


# We don't need XML support on Windows (only needed for mod_tinia_trell...)
IF(Tinia_SERVER)
    find_package( LibXml2 REQUIRED )
ELSE()
    find_package(LibXml2)
ENDIF()

IF(${LIBXML2_FOUND})

    ADD_DEFINITIONS(-DTINIA_HAVE_LIBXML)
ELSE()
    SET(LIBXML2_INCLUDE_DIRS "")
    SET(LIBXML2_INCLUDE_DIR "")

    SET(LIBXML2_LIBRARIES "")
ENDIF()

##### BOOST #####
set(BOOST_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE( Boost 1.46 COMPONENTS unit_test_framework prg_exec_monitor thread date_time system )

##### QT #####
IF(${Tinia_DESKTOP})
  FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXML QtScript QtNetwork REQUIRED)
  INCLUDE(${QT_USE_FILE})
  SET(QT_USE_QTOPENGL TRUE)
  SET(QT_USE_QTXML TRUE)
    SET(QT_USE_QTNETWORK TRUE)
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

IF(${Tinia_SERVER})

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
IF(${LIBXML2_FOUND})
  ADD_SUBDIRECTORY( "src/modelxml" )
ENDIF()
ADD_SUBDIRECTORY( "src/renderlist" )
ADD_SUBDIRECTORY( "src/renderlistgl" )
ADD_SUBDIRECTORY( "src/jobcontroller" )
IF(${Tinia_DESKTOP})
  ADD_SUBDIRECTORY( "js")
  ADD_SUBDIRECTORY( "src/qtcontroller" )

ENDIF()
IF(${Tinia_SERVER})
  ADD_SUBDIRECTORY( "src/trell" )
  ADD_SUBDIRECTORY( "src/mod_trell" )
  ADD_SUBDIRECTORY( "src/trell_master" )
ENDIF()

# Unit tests
IF( Tinia_BUILD_UNIT_TESTS )
  ADD_SUBDIRECTORY( "unittests/model/" )
  IF(${LIBXML2_FOUND})
    ADD_SUBDIRECTORY( "unittests/modelxml/" )
  ENDIF()
  ADD_SUBDIRECTORY( "unittests/renderlist/" )
  ADD_SUBDIRECTORY( "unittests/jobcontroller/" )
  IF(${Tinia_DESKTOP})
    ADD_SUBDIRECTORY( "unittests/qtcontroller/" )
  ENDIF()
  IF( ${LIBXML2_FOUND} AND ${Tinia_DESKTOP} )
    ADD_SUBDIRECTORY( "unittests/combined/" )
  ENDIF()
ENDIF()

# Examples
IF( Tinia_BUILD_EXAMPLES )
  ADD_SUBDIRECTORY( "examples/rlview/" )
  ADD_SUBDIRECTORY( "examples/simplejob" )
  IF(${Tinia_DESKTOP})
    ADD_SUBDIRECTORY( "examples/fpsviewer" )
  ENDIF()
  ADD_SUBDIRECTORY( "examples/cuberenderer")
  ADD_SUBDIRECTORY( "examples/changerestrictions")
  ADD_SUBDIRECTORY( "examples/clock_example")
ENDIF()


#Tutorials
IF( Tinia_BUILD_TUTORIALS )
  ADD_SUBDIRECTORY( "tutorials/tutorial1/")
  ADD_SUBDIRECTORY( "tutorials/tutorial2/")
  ADD_SUBDIRECTORY( "tutorials/tutorial3/")
  ADD_SUBDIRECTORY( "tutorials/tutorial4/")
  IF(${Tinia_DESKTOP})
    ADD_SUBDIRECTORY( "tutorials/tutorial5/")
  ENDIF()
ENDIF()

FILE(GLOB tinia_headerFiles "include/tinia/*.hpp")
install( FILES
	${tinia_headerFiles} 
    DESTINATION include/tinia/  
    COMPONENT tinia_dev)

#SET( CPACK_DEB_COMPONENT_INSTALL ON)

SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/agpl-3.0.txt)

SET(CPACK_PACKAGE_VENDOR "SINTEF ICT")

SET(CPACK_PACKAGE_VERSION ${version_number} )

IF(WIN32)
    SET( CPACK_GENERATOR "NSIS" )
    SET( CPACK_NSIS_CONTACT "kjetil.olsen.lye@sintef.no" )
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "tinia-${version_number}")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Akerselva")
    SET( CPACK_COMPONENTS_ALL 
         model
         model_dev   
         jobcontroller
         jobcontroller_dev
         renderlist
         renderlist_dev
         renderlistgl
         renderlistgl_dev
         qtcontroller
         qtcontroller_dev
		 tinia_dev
         #simple_qt_job
         #tinia_3rdparty 
    )
ELSE()
    SET( CPACK_GENERATOR "DEB" )
    SET( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/postrm;" )
    SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
    SET(CPACK_MONOLITHIC_INSTALL 1)
    SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
ENDIF()






include ( CPack )
