if(UNIX)
  set(_optional_vapi ../../vapi/DPMS.vapi)
  set(_optional_pkgs x11 gdk-x11-3.0)
endif(UNIX)

vala_precompile(VALA_C
    ActivityButton.vala
    Barista.vala
    Cairo.vala
    Connector.vala
    CellRendererStar.vala
    Dialog.vala
    InactiveForm.vala
    MenuTree.vala
    Nagiev.vala
    Prefable.vala
    ScreenDoctor.vala
    StackFilterBox.vala
    TreeExport.vala
    ViewPort.vala
    Palette.vala
  CUSTOM_VAPIS
    ${_optional_vapi}
  GENERATE_GIR
    GpCoreGui-2.0
  GENERATE_VALADOC
    gp-core-gui-2.0
  GENERATE_VAPI
    gp-core-gui
  GENERATE_HEADER
    gp-core-gui
  PACKAGES
    gtk+-3.0
    gp-core-2.0
    ${_optional_pkgs}
  OPTIONS
    --thread)

add_library(gpcoregui SHARED ${VALA_C})

#Translations
set_property( TARGET gpcoregui PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}" )

target_link_libraries(gpcoregui ${GLIB2_LIBRARIES})
target_link_libraries(gpcoregui ${GTK3_LIBRARIES})
target_link_libraries(gpcoregui gpcore)
target_link_libraries(gpcoregui m)

vala_postcompile(gpcoregui GIR_NAME GpCoreGui GIR_VER 2.0)

