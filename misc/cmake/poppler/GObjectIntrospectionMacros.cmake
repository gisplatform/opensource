# Copyright (C) 2010, Pino Toscano, <pino@kde.org
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(${PROJECT_SOURCE_DIR}/../../misc/cmake/camel_to_lower.cmake)
macro(_gir_list_cammell_to_lower _outvar _listvar)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    string_camel_case_to_lower_case_with_hyphens(${_item} _item)
    list(APPEND ${_outvar} ${_item})
  endforeach(_item IN LISTS ${_listvar})
endmacro(_gir_list_cammell_to_lower)

macro(_gir_list_postfix _outvar _listvar _postfix)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    list(APPEND ${_outvar} ${_item}${_postfix})
  endforeach()
endmacro(_gir_list_postfix)

macro(_gir_list_prefix _outvar _listvar _prefix)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    list(APPEND ${_outvar} ${_prefix}${_item})
  endforeach()
endmacro(_gir_list_prefix)

macro(gir_add_introspections introspections_girs)

  if(NOT DEFINED EMBED_BUILD)
    message(-- GIR_NO_EMBED_BUILD)

    set(_gir_typelib_files)
    set(_gir_gir_files)

    foreach(gir IN LISTS ${introspections_girs})

      set(_gir_prefix ${gir}_gir)

      # SS-FIX
      set(REP ${PROJECT_SOURCE_DIR}/../girepository/)
      file(MAKE_DIRECTORY ${REP})
      set(METADATADIR ${PROJECT_SOURCE_DIR}/../metadata/)
      file(MAKE_DIRECTORY ${METADATADIR})

      set(INC ${PROJECT_SOURCE_DIR}/../include/)
      file(MAKE_DIRECTORY ${INC})

      # SS: Мажорная версия берется из Version.cmake.
      include(${PROJECT_SOURCE_DIR}/../../Version.cmake)
      set(_gir_version "${DUALIT_VERSION_MAJOR}.0")

      # _PROGRAM is an optional variable which needs it's own --program argument
      set(_gir_program "${${_gir_prefix}_PROGRAM}")
      if (NOT _gir_program STREQUAL "")
        set(_gir_program "--program=${_gir_program}")
      endif ()

      # Variables which provides a list of things
      _gir_list_prefix(_gir_libraries ${_gir_prefix}_LIBS "--library=")
      _gir_list_prefix(_gir_packages ${_gir_prefix}_PACKAGES "--pkg=")
      _gir_list_prefix(_gir_includes ${_gir_prefix}_INCLUDES "--include=")
      _gir_list_postfix(_gir_includes_my_basename ${_gir_prefix}_INCLUDES_MY ".gir")
      _gir_list_prefix(_gir_includes_my _gir_includes_my_basename "--include-uninstalled=${REP}")
      _gir_list_prefix(_gir_export_packages ${_gir_prefix}_EXPORT_PACKAGES "--pkg-export=")

      # Reuse the LIBTOOL variable from by automake if it's set
      set(_gir_libtool "--no-libtool")

      # 0) SS: Вытащим директории с хедерами и либами, затем подставим в INTROSPECTION_SCANNER.
      set(_dirs)
      set(_inc_dirs)
      set(_link_dirs)
      get_property(_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
      _gir_list_prefix(_inc_dirs _dirs "-I")
      get_property(_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
      _gir_list_prefix(_link_dirs _dirs "-L")
      if(MINGW)
        # Костыль для винды, ибо там так dll.a-файл лежит, в каталоге build.
        if("$ENV{LIBRARY_PATH}" STREQUAL "")
          set(_library_path "LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}")
        else()
          set(_library_path "LIBRARY_PATH=$ENV{LIBRARY_PATH}:${CMAKE_CURRENT_BINARY_DIR}")
        endif()
      endif(MINGW)


      # 0,5) SS: Сформируем абсолютные пути к хедерам и сорцам.
      _gir_list_prefix(_gir_abs_headers ${_gir_prefix}_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/")
      _gir_list_prefix(_gir_abs_sources ${_gir_prefix}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/")

      # 0,7) SS: Вытащим названия хедеров без путей и через запятую.
      set(_gir_headers_commas)
      foreach(_gir_header ${${_gir_prefix}_HEADERS})
        get_filename_component(_gir_header_basename ${_gir_header} NAME)
        if("${_gir_headers_commas}" STREQUAL "")
          set(_gir_headers_commas ${_gir_header_basename})
        else()
          set(_gir_headers_commas ${_gir_headers_commas},${_gir_header_basename})
        endif()
      endforeach()

      # 1) SS: Создаем gir.
      set(_gir_file ${REP}/${gir}-${_gir_version}.gir)
      add_custom_command(
        COMMAND PKG_CONFIG_PATH=$ENV{PKG_CONFIG_PATH} ${_library_path} ${INTROSPECTION_SCANNER}
                ${INTROSPECTION_SCANNER_ARGS}
                --c-include=${_gir_headers_commas}
                --warn-all
                --add-include-path=${REP}
                --namespace=${gir}
                --nsversion=${_gir_version}
                --identifier-prefix=${${_gir_prefix}_IDENTIFIER_PREFIXES}
                --symbol-prefix=${${_gir_prefix}_SYMBOL_PREFIXES}
                ${_gir_libtool}
                ${_gir_program}
                ${_gir_libraries}
                ${_gir_packages}
                ${_gir_includes}
                ${_gir_includes_my}
                ${_gir_export_packages}
                ${${_gir_prefix}_SCANNERFLAGS}
                ${${_gir_prefix}_CFLAGS}
                ${_inc_dirs}
                ${_link_dirs}
                ${_gir_abs_headers}
                ${_gir_abs_sources}
                --output ${_gir_file}
        DEPENDS ${${_gir_prefix}_HEADERS}
                ${${_gir_prefix}_SOURCES}
                ${${_gir_prefix}_LIBS}
        OUTPUT ${_gir_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
      )
      list(APPEND _gir_gir_filess ${_gir_file})

      # 2) SS: Генерим из gir файл typelib.
      set(_typelib_file ${REP}/${gir}-${_gir_version}.typelib)
      add_custom_command(
        COMMAND ${INTROSPECTION_COMPILER}
                ${INTROSPECTION_COMPILER_ARGS}
                --includedir=.
                --includedir=${REP}
                ${_gir_file}
                -o ${_typelib_file}
        DEPENDS ${_gir_file}
        OUTPUT ${_typelib_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      list(APPEND _gir_typelib_files ${_typelib_file})

      # 3) SS: Генерим из gir файл vapi.
      string_camel_case_to_lower_case_with_hyphens(${gir} _vapi)
      if("${VAPI_GEN_EXECUTABLE}" STREQUAL "")
        list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/elementary)
        find_package(VapiGen REQUIRED)
      endif("${VAPI_GEN_EXECUTABLE}" STREQUAL "")

      foreach(_item IN LISTS _gir_includes)
        if("${_item}" STREQUAL "--include=Gtk-2.0")
          set(USE_GTK2_VAPI --pkg=gtk+-2.0)
        endif("${_item}" STREQUAL "--include=Gtk-2.0")
      endforeach(_item IN LISTS _gir_includes)

      foreach(_item IN LISTS _gir_includes)
        if("${_item}" STREQUAL "--include=Gtk-3.0")
          set(USE_GTK3_VAPI --pkg=gtk+-3.0)
        endif("${_item}" STREQUAL "--include=Gtk-3.0")
      endforeach(_item IN LISTS _gir_includes)

      _gir_list_cammell_to_lower(_VAPI_PACKAGES ${_gir_prefix}_INCLUDES_MY)
      _gir_list_prefix(_VAPI_PACKAGES_INC _VAPI_PACKAGES "--pkg=")

      set(_vapi_file ${INC}/${_vapi}-${_gir_version})
      add_custom_command(
        COMMAND ${VAPI_GEN_EXECUTABLE}
                ${_gir_file}
                --library=${_vapi_file}
                --metadatadir=${METADATADIR}
                ${USE_GTK2_VAPI}
                ${USE_GTK3_VAPI}
                --vapidir=${INC}
                --pkg=gio-2.0
                ${_VAPI_PACKAGES_INC}
                --girdir=${REP}
                --girdir=/opt/gtk/share/gir-1.0/
        DEPENDS ${_gir_file}
        OUTPUT ${_vapi_file}.vapi
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      list(APPEND _gir_vapi_files ${_vapi_file}.vapi)

    endforeach()

    # Чтобы у каждого CMake в PROJECT/src/DIR/ была своя цель,
    # формируем имя цели на основе имени проекта и имени директории с CMake-файлом.
    get_filename_component(_target_prefix ${CMAKE_CURRENT_BINARY_DIR} NAME)
    set(_target_prefix "${PROJECT_NAME}-${_target_prefix}-gi")

    if (NOT TARGET ${_target_prefix}-girs)
      add_custom_target(${_target_prefix}-girs ALL DEPENDS ${_gir_gir_files})
    endif()
    if (NOT TARGET ${_target_prefix}-typelibs)
      add_custom_target(${_target_prefix}-typelibs ALL DEPENDS ${_gir_typelib_files})
    endif()
    if (NOT TARGET ${_target_prefix}-vapis)
      add_custom_target(${_target_prefix}-vapis ALL DEPENDS ${_gir_vapi_files})
    endif()

    # 0,9) SS: Очередной костыль для винды, иначе временный gir-бинарик падает за отсутсвием зависимостей.
    if(MINGW)
      add_custom_target(
        ${_target_prefix}-libs_symlinks_for_win_target
        COMMAND ln -Lfs ${PROJECT_SOURCE_DIR}/../lib*/bin/*.dll ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ln -Lfs ${PROJECT_SOURCE_DIR}/../mkl/lib/*.dll ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${${_gir_prefix}_LIBS}
      )
      add_dependencies(${_target_prefix}-girs ${_target_prefix}-libs_symlinks_for_win_target)
    endif(MINGW)

  endif(NOT DEFINED EMBED_BUILD)

endmacro(gir_add_introspections)

