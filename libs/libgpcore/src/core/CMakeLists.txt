vala_precompile(VALA_C
    ActivityFunc.vala
    Bookmark.vala
    Chunk.vala
    Dumper.vala
    GrowingArea.vala
    Tile.vala
    MemTile.vala
    Dispatcher.vala
    FileUtils.vala
    Filter.vala
    GeoConst.vala
    Ladle.vala
    Point.vala
    StackFilter.vala
    Trunk.vala
    Worker.vala
    ${pulse_valas}
  CUSTOM_VAPIS
    ${pulse_vapis}
  GENERATE_GIR
    GpCore-2.0
  GENERATE_VALADOC
    gp-core-2.0
  GENERATE_VAPI
    gp-core
  GENERATE_HEADER
    gp-core
  PACKAGES
    gio-2.0
    glib-2.0
    ${pulse_pkgs}
  OPTIONS
    --thread)

# GResources.
compile_gresources(RESOURCE_FILE XML_OUT TYPE EMBED_C PREFIX "@ru@gis-platform@${PROJECT_NAME}"
  RESOURCES dist/lebaston-click.wav dist/steady-heart.wav dist/wine-glass.wav)
add_custom_target(${PROJECT_NAME}-resource ALL DEPENDS ${RESOURCE_FILE})

add_library(gpcore SHARED ${VALA_C} resources.c)
target_link_libraries(gpcore ${GLIB2_LIBRARIES} ${PULSE_SIMPLE_LIBRARIES} m)

#Translations
set_property( TARGET gpcore PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}" )

vala_postcompile(gpcore GIR_NAME GpCore GIR_VER 2.0)

