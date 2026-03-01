include(FetchContent)
#set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/)
FetchContent_Declare(
  glfw3
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4 # glfw 3.4
)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
)

FetchContent_MakeAvailable(glfw3 imgui)
find_package(OpenGL REQUIRED)
set(ROOT ${imgui_SOURCE_DIR}) 
add_library(glad gl.c)
add_library(imgui STATIC
	${ROOT}/imgui.cpp
	${ROOT}/imgui.h
	${ROOT}/imstb_rectpack.h
	${ROOT}/imstb_textedit.h
	${ROOT}/imstb_truetype.h
    ${ROOT}/imgui_demo.cpp
    ${ROOT}/imgui_draw.cpp
    ${ROOT}/imgui_internal.h
    ${ROOT}/imgui_tables.cpp
    ${ROOT}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${ROOT}>
    $<INSTALL_INTERFACE:include>)

foreach(BACKEND glfw opengl3)
	# Construct the library name and header file path
	set(NAME imgui_impl_${BACKEND})
	set(HEADER ${ROOT}/backends/${NAME}.h)

	# Create a static library for this backend from its .cpp and .h files
	add_library(${NAME} STATIC ${ROOT}/backends/${NAME}.cpp ${HEADER})

	# Each backend library links publicly against the core imgui library
	target_link_libraries(${NAME} PUBLIC imgui)

	# Add the backends directory to the include paths for this backend library
	# This allows backend headers to include each other if needed
	target_include_directories(${NAME} PUBLIC
	    $<BUILD_INTERFACE:${ROOT}/backends>
	    $<INSTALL_INTERFACE:include>)

endforeach()
target_link_libraries(imgui_impl_glfw PRIVATE glfw)
target_sources(imgui_impl_opengl3 PRIVATE ${ROOT}/backends/imgui_impl_opengl3_loader.h)
target_link_libraries(imgui_impl_opengl3 PRIVATE OpenGL::GL)



target_include_directories(imgui PUBLIC ${ROOT}/backends)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(main main.cpp)
target_link_libraries(main glfw glad imgui imgui_impl_glfw imgui_impl_opengl3)
