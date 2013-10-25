cmake_minimum_required(VERSION 2.8)

# Set this to where you have your required Find*.cmake scripts. (GLEW and GLM)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/Modules")

# It's smart to have different options for building a server of dekstop version
IF(NOT WIN32)
       OPTION(SERVER "Build server version?" ON)
ELSE()
	OPTION(SERVER "Build server version?" OFF)
ENDIF()

OPTION(DESKTOP "Build desktop version?" ON)

# Tinia requires C++11
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET(CMAKE_CXX_FLAGS "-std=c++0x")
ENDIF()


IF(${SERVER})
  IF(${DESKTOP})
    FIND_PACKAGE(Tinia COMPONENTS Desktop Server)
  ELSE()
    FIND_PACKAGE(Tinia COMPONENTS Server)
  ENDIF()
ELSEIF(${DESKTOP})
  FIND_PACKAGE(Tinia COMPONENTS Desktop)
ENDIF()

INCLUDE_DIRECTORIES("./include" ${TINIA_INCLUDE_DIRS})

IF(${SERVER})
  ADD_EXECUTABLE(sample_project_web
	"src/main_web.cpp"
	"src/MyJob.cpp")
  TARGET_LINK_LIBRARIES(sample_project_web
	${TINIA_LIBRARIES})
ENDIF()

IF(${DESKTOP})
  ADD_EXECUTABLE(sample_project_desktop
	"src/main_desktop.cpp"
	"src/MyJob.cpp"
  )

  TARGET_LINK_LIBRARIES(sample_project_desktop
	${TINIA_LIBRARIES})
ENDIF()
