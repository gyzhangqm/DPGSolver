# Define functions for use in cmake files.

# Print whether package was found or not.
function(print_found pkg_name flag_found)
	if(${flag_found})
		message("\tSearching for package: ${pkg_name} ... found.")
	else()
		message(WARNING "\tSearching for package: ${pkg_name} ... not found.")
	endif()
endfunction()

# Add a subdirectory and add it to the searched include directories.
function(add_and_include_directory dir_name)
	add_subdirectory(${dir_name})
	include_directories(${dir_name})
endfunction()


# Configure file_name.h.in files with cmake defines.
function(configure_file_h_in file_path file_name)
	set(ENV{PROJECT_ADDED_CMAKE_H} "YES")

	# Obtain file name without `.in` extension
	string(REGEX MATCH "[a-z_]+.[a-z]+" file_name_strip_in ${file_name})

	# Configure file for the build
	set(ENV{PROJECT_BUILD_INCLUDE_DIR} "${PROJECT_BINARY_DIR}/include")
	configure_file("${file_path}${file_name}"
	               "$ENV{PROJECT_BUILD_INCLUDE_DIR}/${file_name_strip_in}")
endfunction()


# Add include paths for cmake generated header files (if necessary).
function(add_include_directories)
	if ($ENV{PROJECT_ADDED_CMAKE_H})
		include_directories("$ENV{PROJECT_BUILD_INCLUDE_DIR}")
	endif()
endfunction()


# Add to environment variable (semi-colon separated list).
function(add_to_env env_var addition)
	set(ENV{${env_var}} "$ENV{${env_var}}${addition};")
endfunction()


# Add external project (download from github)
# Requires that a CMakeLists.txt file exists in EXTERNAL_SRC_ROOT
function(add_external_project proj_name)
	set(EXTERNAL_SRC_ROOT  "${CMAKE_SOURCE_DIR}/external/${proj_name}")
	set(EXTERNAL_BIN_ROOT  "${CMAKE_BINARY_DIR}/external/${proj_name}")
	set(EXTERNAL_SRC_DIR   "${EXTERNAL_BIN_ROOT}/src")
	set(EXTERNAL_BUILD_DIR "${EXTERNAL_BIN_ROOT}/build")
	set(EXTERNAL_DOWN_DIR  "${EXTERNAL_BIN_ROOT}/download")

	configure_file(${EXTERNAL_SRC_ROOT}/CMakeLists.txt.in
	               ${EXTERNAL_BIN_ROOT}/download/CMakeLists.txt)

	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	                RESULT_VARIABLE result
	                WORKING_DIRECTORY ${EXTERNAL_DOWN_DIR})
	if(result)
		message(FATAL_ERROR "CMake step for ${proj_name} failed: ${result}")
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} --build .
	                RESULT_VARIABLE result
	                WORKING_DIRECTORY ${EXTERNAL_DOWN_DIR})
	if(result)
		message(FATAL_ERROR "Build step for ${proj_name} failed: ${result}")
	endif()

	add_subdirectory(${EXTERNAL_BIN_ROOT}/src
	                 ${EXTERNAL_BIN_ROOT}/build)

	add_to_env(EXTRA_LIBS gtest_main)
endfunction()

# Print all include_directories from the current_source_dir
function (print_include_dirs)
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	foreach(dir ${dirs})
		message("dir='${dir}'")
	endforeach()
endfunction()
