# Install script for directory: D:/v1.5/v1.5/Libs/PolyHook

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/v1.5/v1.5/Libs/PolyHook/out/install/x86-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/v1.5/v1.5/Libs/PolyHook/asmtk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/v1.5/v1.5/Libs/PolyHook/zydis/dependencies/zycore/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/v1.5/v1.5/Libs/PolyHook/zydis/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2" TYPE FILE FILES
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Enums.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/IHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Instruction.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Misc.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/UID.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/ErrorLog.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/MemProtector.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/MemAccessor.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/FBAllocator.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/RangeAllocator.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Tests/TestEffectTracker.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Tests/StackCanary.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/EventDispatcher.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/PolyHookOs.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/PolyHookOsIncludes.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2" TYPE FILE FILES "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/ZydisDisassembler.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2/Detour" TYPE FILE FILES
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Detour/ADetour.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Detour/NatDetour.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Detour/x64Detour.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Detour/x86Detour.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2/Detour" TYPE FILE FILES "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Detour/ILCallback.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2/Exceptions" TYPE FILE FILES
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Exceptions/AVehHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Exceptions/BreakPointHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Exceptions/HWBreakPointHook.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2/PE" TYPE FILE FILES
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/PE/EatHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/PE/IatHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/PE/PEB.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polyhook2/Virtuals" TYPE FILE FILES
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Virtuals/VTableSwapHook.hpp"
    "D:/v1.5/v1.5/Libs/PolyHook/polyhook2/Virtuals/VFuncSwapHook.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2" TYPE FILE FILES "D:/v1.5/v1.5/Libs/PolyHook/PolyHook_2-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/v1.5/v1.5/Libs/PolyHook/Debug/PolyHook_2.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2/PolyHook_2-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2/PolyHook_2-targets.cmake"
         "D:/v1.5/v1.5/Libs/PolyHook/CMakeFiles/Export/603899fb80b76890337322255e6aa6fc/PolyHook_2-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2/PolyHook_2-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2/PolyHook_2-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2" TYPE FILE FILES "D:/v1.5/v1.5/Libs/PolyHook/CMakeFiles/Export/603899fb80b76890337322255e6aa6fc/PolyHook_2-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/PolyHook_2" TYPE FILE FILES "D:/v1.5/v1.5/Libs/PolyHook/CMakeFiles/Export/603899fb80b76890337322255e6aa6fc/PolyHook_2-targets-debug.cmake")
  endif()
endif()

