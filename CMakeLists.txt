CMAKE_MINIMUM_REQUIRED (VERSION 3.2.2)
SET(PROJECT_NAME web-shoot)
PROJECT ( ${PROJECT_NAME} )

IF(MSVC)
ELSEIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -Wall -g -O0 -D_GNU_SOURCE")
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


ADD_EXECUTABLE(${PROJECT_NAME} background.cpp engine_deviantart.cpp engine_qwant.cpp engine_yandex.cpp main.cpp opengl.cpp common.cpp engine_file.cpp engine_test.cpp image_fifo.cpp misc.cpp debug.cpp engine_framabee.cpp engine_wikimedia.cpp engine_vacuum.cpp loader.cpp network.cpp WebPage.cpp)

TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${SDL_LIBRARIES} ${SDLIMAGE_LIBRARIES} ${OPENGL_LIBRARIES} readline)
