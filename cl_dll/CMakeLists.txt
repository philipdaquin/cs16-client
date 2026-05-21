cmake_minimum_required(VERSION 3.10)
project(client)

set(CLIENT_LIB client)

file(GLOB CS_CLIENT_SRC "*.cpp")
file(GLOB CS_EV_SRC "events/*.cpp")
file(GLOB CS_VGUI2_SRC "vgui2/*.cpp")
file(GLOB CS_VGUI2_GAMECTRL_SRC "vgui2/game_controls/*.cpp")
file(GLOB_RECURSE CS_VGUI2_CSMOE_SRC "vgui2/csmoe/*.cpp")
file(GLOB_RECURSE CS_VGUI2_CSMOE_ZOMBIE_SRC
	"vgui2/csmoe/zsh/*.cpp"
	"vgui2/csmoe/zb2/*.cpp")
file(GLOB CS_WPN_SRC "cs_wpn/*.cpp")
file(GLOB CS_WPNSH_SRC "../dlls/wpn_shared/*.cpp")
file(GLOB CS_HUD_SRC "hud/*.cpp")
file(GLOB CS_STUDIO_SRC "studio/*.cpp")
file(GLOB CS_INPUT_SRC "input/*.cpp")
file(GLOB CS_PM_SRC "../pm_shared/*.cpp")
file(GLOB CS_GAMSH_SRC "../game_shared/*.cpp")
file(GLOB VGUI2_EMBEDDED_FONT_FILES "../game/font/*.ttf")
file(GLOB MINIUTL_SRC "../mainui_cpp/miniutl/*.cpp")
list(REMOVE_ITEM MINIUTL_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/../mainui_cpp/miniutl/strtools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../mainui_cpp/miniutl/utlbuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../mainui_cpp/miniutl/utlstring.cpp")

list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ev_hldm.cpp")
list(REMOVE_ITEM CS_INPUT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/input/input_sdl.cpp")
list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/inputw32.cpp")
list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/vgui2_testpanel.cpp")
list(REMOVE_ITEM CS_WPN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/cs_wpn/cs_baseentity.cpp")
list(APPEND CS_CLIENT_SRC ../public/utflib.cpp)
list(APPEND CS_CLIENT_SRC vgui2_bootstrap.cpp)
set(VGUI2_SDK_RUNTIME_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/KeyValues.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/interface.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/memstack.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/mempool.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/strtools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/tier1.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/utlbuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/utlstring.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier1/utlsymbol.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/vstdlib/KeyValuesSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/vstdlib/random.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier0/vcrmode_posix.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier0/mem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier0/mem_helpers.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier0/memstd.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/tier0/threadtools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/UnicodeFileHelpers.cpp"
)

set(VGUI2_SDK_MATHLIB_WASM_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/mathlib/mathlib_wasm.cpp")

file(GLOB VGUI2_SUPPORT_ROOT_SOURCES "../vgui2_support/*.cpp")
file(GLOB VGUI2_SUPPORT_VGUI2_SOURCES "../vgui2_support/vgui2/src/*.cpp")
file(GLOB VGUI2_SUPPORT_CONTROL_SOURCES "../vgui2_support/vgui_controls/*.cpp")
file(GLOB VGUI2_SUPPORT_GAMEUI_SOURCES "../vgui2_support/GameUI/*.cpp")
file(GLOB VGUI2_SUPPORT_DEAD_SOURCES "../vgui2_support/dead_wrapper/*.cpp")
file(GLOB VGUI2_SUPPORT_SURFACELIB_SOURCES "../vgui2_support/vgui2/vgui_surfacelib/*.cpp")
file(GLOB VGUI2_SUPPORT_MATSURFACE_SOURCES "../vgui2_support/vguimatsurface/*.cpp")
file(GLOB VGUI2_SUPPORT_CSO_CONTROL_SOURCES "../vgui2_support/cso_controls/*.cpp")

list(REMOVE_ITEM VGUI2_SUPPORT_CONTROL_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui_controls/AnalogBar.cpp")

list(REMOVE_ITEM VGUI2_SUPPORT_VGUI2_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/src/system_win32.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/src/HtmlWindow.cpp")
list(REMOVE_ITEM VGUI2_SUPPORT_SURFACELIB_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/vgui_surfacelib/Win32Font.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/vgui_surfacelib/Win32Font_x360.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/vgui_surfacelib/osxfont.cpp")
set(VGUI2_SUPPORT_ALL_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/filesystem_helpers.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui2/vgui_key_translation.cpp"
	${VGUI2_SUPPORT_ROOT_SOURCES}
	${VGUI2_SUPPORT_VGUI2_SOURCES}
	${VGUI2_SUPPORT_CONTROL_SOURCES}
	${VGUI2_SUPPORT_DEAD_SOURCES}
	${VGUI2_SUPPORT_SURFACELIB_SOURCES}
	${VGUI2_SUPPORT_MATSURFACE_SOURCES}
	${VGUI2_SUPPORT_CSO_CONTROL_SOURCES})
list(APPEND CS_CLIENT_SRC ${VGUI2_SDK_RUNTIME_SOURCES})
list(APPEND CS_CLIENT_SRC ${VGUI2_SUPPORT_ALL_SOURCES})
list(APPEND CS_CLIENT_SRC ${VGUI2_SUPPORT_GAMEUI_SOURCES})

# The create-game dialog family still assumes the older VGUI control wiring
# and does not build cleanly in the static wasm client yet.
list(REMOVE_ITEM VGUI2_SUPPORT_GAMEUI_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameBotPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameGameplayPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameServerPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/GameConsoleDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/LogoFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoeLogin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoeSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoEBulletIn.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubAdvanced.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubAudio.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubButtonSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubKeyboard.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubMouse.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubMultiplayer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubTouch.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubVideo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubVoice.cpp")
list(REMOVE_ITEM CS_CLIENT_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameBotPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameGameplayPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/CreateMultiplayerGameServerPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/GameConsoleDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/LogoFile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoeLogin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoeSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/MoEBulletIn.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubAdvanced.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubAudio.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubButtonSettings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubKeyboard.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubMouse.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubMultiplayer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubTouch.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubVideo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/GameUI/OptionsSubVoice.cpp")

list(APPEND CS_CLIENT_SRC ${CS_EV_SRC})
list(APPEND CS_CLIENT_SRC ${CS_WPN_SRC})
list(APPEND CS_CLIENT_SRC ${CS_WPNSH_SRC})
list(APPEND CS_CLIENT_SRC ${CS_HUD_SRC})
list(APPEND CS_CLIENT_SRC ${CS_STUDIO_SRC})
list(APPEND CS_CLIENT_SRC ${CS_PM_SRC})
list(APPEND CS_CLIENT_SRC ${CS_INPUT_SRC})
list(APPEND CS_CLIENT_SRC ${CS_GAMSH_SRC})
list(APPEND CS_CLIENT_SRC ${CS_VGUI_SRC})
list(APPEND CS_CLIENT_SRC ${CS_VGUI2_SRC})
list(APPEND CS_CLIENT_SRC ${CS_VGUI2_GAMECTRL_SRC})
list(APPEND CS_CLIENT_SRC ${CS_VGUI2_CSMOE_SRC})
list(REMOVE_ITEM CS_CLIENT_SRC ${CS_VGUI2_CSMOE_ZOMBIE_SRC})
list(APPEND CS_CLIENT_SRC ${MINIUTL_SRC})
list(APPEND CS_CLIENT_SRC ${VGUI2_SDK_MATHLIB_WASM_SOURCES})

set(VGUI2_EMBEDDED_FONTS_CPP "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedVGUIFonts.cpp")
add_custom_command(
	OUTPUT "${VGUI2_EMBEDDED_FONTS_CPP}"
	COMMAND ${Python_EXECUTABLE}
		"${CMAKE_CURRENT_SOURCE_DIR}/../scripts/embed_vgui_fonts.py"
		"${VGUI2_EMBEDDED_FONTS_CPP}"
		${VGUI2_EMBEDDED_FONT_FILES}
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../scripts/embed_vgui_fonts.py"
		${VGUI2_EMBEDDED_FONT_FILES}
	COMMENT "Embedding bundled VGUI font files"
	VERBATIM
)
list(APPEND CS_CLIENT_SRC "${VGUI2_EMBEDDED_FONTS_CPP}")

# Keep the live TeamMenu / BuyMenu / class menu path, but leave the zombie-mode
# csmoe sources in-tree for future restoration if needed.
list(REMOVE_ITEM CS_VGUI2_CSMOE_SRC ${CS_VGUI2_CSMOE_ZOMBIE_SRC})

set(VGUI2_VECTOR2D_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/vgui2_bootstrap.cpp"
	${CS_VGUI2_SRC}
	${CS_VGUI2_GAMECTRL_SRC}
	${VGUI2_SUPPORT_ALL_SOURCES})

set_source_files_properties(${VGUI2_VECTOR2D_SOURCES}
	PROPERTIES COMPILE_DEFINITIONS "CS16_USE_SOURCE_SDK_VECTOR2D")

set_source_files_properties(${VGUI2_SDK_RUNTIME_SOURCES}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier0;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public")

set_source_files_properties(${VGUI2_SUPPORT_ALL_SOURCES}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/interfaces;-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui_controls;-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/shims;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier0;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public")

set_source_files_properties(${CS_VGUI2_SRC} ${CS_VGUI2_GAMECTRL_SRC}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/interfaces;-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/shims;-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/vgui_controls;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier0;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public")

set_source_files_properties(${CS_VGUI2_CSMOE_SRC}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/shims;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier0;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public")

set_source_files_properties(${CS_VGUI_SRC}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public/tier0;-I${CMAKE_CURRENT_SOURCE_DIR}/../SourceSDK/public;-I${CMAKE_CURRENT_SOURCE_DIR}/../vgui2_support/shims")

include_directories(
	include
	include/hud
	include/studio
	include/math
	../cl_dll
	../cl_dll/vgui2
	../common
	../engine
	../pm_shared
	../dlls
	../game_shared
	../public/
	../public/cl_dll
	../SourceSDK/public/tier1
	../SourceSDK/public/tier0
	../SourceSDK/public
	../third_party/ValveFileVDF
	../vgui2_support
	../vgui2_support/interfaces
	../vgui2_support/vgui2
	../vgui2_support/vgui_controls
	../vgui2_support/cso_controls
	../vgui2_support/vguimatsurface
	../3rdparty/ReGameDLL_CS/regamedll/common
	../3rdparty/ReGameDLL_CS/regamedll/game_shared
	../../xash3d-fwgs/engine
	../mainui_cpp
	../mainui_cpp/controls
	../mainui_cpp/menus
	../mainui_cpp/miniutl)

add_definitions(-DCLIENT_WEAPONS -DCLIENT_DLL -DSTDINT_H=<cstdint>)
add_definitions(-DSOURCE_SDK_GFX_PATH=\"gfx/vgui2\")
add_definitions(-DCS16_USE_SOURCE_SDK_VECTOR2D)

if(NOT MSVC)
	add_definitions(-DGNUC -DPOSIX -DLINUX -D_LINUX)
	add_definitions(-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -fms-extensions)
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if(XASH_EMSCRIPTEN)
	add_definitions(-DXASH_STATIC_GAMELIB)
	add_executable(${CLIENT_LIB} ${CS_CLIENT_SRC})
	target_compile_options(${CLIENT_LIB} PRIVATE -fPIC -Oz -sUSE_FREETYPE=1)
	target_link_options(${CLIENT_LIB} PRIVATE -sSIDE_MODULE=1 -Oz -sASSERTIONS=2 -sUSE_FREETYPE=1)
	set_target_properties(${CLIENT_LIB} PROPERTIES SUFFIX ".wasm")
else()
	add_library(${CLIENT_LIB} SHARED ${CS_CLIENT_SRC})
endif()

set_target_postfix(${CLIENT_LIB})

install(TARGETS ${CLIENT_LIB}
	DESTINATION "${GAME_DIR}/${CLIENT_INSTALL_DIR}/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	    GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE)

# Install PDB file on Windows
if(MSVC)
	install(FILES $<TARGET_PDB_FILE:${CLIENT_LIB}>
		DESTINATION "${GAME_DIR}/${CLIENT_INSTALL_DIR}/" OPTIONAL)
endif()
