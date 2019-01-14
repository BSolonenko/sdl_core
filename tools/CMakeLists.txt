cmake_minimum_required(VERSION 2.8.3)

include(ExternalProject)

set(intergen_SOURCES "${CMAKE_CURRENT_LIST_DIR}/intergen")
set(intergen_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/intergen")
set(intergen_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/intergen")
set(CMAKE_CMD ${CMAKE_COMMAND})
set(intergen_CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_INSTALL_PREFIX=${intergen_INSTALL_DIR} -DCMAKE_CXX_FLAGS="-std=gnu++0x")

if (${CMAKE_CROSSCOMPILING})
  find_program(CMAKE_CMD NAMES cmake PATHS /bin /usr/bin /usr/share/bin NO_DEFAULT_PATH)
  find_program(C_COMPILER NAMES cc PATHS /bin /usr/bin /usr/share/bin NO_DEFAULT_PATH)
  find_program(CXX_COMPILER NAMES c++ PATHS /bin /usr/bin /usr/share/bin NO_DEFAULT_PATH)

  if (NOT CXX_COMPILER OR
      NOT C_COMPILER OR
      NOT CMAKE_CMD)
    message(SEND_ERROR "Default compiler cannot be found!")
  endif()

  # The following is necessary to overwrite environment variables CFLAGS and CXXFLAGS
  set(CMAKE_CMD CXXFLAGS="-std=gnu++11" ${CMAKE_CMD})

  set(intergen_CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_INSTALL_PREFIX=${intergen_INSTALL_DIR})

  mark_as_advanced(CMAKE_CMD C_COMPILER CXX_COMPILER)
endif()

ExternalProject_Add(intergen
                    SOURCE_DIR ${intergen_SOURCES}
                    PREFIX ${intergen_PREFIX}
                    INSTALL_DIR ${intergen_INSTALL_DIR}
                    CMAKE_COMMAND ${CMAKE_CMD}
                    CMAKE_ARGS ${intergen_CMAKE_ARGS})

# This will force intergen make call every time core is built
# When intergen source didn't change no files are updated
# It is required to automatically rebuild interfaces if intergen was changed
ExternalProject_Add_Step(intergen forcebuild
                         COMMAND ${CMAKE_COMMAND} -E echo "Force intergen build"
                         DEPENDEES update DEPENDERS build
                         ALWAYS 1)

if(BUILD_TESTS)
  add_subdirectory(intergen/test)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/run_tests.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/TestBin)
endif()
add_subdirectory(policy_table_validator)
