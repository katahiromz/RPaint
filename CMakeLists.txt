# RPaint --- Independent ReactOS Paint

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# Set policy
cmake_policy(SET CMP0079 NEW)

# project name and languages
project(RPaint C CXX RC)

##############################################################################
# Architecture

if (MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions(-D_AMD64_ -D_M_AMD64)
    else()
        add_definitions(-D_X86_ -D_M_IX86)
    endif()
endif()

##############################################################################
# RATL

# option ENABLE_ATLTESTS
option(ENABLE_ATLTESTS "Enable ATL tests" OFF)

# RATL
add_subdirectory(RATL)

# get RATL variables
get_directory_property(RATL_DEFINITIONS DIRECTORY RATL DEFINITION RATL_DEFINITIONS)
get_directory_property(RATL_INCLUDE_DIRS DIRECTORY RATL DEFINITION RATL_INCLUDE_DIRS)
get_directory_property(RATL_LIBRARIES DIRECTORY RATL DEFINITION RATL_LIBRARIES)

##############################################################################
# mspaint.exe

add_executable(mspaint WIN32
    canvas.cpp
    dialogs.cpp
    dib.cpp
    drawing.cpp
    fullscreen.cpp
    history.cpp
    main.cpp
    miniature.cpp
    mouse.cpp
    palette.cpp
    palettemodel.cpp
    registry.cpp
    selectionmodel.cpp
    sizebox.cpp
    textedit.cpp
    toolbox.cpp
    toolsettings.cpp
    toolsmodel.cpp
    rsrc.rc)

target_compile_definitions(mspaint PRIVATE
    __RPAINT__
    UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
    -DUNREACHABLE=
    LANGUAGE_BG_BG
    LANGUAGE_CS_CZ
    LANGUAGE_DA_DK
    LANGUAGE_DE_DE
    LANGUAGE_EL_GR
    LANGUAGE_EN_US
    LANGUAGE_ES_ES
    LANGUAGE_ET_EE
    LANGUAGE_EU_ES
    LANGUAGE_FI_FI
    LANGUAGE_FR_FR
    LANGUAGE_HE_IL
    LANGUAGE_HI_IN
    LANGUAGE_HR_HR
    LANGUAGE_HU_HU
    LANGUAGE_HY_AM
    LANGUAGE_ID_ID
    LANGUAGE_IT_IT
    LANGUAGE_JA_JP
    LANGUAGE_KO_KR
    LANGUAGE_LT_LT
    LANGUAGE_MS_MY
    LANGUAGE_NB_NO
    LANGUAGE_NL_NL
    LANGUAGE_NO_NO
    LANGUAGE_PL_PL
    LANGUAGE_PT_BR
    LANGUAGE_PT_PT
    LANGUAGE_RO_RO
    LANGUAGE_RU_RU
    LANGUAGE_SK_SK
    LANGUAGE_SL_SI
    LANGUAGE_SQ_AL
    LANGUAGE_SV_SE
    LANGUAGE_TH_TH
    LANGUAGE_TR_TR
    LANGUAGE_UK_UA
    LANGUAGE_UZ_UZ
    LANGUAGE_ZH_CN
    LANGUAGE_ZH_HK
    LANGUAGE_ZH_TW
    ${RATL_DEFINITIONS})

if (NOT MSVC)
    target_link_options(mspaint BEFORE PRIVATE -municode)
endif()

target_include_directories(mspaint PRIVATE ./ reactos/ ${RATL_INCLUDE_DIRS})
target_link_libraries(mspaint PRIVATE user32 gdi32 comctl32 comdlg32 advapi32 shell32 msvcrt kernel32 ${RATL_LIBRARIES})

if (MSVC)
    # Use wWinMain
    target_link_options(mspaint PRIVATE "/entry:wWinMainCRTStartup")

    # No default manifest
    target_link_options(mspaint PRIVATE /MANIFEST:NO)
endif()

##############################################################################
