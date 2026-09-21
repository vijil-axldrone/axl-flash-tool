# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-src"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-build"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/tmp"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/src/esp-serial-flasher-populate-stamp"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/src"
  "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/src/esp-serial-flasher-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/src/esp-serial-flasher-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/vijil/Desktop/My Flash tools/axlflash/build/_deps/esp-serial-flasher-subbuild/esp-serial-flasher-populate-prefix/src/esp-serial-flasher-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
