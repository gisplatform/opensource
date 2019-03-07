
# Архитектура.

set(DUALIT_INTEL_32BIT FALSE)
set(DUALIT_INTEL_64BIT FALSE)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(i686)|(i386)|(I686)|(I386)")
  set(DUALIT_INTEL_32BIT TRUE)
  message("X32 WAS detected! Setting generic tune! (arch.cmake)")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  set(DUALIT_INTEL_64BIT TRUE)
  message("X86 WAS detected! Setting generic tune! (arch.cmake)")
else()
  message("X86 or X32 was NOT detected! NO generic tune! (arch.cmake)")
endif()

add_definitions(-DDUALIT_INTEL_32BIT=${DUALIT_INTEL_32BIT})
add_definitions(-DDUALIT_INTEL_64BIT=${DUALIT_INTEL_64BIT})


# Устанавливает generic-архитектуру процессора.

if(DUALIT_INTEL_64BIT OR DUALIT_INTEL_32BIT)

  if(CMAKE_COMPILER_IS_GNUCC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      if(DUALIT_INTEL_64BIT)        
        set(CMAKE_FLAGS_ARCH "${CMAKE_FLAGS_ARCH} -march=x86-64 -mtune=generic")
      elseif(CDUALIT_INTEL_32BIT)
        set(CMAKE_FLAGS_ARCH "${CMAKE_FLAGS_ARCH} -march=i386 -mtune=generic")        
      endif()
    else()
      set(CMAKE_FLAGS_ARCH "${CMAKE_FLAGS_ARCH} -march=pentium4")
    endif()

    set(CMAKE_C_FLAGS_OPTIM     "${CMAKE_C_FLAGS_OPTIM}     ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_C_FLAGS_PROFILE   "${CMAKE_C_FLAGS_PROFILE}   ${CMAKE_FLAGS_ARCH}")

    set(CMAKE_CXX_FLAGS_OPTIM   "${CMAKE_CXX_FLAGS_OPTIM}   ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${CMAKE_FLAGS_ARCH}")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${CMAKE_FLAGS_ARCH}")
  endif(CMAKE_COMPILER_IS_GNUCC)

endif()
