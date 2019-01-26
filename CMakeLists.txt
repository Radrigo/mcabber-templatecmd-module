## Copyright 2009-2012 Myhailo Danylenko
# This file is part of mcabber-templatecmd
#
# mcabber-templatecmd is a free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, either version
# 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6) 
project(templatecmd C) 
set(PROJECT_VERSION "0.0.1")

## User settable options
 
## Check for build dependencies
find_package(PkgConfig REQUIRED) 
pkg_check_modules(GLIB REQUIRED glib-2.0) 
pkg_check_modules(MCABBER REQUIRED mcabber)
link_directories(${GLIB_LIBRARY_DIRS}
				 ${MCABBER_LIBRARY_DIRS})

## Target definitions
add_library(templatecmd MODULE templatecmd.c) 

## Compiler setup
configure_file(config.h.in config.h)
include_directories(SYSTEM ${GLIB_INCLUDE_DIRS} 
                    ${MCABBER_INCLUDE_DIRS})
target_link_libraries(templatecmd ${GLIB_LIBRARIES} 
					  ${MCABBER_LIBRARIES})
include_directories(${templatecmd_SOURCE_DIR} 
                    ${templatecmd_BINARY_DIR})
set_target_properties(templatecmd PROPERTIES COMPILE_FLAGS "-Wall")

## Packaging information
set(CPACK_PACKAGE_NAME libmcabber-templatecmd)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "IsBear")
set(CPACK_PACKAGE_CONTACT "Myhailo Danylenko <isbear@ukrpost.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Temlate commands for mcabber")
set(CPACK_RESOURCE_FILE_LICENSE ${templatecmd_SOURCE_DIR}/COPYING)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_GENERATOR DEB CACHE TEXT "Binary package generator, eg DEB, RPM, TGZ, NSIS...")
set(CPACK_DEBIAN_PACKAGE_SECTION libs)
find_program(DPKG_EXECUTABLE dpkg)
if(DPKG_EXECUTABLE)
	execute_process(COMMAND ${DPKG_EXECUTABLE} --print-architecture OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386 CACHE TEXT "Architecture of package")
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\..*;\\\\.swp;~$;/build/;\\\\.tar\\\\.;\\\\.deb;\\\\.so")
include(CPack)

configure_file(templatecmd.avv.in templatecmd.avv)

## Installation
install(TARGETS templatecmd DESTINATION lib/mcabber) 
install(FILES docs/templatecmd.rc TODO docs/readme.mdwn COPYING DESTINATION share/doc/${CPACK_PACKAGE_NAME})
install(DIRECTORY help DESTINATION share/mcabber)
install(FILES ${PROJECT_BINARY_DIR}/templatecmd.avv DESTINATION share/mcabber/avv/modules RENAME templatecmd)

## The End ## vim: se ts=4: ##
