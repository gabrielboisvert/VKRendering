# set the project name
get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
set(PROJECT_NAME ${CURRENT_FOLDER_NAME})


###############################
#                             #
# Sources                     #
#                             #
###############################

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c++)
	
# Add header files
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Header)
file(GLOB_RECURSE HEADER_FILES 
	${PROJECT_INCLUDE_DIR}/*.h
	${PROJECT_INCLUDE_DIR}/*.hpp
	${PROJECT_INCLUDE_DIR}/*.inl)
	
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES} ${SOURCE_FILES})


###############################
#                             #
# Executable                  #
#                             #
###############################

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIR})