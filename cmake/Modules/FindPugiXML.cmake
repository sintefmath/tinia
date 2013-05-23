include(FindPackageHandleStandardArgs)
#Tries to find the PUGIXML library

#Find glew library
FIND_LIBRARY(PUGIXML_LIBRARY
  NAMES pugixml
  PATHS "/usr/lib"
  "/usr/lib64"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/VC/PlatformSDK/Lib"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 9.0/VC/lib/"
  "$ENV{PROGRAMW6432}/Microsoft SDKs/Windows/v6.0A/Lib"
  "~/local/lib"
  "$ENV{ProgramFiles}/Microsoft SDKs/Windows/v7.0A/Lib"
  )

#Find glew header
FIND_PATH(PUGIXML_INCLUDE_DIR "pugixml.hpp"
  "/usr/include"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/VC/PlatformSDK/Include"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 9.0/VC/include/"
  "$ENV{PROGRAMW6432}/Microsoft SDKs/Windows/v6.0A/Include"
  "~/local/include"
  "$ENV{ProgramFiles}/Microsoft SDKs/Windows/v7.0A/Include"
)

 find_package_handle_standard_args(PugiXML DEFAULT_MSG
    PUGIXML_INCLUDE_DIR
    PUGIXML_LIBRARY)
