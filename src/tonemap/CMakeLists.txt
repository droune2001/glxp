# TODO: find the var for "current sub directory"
set(CURRENT_TARGET tonemap)

set(SHADER_SOURCE_DIR "${ASSETS_DIR}/${CURRENT_TARGET}/shaders")

file( GLOB CURRENT_TARGET_SOURCES "*.c*" )
file( GLOB CURRENT_TARGET_HEADERS "*.h*" )
file( GLOB CURRENT_TARGET_SHADERS 
   "${SHADER_SOURCE_DIR}/*.frag"
   "${SHADER_SOURCE_DIR}/*.vert"
   "${SHADER_SOURCE_DIR}/*.comp")
   
source_group( "Common"  FILES ${COMMON_SOURCES} ${COMMON_HEADERS})
source_group( "Sources" FILES ${CURRENT_TARGET_SOURCES} )
source_group( "Headers" FILES ${CURRENT_TARGET_HEADERS} )
source_group( "Shaders" FILES ${CURRENT_TARGET_SHADERS} )

add_executable(${CURRENT_TARGET}
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
    ${CURRENT_TARGET_SOURCES} 
    ${CURRENT_TARGET_HEADERS} 
    ${CURRENT_TARGET_SHADERS})

target_link_libraries(${CURRENT_TARGET} 
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    OpenGL32.lib)
