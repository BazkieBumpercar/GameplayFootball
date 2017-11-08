
cmake_minimum_required(VERSION 2.8)

set(PACKAGE gameplayfootball)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
if(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else(${VERSION_PATCH})
   set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
endif(${VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

project(gameplayfootball)

if(UNIX)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
endif(UNIX)

# Find needed libraries
FIND_PACKAGE(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

FIND_PACKAGE(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

FIND_PACKAGE(SDL_image REQUIRED)
include_directories(${SDL_IMAGE_DIRS})

FIND_PACKAGE(SDL_ttf REQUIRED)
include_directories(${SDL_TTF_DIRS})

FIND_PACKAGE(SDL_gfx REQUIRED)
include_directories(${SDL_GFX_DIRS})

FIND_PACKAGE(SDL_net REQUIRED)
include_directories(${SDL_NET_INCLUDE_DIRS})

FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread signals filesystem)
include_directories(${Boost_INCLUDE_DIR})

FIND_PACKAGE(SGE REQUIRED)
include_directories(${SGE_INCLUDE_DIR})

FIND_PACKAGE(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

FIND_PACKAGE(Blunted2 REQUIRED)
include_directories(${BLUNTED2_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src)

# Include the sources
include(sources.cmake)

add_library(leaguelib ${LEAGUE_SOURCES} ${LEAGUE_HEADERS})
add_library(gamelib ${GAME_SOURCES} ${GAME_HEADERS})
add_library(hidlib ${HID_SOURCES} ${HID_HEADERS})
add_library(menulib ${MENU_SOURCES} ${MENU_HEADERS})
add_library(datalib ${DATA_SOURCES} ${DATA_HEADERS})

set(LIBRARIES gamelib hidlib menulib datalib leaguelib 
   ${BLUNTED2_LIBRARY} ${Boost_LIBRARIES} 
   ${SDL_IMAGE_LIBRARIES} ${SDL_TTF_LIBRARIES} ${SDL_GFX_LIBRARIES}
   ${SDL_NET_LIBRARIES}
   ${SDL_LIBRARY} ${OPENAL_LIBRARY} ${SGE_LIBRARY} dl m ${OPENGL_LIBRARIES})

add_executable(gameplayfootball WIN32 ${CORE_SOURCES} ${CORE_HEADERS})
target_link_libraries(gameplayfootball ${LIBRARIES})

