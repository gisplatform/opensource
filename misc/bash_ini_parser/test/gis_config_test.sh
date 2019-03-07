. ../read_ini.sh
if ! read_ini ~/gis_platform_data/config/advanced.ini GpsdConverter ;then exit 1; fi

echo "GpsdConverter__enable:$INI__GpsdConverter__enable"
echo "GpsdConverter__input:$INI__GpsdConverter__input"
echo "Guidance__enable:$INI__Guidance__enable"

