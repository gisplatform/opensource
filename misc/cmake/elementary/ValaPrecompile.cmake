##
# Copyright 2009-2010 Jakob Westhoff. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
# 
#    2. Redistributions in binary form must reproduce the above copyright notice,
#       this list of conditions and the following disclaimer in the documentation
#       and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY JAKOB WESTHOFF ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL JAKOB WESTHOFF OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies,
# either expressed or implied, of Jakob Westhoff
##

include(ParseArguments)
find_package(Vala REQUIRED)

##
# Compile vala files to their c equivalents for further processing. 
#
# The "vala_precompile" macro takes care of calling the valac executable on the
# given source to produce c files which can then be processed further using
# default cmake functions.
# 
# The first parameter provided is a variable, which will be filled with a list
# of c files outputted by the vala compiler. This list can than be used in
# conjuction with functions like "add_executable" or others to create the
# neccessary compile rules with CMake.
# 
# The initial variable is followed by a list of .vala files to be compiled.
# Please take care to add every vala file belonging to the currently compiled
# project or library as Vala will otherwise not be able to resolve all
# dependencies.
# 
# The following sections may be specified afterwards to provide certain options
# to the vala compiler:
# 
# PACKAGES
#   A list of vala packages/libraries to be used during the compile cycle. The
#   package names are exactly the same, as they would be passed to the valac
#   "--pkg=" option.
# 
# OPTIONS
#   A list of optional options to be passed to the valac executable. This can be
#   used to pass "--thread" for example to enable multi-threading support.
#
# CUSTOM_VAPIS
#   A list of custom vapi files to be included for compilation. This can be
#   useful to include freshly created vala libraries without having to install
#   them in the system.
#
# GENERATE_GIR
#   Pass gir name to generate.
#
# GENERATE_VALADOC
#   Pass package name to generate valadoc.
#
# GENERATE_VAPI
#   Pass all the needed flags to the compiler to create an internal vapi for
#   the compiled library. The provided name will be used for this and a
#   <provided_name>.vapi file will be created.
# 
# GENERATE_HEADER
#   Let the compiler generate a header file for the compiled code. There will
#   be a header file as well as an internal header file being generated called
#   <provided_name>.h and <provided_name>_internal.h
#
#
# The following call is a simple example to the vala_precompile macro showing
# an example to every of the optional sections:
#
#   vala_precompile(VALA_C
#       source1.vala
#       source2.vala
#       source3.vala
#   PACKAGES
#       gtk+-2.0
#       gio-1.0
#       posix
#   DIRECTORY
#       gen
#   OPTIONS
#       --thread
#   CUSTOM_VAPIS
#       some_vapi.vapi
#   GENERATE_GIR
#       MyPkg-2.0
#   GENERATE_VALADOC
#       my-pkg-2.0
#   GENERATE_VAPI
#       myvapi
#   GENERATE_HEADER
#       myheader
#   )
#
# Most important is the variable VALA_C which will contain all the generated c
# file names after the call.
##

