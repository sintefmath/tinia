# - FindGLM
#
# Author: Andre.Brodtkorb@sintef.no
#
# This module looks for the OpenGL Mathematics library
# It will define the following variable(s)
#  GLM_INCLUDE_DIRS = where glm/glm.hpp can be found

find_path(GLM_INCLUDE_DIR
	NAMES "glm/glm.hpp"
	HINTS
	"~/mylibs/glm"
	"/usr/local/include/glm"
        "$ENV{PROGRAMFILES(x86)}/glm/include" # This is the default installation path    
)

if(NOT GLM_INCLUDE_DIR OR GLM_INCLUDE_DIR MATCHES "NOTFOUND")
	set(GLM_FOUND false)
	if(NOT GLM_QUIET)
		message( STATUS "GLM was not found")
	elseif(GLM_REQUIRED)
		message( SEND_ERROR "GLM was not found")
	endif()
else()
	set(GLM_FOUND true)
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
endif()
