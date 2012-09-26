#Tries to find GLEW library
#
# Copyright <André Rigland Brodtkorb> Andre.Brodtkorb@sintef.no
#

#Find glew library
FIND_LIBRARY(GLEW_LIBRARY 
  NAMES GLEW glew glew32
  PATHS "/usr/lib"
  "/usr/lib64"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/VC/PlatformSDK/Lib"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 9.0/VC/lib/"
  "$ENV{PROGRAMW6432}/Microsoft SDKs/Windows/v6.0A/Lib"
  "~/mylibs/glew/lib"
  "$ENV{ProgramFiles}/Microsoft SDKs/Windows/v7.0A/Lib"
  "${GLEW_ROOT}/lib"
  )

#Find glew header
FIND_PATH(GLEW_INCLUDE_DIR "GL/glew.h"
  "/usr/include"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/VC/PlatformSDK/Include"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 9.0/VC/include/"
  "$ENV{PROGRAMW6432}/Microsoft SDKs/Windows/v6.0A/Include"
  "~/mylibs/glew/include"
  "$ENV{ProgramFiles}/Microsoft SDKs/Windows/v7.0A/Include"
  "${GLEW_ROOT}/include"
)

#check that we have found everything
SET(GLEW_FOUND "NO")
IF(GLEW_LIBRARY)
  IF(GLEW_INCLUDE_DIR)
    SET(GLEW_FOUND "YES")
  ENDIF(GLEW_INCLUDE_DIR)
ENDIF(GLEW_LIBRARY)

#Mark options as advanced
MARK_AS_ADVANCED(
  GLEW_INCLUDE_DIR
  GLEW_LIBRARY
)
