CMAKE_MINIMUM_REQUIRED (VERSION 3.2.2)
SET(PROJECT_NAME web-shoot)
PROJECT ( ${PROJECT_NAME} C )

IF(MSVC)
ELSEIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0 -D_GNU_SOURCE")
ENDIF()

#Add pthread
FIND_PACKAGE (Threads)

FIND_PACKAGE (CURL)

FIND_PACKAGE(OpenGL REQUIRED)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL REQUIRED sdl)
PKG_SEARCH_MODULE(SDLIMAGE REQUIRED SDL_image)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIRS} ${SDLIMAGE_INCLUDE_DIR})

include_directories(/usr/include/readline)


ADD_EXECUTABLE(${PROJECT_NAME} background.c engine_deviantart.c engine_qwant.c engine_yandex.c main.c opengl.c common.c engine_file.c engine_test.c image_fifo.c misc.c debug.c engine_framabee.c engine_wikimedia.c engine_vacuum.c loader.c network.c)

TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${SDL_LIBRARIES} ${SDLIMAGE_LIBRARIES} ${OPENGL_LIBRARIES} readline)
