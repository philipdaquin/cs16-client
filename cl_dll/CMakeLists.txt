cmake_minimum_required(VERSION 3.10)
project(client)

set(CLIENT_LIB client)

file(GLOB CS_CLIENT_SRC "*.cpp")
file(GLOB CS_EV_SRC "events/*.cpp")
file(GLOB CS_VGUI_SRC "VGUI/*.cpp")
file(GLOB CS_WPN_SRC "cs_wpn/*.cpp")
file(GLOB CS_WPNSH_SRC "../dlls/wpn_shared/*.cpp")
file(GLOB CS_HUD_SRC "hud/*.cpp")
file(GLOB CS_STUDIO_SRC "studio/*.cpp")
file(GLOB CS_INPUT_SRC "input/*.cpp")
file(GLOB CS_PM_SRC "../pm_shared/*.cpp")
file(GLOB CS_GAMSH_SRC "../game_shared/*.cpp")
file(GLOB MINIUTL_SRC "../3rdparty/mainui_cpp/miniutl/*.cpp")
list(REMOVE_ITEM MINIUTL_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/mainui_cpp/miniutl/strtools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/mainui_cpp/miniutl/utlbuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/mainui_cpp/miniutl/utlstring.cpp")

list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ev_hldm.cpp")
list(REMOVE_ITEM CS_INPUT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/input/input_sdl.cpp")
list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/inputw32.cpp")
list(REMOVE_ITEM CS_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/vgui2_testpanel.cpp")
list(REMOVE_ITEM CS_WPN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/cs_wpn/cs_baseentity.cpp")
list(APPEND CS_CLIENT_SRC ../common/interface.cpp)
list(APPEND CS_CLIENT_SRC ../public/utflib.cpp)
list(APPEND CS_CLIENT_SRC vgui2_bootstrap.cpp vgui2_stub_impl.cpp)
set(VGUI2_SDK_RUNTIME_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/KeyValues.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/mempool.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/strtools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/tier1.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/utlbuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/utlstring.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier1/utlsymbol.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier2/KeyValuesCompat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier2/tier2.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/tier2/steam_api.cpp"
)

set(VGUI2_SDK_CONTROLS_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public/filesystem_helpers.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public/vgui_controls/vgui_controls.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/src/vgui_key_translation.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/AnalogBar.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/AnimatingImagePanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/AnimationController.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/BitmapImagePanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/BuildFactoryHelper.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/BuildGroup.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/BuildModeDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Button.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/CheckButton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/CheckButtonList.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ComboBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ControllerMap.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/controls.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/DirectorySelectDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Divider.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/EditablePanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ExpandButton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/FileOpenDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/FocusNavGroup.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Frame.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/GraphPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/HTML.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Image.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ImageList.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ImagePanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/InputDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/KeyBindingHelpDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/KeyBoardEditorDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/KeyRepeat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Label.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ListPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ListViewPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Menu.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/MenuBar.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/MenuButton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/MenuItem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/MessageBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/MessageDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Panel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/PanelListPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ProgressBar.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ProgressBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/PropertyDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/PropertyPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/PropertySheet.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/QueryBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/RadioButton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/RichText.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/savedocumentquery.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ScrollableEditablePanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ScrollBar.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ScrollBarSlider.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/SectionedListPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Slider.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Splitter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/TextEntry.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/TextImage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ToggleButton.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/Tooltip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/ToolWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/TreeView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/TreeViewListControl.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/URLLabel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/WizardPanel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/vgui2/vgui_controls/WizardSubPanel.cpp")
list(APPEND CS_CLIENT_SRC ${VGUI2_SDK_RUNTIME_SOURCES})
list(APPEND CS_CLIENT_SRC ${VGUI2_SDK_CONTROLS_SOURCES})

list(APPEND CS_CLIENT_SRC ${CS_EV_SRC})
list(APPEND CS_CLIENT_SRC ${CS_WPN_SRC})
list(APPEND CS_CLIENT_SRC ${CS_WPNSH_SRC})
list(APPEND CS_CLIENT_SRC ${CS_HUD_SRC})
list(APPEND CS_CLIENT_SRC ${CS_STUDIO_SRC})
list(APPEND CS_CLIENT_SRC ${CS_PM_SRC})
list(APPEND CS_CLIENT_SRC ${CS_INPUT_SRC})
list(APPEND CS_CLIENT_SRC ${CS_GAMSH_SRC})
list(APPEND CS_CLIENT_SRC ${CS_VGUI_SRC})
list(APPEND CS_CLIENT_SRC ${MINIUTL_SRC})

set(VGUI2_SDK_VECTOR2D_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/vgui2_bootstrap.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/buymenu.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/buysubmenu.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/counterstrikeviewport.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/counterstrikeviewport_interface.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/cstrikeclassmenu.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VGUI/cstriketeammenu.cpp")

set_source_files_properties(${VGUI2_SDK_VECTOR2D_SOURCES}
	PROPERTIES COMPILE_DEFINITIONS "CS16_USE_SOURCE_SDK_VECTOR2D")

set_source_files_properties(${VGUI2_SDK_RUNTIME_SOURCES}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public")

set_source_files_properties(${VGUI2_SDK_CONTROLS_SOURCES}
	PROPERTIES COMPILE_OPTIONS
	"-I${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public/tier1;-I${CMAKE_CURRENT_SOURCE_DIR}/../../hl1_source_sdk/public")

include_directories(
	include
	include/hud
	include/studio
	include/math
	../cl_dll
	../common
	../engine
	../pm_shared
	../dlls
	../game_shared
	../public/
	../public/cl_dll
	../../hl1_source_sdk/public
	../../hl1_source_sdk/common
	../../hl1_source_sdk/public/tier1
	../../hl1_source_sdk/public/tier2
	../../hl1_source_sdk/public/vgui
	../../hl1_source_sdk/public/vgui_controls
	../../hl1_source_sdk/vgui2/vgui_controls
	../3rdparty/mainui_cpp
	../3rdparty/mainui_cpp/controls
	../3rdparty/mainui_cpp/menus
	../3rdparty/mainui_cpp/miniutl)

add_definitions(-DCLIENT_WEAPONS -DCLIENT_DLL -DSTDINT_H=<cstdint>)
add_definitions(-DSOURCE_SDK_GFX_PATH=\"gfx/vgui2\")

if(NOT MSVC)
	add_definitions(-DGNUC -DPOSIX -DLINUX -D_LINUX)
	add_definitions(-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -fms-extensions)
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if(XASH_EMSCRIPTEN)
	add_executable(${CLIENT_LIB} ${CS_CLIENT_SRC})
	target_compile_options(${CLIENT_LIB} PRIVATE -fPIC -Oz)
	target_link_options(${CLIENT_LIB} PRIVATE -sSIDE_MODULE=1 -Oz)
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
