# Translations.cmake, CMake macros written for Marlin, feel free to re-use them

macro(add_translations)
  set(NLS_PACKAGE ${PROJECT_NAME})

  if (NOT TARGET ${NLS_PACKAGE}-i18n)
    add_custom_target(${NLS_PACKAGE}-i18n ALL)
    find_program(MSGFMT_EXECUTABLE msgfmt)

    # SS -->
      find_program(MSGMERGE_EXECUTABLE msgmerge)
      # TODO: Для каждого языка вручную сначала что-то вроде:
      # msginit --locale=ru --input=PACKAGE.pot,
      # чтобы сгенерить первоначальный po.
    # SS <--

    file(GLOB PO_FILES ${PROJECT_SOURCE_DIR}/po/*.po)
    foreach (PO_INPUT ${PO_FILES})
      get_filename_component (PO_INPUT_BASE ${PO_INPUT} NAME_WE)

      # SS -->
        # Перегенерим сами po, т.к. у elementary это делает ланчпад.
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND ${MSGMERGE_EXECUTABLE} --lang=${PO_INPUT_BASE} ${PO_INPUT} ${PROJECT_SOURCE_DIR}/po/${NLS_PACKAGE}.pot -o ${PO_INPUT})
      # SS <--

      # SS -->
        # Пишем в share/locale mo-файлы на каталог выше каталога с исходниками, устанавливать будем сами вручную, не здесь.
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/../../share/locale/${PO_INPUT_BASE}/LC_MESSAGES/)
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND rm -f ${PROJECT_SOURCE_DIR}/../../share/locale/${PO_INPUT_BASE}/LC_MESSAGES/${NLS_PACKAGE}.mo)
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND ${MSGFMT_EXECUTABLE} -o ${PROJECT_SOURCE_DIR}/../../share/locale/${PO_INPUT_BASE}/LC_MESSAGES/${NLS_PACKAGE}.mo ${PO_INPUT})
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND rm -rf share WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
        add_custom_command(TARGET ${NLS_PACKAGE}-i18n COMMAND ln -sf ../../share share WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

        #set (MO_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PO_INPUT_BASE}.mo)
        #add_custom_command (TARGET ${NLS_PACKAGE}-i18n COMMAND ${MSGFMT_EXECUTABLE} -o ${MO_OUTPUT} ${PO_INPUT})
        #install (FILES ${MO_OUTPUT} DESTINATION
        #    share/locale/${PO_INPUT_BASE}/LC_MESSAGES
        #    RENAME ${NLS_PACKAGE}.mo)
      # SS <--
    endforeach (PO_INPUT ${PO_FILES})
  endif()

  if(NOT TARGET ${NLS_PACKAGE}-pot)
    add_custom_target(${NLS_PACKAGE}-pot)
    find_program (XGETTEXT_EXECUTABLE xgettext)

    set(SOURCE "")

    foreach(FILES_INPUT ${ARGN})
      file (GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${FILES_INPUT}/*.c)
      foreach(ONE_FILE ${SOURCE_FILES})
        set(SOURCE ${SOURCE} ${ONE_FILE})
      endforeach()
      file (GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${FILES_INPUT}/*.h)
      foreach(ONE_FILE ${HEADER_FILES})
        set(SOURCE ${SOURCE} ${ONE_FILE})
      endforeach()
      file (GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${FILES_INPUT}/*.vala)
      foreach(ONE_FILE ${SOURCE_FILES})
        set(SOURCE ${SOURCE} ${ONE_FILE})
      endforeach()
      file (GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${FILES_INPUT}/*.ui)
      foreach(ONE_FILE ${SOURCE_FILES})
        set(SOURCE ${SOURCE} ${ONE_FILE})
      endforeach()
    endforeach()

    add_custom_command (TARGET ${NLS_PACKAGE}-pot
      COMMAND
        ${XGETTEXT_EXECUTABLE} --keyword=C_:1c,2  -d ${NLS_PACKAGE} -o ${PROJECT_SOURCE_DIR}/po/${NLS_PACKAGE}.pot
        ${SOURCE} --keyword="_" --keyword="N_" --from-code=UTF-8
        # SS -->
          COMMAND
            # В ссылках на исходники вырезаем полный путь (паттерн -- аналог basename).
            sed -i s=^\#:.*/=\#:\ = ${PROJECT_SOURCE_DIR}/po/${NLS_PACKAGE}.pot
          COMMAND
            # В ссылках на исходники вырезаем номера строк.
            sed -i s=:[[:digit:]]\\\\\\+\$$== ${PROJECT_SOURCE_DIR}/po/${NLS_PACKAGE}.pot
        # SS -->
      )
  endif()
endmacro()