macro(vala_precompile output)
    parse_arguments(ARGS "PACKAGES;OPTIONS;DIRECTORY;GENERATE_HEADER;GENERATE_VAPI;GENERATE_GIR;GENERATE_VALADOC;CUSTOM_VAPIS" "" ${ARGN})
    if(ARGS_DIRECTORY)
        set(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_DIRECTORY})
    else(ARGS_DIRECTORY)
        set(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif(ARGS_DIRECTORY)
    include_directories(${DIRECTORY})
    set(vala_pkg_opts "")
    foreach(pkg ${ARGS_PACKAGES})
        list(APPEND vala_pkg_opts "--pkg=${pkg}")
    endforeach(pkg ${ARGS_PACKAGES})
    set(in_files "")
    set(out_files "")
    set(${output} "")
    foreach(src ${ARGS_DEFAULT_ARGS})
        list(APPEND in_files "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        string(REPLACE ".vala" ".c" src ${src})
        string(REPLACE ".gs" ".c" src ${src})
        set(out_file "${DIRECTORY}/${src}")
        list(APPEND out_files "${DIRECTORY}/${src}")
        list(APPEND ${output} ${out_file})

        # SS-FIX: работаем только с предупреждениями vala-компилятора, gcc игнорим.
        set_property(SOURCE ${out_file} APPEND PROPERTY COMPILE_FLAGS "-w")
    endforeach(src ${ARGS_DEFAULT_ARGS})

    # SS-FIX
    set(DIRECTORY_FOR_INCLUDES ${PROJECT_SOURCE_DIR}/../../libs/include/)
    if(ARGS_GENERATE_VAPI OR ARGS_GENERATE_HEADER)
      file(MAKE_DIRECTORY ${DIRECTORY_FOR_INCLUDES})
    endif()

    set(custom_vapi_arguments "")
    if(ARGS_CUSTOM_VAPIS)
        foreach(vapi ${ARGS_CUSTOM_VAPIS})
            if(${vapi} MATCHES ${PROJECT_SOURCE_DIR})
                list(APPEND custom_vapi_arguments ${vapi})
            else (${vapi} MATCHES ${PROJECT_SOURCE_DIR})
                list(APPEND custom_vapi_arguments ${PROJECT_SOURCE_DIR}/../../libs/include/${vapi})
            endif(${vapi} MATCHES ${PROJECT_SOURCE_DIR})
        endforeach(vapi ${ARGS_CUSTOM_VAPIS})
    endif(ARGS_CUSTOM_VAPIS)

    # Мажорная версия берется из Version.cmake.
    include(${PROJECT_SOURCE_DIR}/../../Version.cmake)
    set(_vapi_version "${DUALIT_VERSION_MAJOR}.0")

    set(vapi_arguments "")
    if(ARGS_GENERATE_VAPI)
        list(APPEND out_files "${DIRECTORY_FOR_INCLUDES}/${ARGS_GENERATE_VAPI}-${_vapi_version}.vapi")
        set(vapi_arguments "--vapi=${DIRECTORY_FOR_INCLUDES}/${ARGS_GENERATE_VAPI}-${_vapi_version}.vapi")

        # Header and internal header is needed to generate internal vapi
        if (NOT ARGS_GENERATE_HEADER)
            set(ARGS_GENERATE_HEADER ${ARGS_GENERATE_VAPI})
        endif(NOT ARGS_GENERATE_HEADER)
    endif(ARGS_GENERATE_VAPI)

    set(header_arguments "")
    if(ARGS_GENERATE_HEADER)
        list(APPEND out_files "${DIRECTORY_FOR_INCLUDES}/${ARGS_GENERATE_HEADER}.h")
        list(APPEND out_files "${DIRECTORY}/${ARGS_GENERATE_HEADER}_internal.h")
        list(APPEND header_arguments "--header=${DIRECTORY_FOR_INCLUDES}/${ARGS_GENERATE_HEADER}.h")
        list(APPEND header_arguments "--internal-header=${DIRECTORY}/${ARGS_GENERATE_HEADER}_internal.h")
    endif(ARGS_GENERATE_HEADER)

    if(ARGS_GENERATE_GIR)
      set(_out_gir "--gir=${ARGS_GENERATE_GIR}.gir")
      set(_lib_name "--library=${ARGS_GENERATE_GIR}")
    endif(ARGS_GENERATE_GIR)

    if(ARGS_GENERATE_VALADOC)
      find_package(Valadoc)
    endif(ARGS_GENERATE_VALADOC)

    if(NOT GLIB2_TARGET_VERSION_MAJOR_AND_MINOR)
      message(FATAL_ERROR "glib.cmake hasn't been launched, but we need variable GLIB2_TARGET_VERSION_MAJOR_AND_MINOR.")
    endif(NOT GLIB2_TARGET_VERSION_MAJOR_AND_MINOR)

    # Чтобы глибовские G_OS_WIN32 и G_OS_UNIX работали и в препроцессоре Vala.
    if(UNIX)
      set(_os_type G_OS_UNIX)
    else(UNIX)
      if(WIN32)
        set(_os_type G_OS_WIN32)
      else(WIN32)
        set(_os_type G_OS_UNKNOWN)
      endif(WIN32)
    endif(UNIX)

    if(VALADOC_FOUND AND ARGS_GENERATE_VALADOC)
      add_custom_command(OUTPUT ${out_files}
        COMMAND 
            ${VALA_EXECUTABLE}
        ARGS 
            "--target-glib=${GLIB2_TARGET_VERSION_MAJOR_AND_MINOR}"
            "-C" 
            ${header_arguments} 
            ${vapi_arguments}
            "--vapidir" ${DIRECTORY_FOR_INCLUDES}
            "-b" ${CMAKE_CURRENT_SOURCE_DIR} 
            "-d" ${DIRECTORY} 
            "-D" ${_os_type} 
            ${_lib_name} 
            ${_out_gir} 
            ${vala_pkg_opts} 
            ${ARGS_OPTIONS} 
            ${in_files} 
            ${custom_vapi_arguments}
        COMMAND 
            ${VALADOC_EXECUTABLE} 
        ARGS 
            "--target-glib=${GLIB2_TARGET_VERSION_MAJOR_AND_MINOR}"
            "--package-name=${ARGS_GENERATE_VALADOC}"
            "--no-protected"
            "--force" "--verbose"
            "--vapidir" ${DIRECTORY_FOR_INCLUDES}
            ${vala_pkg_opts} 
            -o ${PROJECT_SOURCE_DIR}/doc/documentation
            ${in_files}
            ${custom_vapi_arguments}
        DEPENDS 
            ${in_files} 
            ${custom_vapi_arguments}
        COMMAND 
            ${VALADOC_EXECUTABLE} 
        ARGS 
            "--target-glib=${GLIB2_TARGET_VERSION_MAJOR_AND_MINOR}"
            "--package-name=${ARGS_GENERATE_VALADOC}-internal"
            "--internal"
            "--private"
            "--force" "--verbose"
            "--vapidir" ${DIRECTORY_FOR_INCLUDES}
            ${vala_pkg_opts} 
            -o ${PROJECT_SOURCE_DIR}/doc/documentation
            ${in_files}
            ${custom_vapi_arguments}
        DEPENDS 
            ${in_files} 
            ${custom_vapi_arguments}
        # Костыль, чтобы можно было собирать C++-компилятором -->
        COMMAND
          sed
        ARGS
            -i '0,/\#define/s//G_BEGIN_DECLS\\n\#define/' ${${output}}
        COMMAND
          sed
        ARGS
            -i '$$s/$$/\\nG_END_DECLS/' ${${output}}
        # Костыль, чтобы можно было собирать C++-компилятором <--
      )
    else(VALADOC_FOUND AND ARGS_GENERATE_VALADOC)
      add_custom_command(OUTPUT ${out_files}
        COMMAND 
            ${VALA_EXECUTABLE}
        ARGS 
            "--target-glib=${GLIB2_TARGET_VERSION_MAJOR_AND_MINOR}"
            "-C" 
            ${header_arguments} 
            ${vapi_arguments}
            "--vapidir" ${DIRECTORY_FOR_INCLUDES}
            "-b" ${CMAKE_CURRENT_SOURCE_DIR} 
            "-d" ${DIRECTORY} 
            "-D" ${_os_type} 
            ${_lib_name} 
            ${_out_gir} 
            ${vala_pkg_opts} 
            ${ARGS_OPTIONS} 
            ${in_files} 
            ${custom_vapi_arguments}
        DEPENDS 
            ${in_files} 
            ${custom_vapi_arguments}
        # Костыль, чтобы можно было собирать C++-компилятором -->
        COMMAND
          sed
        ARGS
            -i '0,/\#define/s//G_BEGIN_DECLS\\n\#define/' ${${output}}
        COMMAND
          sed
        ARGS
            -i '$$s/$$/\\nG_END_DECLS/' ${${output}}
        # Костыль, чтобы можно было собирать C++-компилятором <--
        )
    endif(VALADOC_FOUND AND ARGS_GENERATE_VALADOC)
endmacro(vala_precompile)

# Экспериментальный код -- им будем копировать в girepository gir-файл и генерить typelib.
macro(vala_postcompile)
    parse_arguments(ARGS "GIR_NAME;GIR_VER;DIRECTORY;" "" ${ARGN})
    if(ARGS_DIRECTORY)
        set(_GIR_WORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_DIRECTORY})
    else(ARGS_DIRECTORY)
        set(_GIR_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif(ARGS_DIRECTORY)

    set(DIRECTORY_FOR_GIRS ${PROJECT_SOURCE_DIR}/../../libs/girepository/)
    set(_VAPI_DIR ${PROJECT_SOURCE_DIR}/../../libs/include/)

    file(MAKE_DIRECTORY ${DIRECTORY_FOR_GIRS})

    include(${PROJECT_SOURCE_DIR}/../../misc/cmake/camel_to_lower.cmake)
    string_camel_case_to_lower_case_with_hyphens(${ARGS_GIR_NAME} _VAPI_NAME)

    add_custom_command(TARGET ${ARGS_DEFAULT_ARGS}
      POST_BUILD
      COMMAND
        echo
      ARGS
        ValaPostcompile for ${ARGS_GIR_NAME}
      COMMAND
        # Костыль, т.к. g-ir-compiler не воспринимает package.
        sed
      ARGS
          -i '/^<package/d' ${DIRECTORY}/${ARGS_GIR_NAME}-${ARGS_GIR_VER}.gir
      COMMAND
        # Костыль, т.к. g-ir-compiler не воспринимает тег annotation.
        sed
      ARGS
          -i '/^\t<annotation/d' ${DIRECTORY}/${ARGS_GIR_NAME}-${ARGS_GIR_VER}.gir
      COMMAND
        # Костыль, т.к. g-ir-compiler не воспринимает тег attribute.
        sed
      ARGS
          -i '/^\t<attribute/d' ${DIRECTORY}/${ARGS_GIR_NAME}-${ARGS_GIR_VER}.gir
      COMMAND
        # Костыль, т.к. g-ir-compiler воспринимает c:identifier-prefixe, а не c:prefix.
        sed
      ARGS
          -i -e 's/c:prefix/c:identifier-prefixes/' ${DIRECTORY}/${ARGS_GIR_NAME}-${ARGS_GIR_VER}.gir
      COMMAND
        # Удаляем старый CCode из vapi, если есть.
        sed
      ARGS
          -i '/^\\[CCode/d' ${_VAPI_DIR}/${_VAPI_NAME}-${ARGS_GIR_VER}.vapi
      COMMAND
        # Вставляем новый хороший CCode в vapi
        sed
      ARGS
          -i -e 's/^namespace/[CCode ( gir_namespace\ =\ \"${ARGS_GIR_NAME}\",\ gir_version\ =\ \"2.0\") ]\\n&/' ${_VAPI_DIR}/${_VAPI_NAME}-${ARGS_GIR_VER}.vapi
      COMMAND 
          cmake -E copy
      ARGS
          ${_GIR_WORK_DIR}/${ARGS_GIR_NAME}-${ARGS_GIR_VER}.gir
          ${DIRECTORY_FOR_GIRS}
      #COMMAND 
      #    g-ir-compiler
      #ARGS
      #    "--shared-library=${ARGS_DEFAULT_ARGS}"
      #    "${DIRECTORY}/${ARGS_GIR_NAME}.gir"
      #    "-o" "${DIRECTORY_FOR_GIRS}/${ARGS_GIR_NAME}.typelib"
      )
endmacro(vala_postcompile)

