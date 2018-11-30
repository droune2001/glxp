set(GLEW_LIBRARY_DIR "$ENV{GLEW_DIR}/lib/Release/x64")
set(GLEW_BINARY_DIR  "$ENV{GLEW_DIR}/bin/Release/x64")
set(GLEW_DLLS        "${GLEW_BINARY_DIR}/glew32.dll")

# exported
set(GLEW_INCLUDE_DIRS "$ENV{GLEW_DIR}/include")
set(GLEW_LIBRARIES    "${GLEW_LIBRARY_DIR}/glew32.lib")

MESSAGE(STATUS ${GLEW_INCLUDE_DIRS})
MESSAGE(STATUS ${GLEW_LIBRARY_DIR})
MESSAGE(STATUS ${GLEW_LIBRARIES})
MESSAGE(STATUS ${GLEW_BINARY_DIR})
MESSAGE(STATUS ${GLEW_DLLS})
