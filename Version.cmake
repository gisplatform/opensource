# Версия всего ПО в репозитории.
# Правится с помощью inc_ver.sh или прямо здесь вручную.

set(DUALIT_VERSION_MAJOR "2")
set(DUALIT_VERSION_MINOR "6")
set(DUALIT_VERSION_PATCH "8")

# Хэш коммита.
execute_process(
  COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE DUALIT_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Имя ветки (если есть знаки "-" в имени ветки, берется только текст после "-").
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE DUALIT_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if("${DUALIT_GIT_BRANCH}" STREQUAL "")
  set(DUALIT_GIT_BRANCH "standalone")
else("${DUALIT_GIT_BRANCH}" STREQUAL "")
  string(REGEX REPLACE ".*-" "" DUALIT_GIT_BRANCH ${DUALIT_GIT_BRANCH})
endif("${DUALIT_GIT_BRANCH}" STREQUAL "")

# Есть ли не закоммиченные файлы ("грязный" ли гит?).
execute_process(
  COMMAND git status --porcelain apps libs misc share utils vapi
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../
    OUTPUT_VARIABLE DUALIT_GIT_DIRTY
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT DUALIT_GIT_DIRTY STREQUAL "")
  set(DUALIT_GIT_DIRTY ".dirty")
endif(NOT DUALIT_GIT_DIRTY STREQUAL "")

# Пример использования DUALIT_INFO : g_print("%s\r\n", DUALIT_INFO);
set(DUALIT_INFO ${CMAKE_PROJECT_NAME}_${DUALIT_GIT_BRANCH}-${DUALIT_VERSION_MAJOR}.${DUALIT_VERSION_MINOR}.${DUALIT_VERSION_PATCH}.hash:${DUALIT_GIT_HASH}${DUALIT_GIT_DIRTY})

add_definitions(-DDUALIT_VERSION_MAJOR=${DUALIT_VERSION_MAJOR})
add_definitions(-DDUALIT_VERSION_MINOR=${DUALIT_VERSION_MINOR})
add_definitions(-DDUALIT_VERSION_PATCH=${DUALIT_VERSION_PATCH})

add_definitions(-DDUALIT_GIT_HASH=\"${DUALIT_GIT_HASH}\")
add_definitions(-DDUALIT_GIT_BRANCH=\"${DUALIT_GIT_BRANCH}\")
add_definitions(-DDUALIT_INFO=\"${DUALIT_INFO}\")


# Version.cmake is a usefull cmake-file.
#
# Copyright 2015 Sergey Volkhin.
#
# Version.cmake is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Version.cmake is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Version.cmake. If not, see <http://www.gnu.org/licenses/>.

