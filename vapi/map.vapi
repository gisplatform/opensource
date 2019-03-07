/*
 * map.vapi is a file with panorama mapapi bindings for vala
 *
 * Copyright (C) 2012 Sergey Volkhin, Andrey Vodilov.
 *
 * map.vapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * map.vapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with map.vapi. If not, see <http://www.gnu.org/licenses/>.
 *
*/

[CCode (cheader_filename = "gsslink.h,mapapi.h,vecexapi.h,netapi.h,mapsyst.h,mappicex.h,gmlapi.h,logapi.h")]
namespace Mapapi
{
  [CCode (cname = "MAPAPIVERSION")]
  public const int MAPAPI_VERSION;

  [CCode (cname = "MAPACCESSVERSION")]
  public const int MAP_ACCESS_VERSION;

  [CCode (cname = "ERRORHEIGHT")]
  public const double ERRORHEIGHT;

  [CCode (cname = "ERRORPOWER")]
  public const double ERRORPOWER;

	[CCode (cname = "XIMAGEDESC")]
	public struct XImageDesc	// Описатель битовой области XImage
	{                           // (для применения с XWindow)    // 25/09/00
	  public char *    Point;          // Адрес начала области пикселов
	  public int  Width;          // Ширина строки в пикселах
	  public int  Height;         // Число строк
	  public int  Depth;          // Размер элемента в битах (8,15,16,24,32)
	  public int  CellSize;       // Размер элемента(пиксела) в байтах
	  public int  RowSize;        // Ширина строки в байтах
	}

	[CCode (cname = "RECT")]
	public struct Map_Rect
	{
		public int left;
		public int top;
		public int right;
		public int bottom;
	}

	[CCode (cname = "DFRAME")]
	public struct Map_Frame
	{
	  public double X1;
	  public double Y1;
	  public double X2;
	  public double Y2;
	}

	[CCode (cname = "CROSSPOINT", destroy_function = "")]
	public struct CrossPoint
	{
	  MapPoint XY;     // Координата точки пересечения
	  double    H;     // Высота точки пересечения
						  // (из метрики)
	  Hobj Info1;
	  int Number1;   // За какой точкой объекта Info1
						  // находится точка пересечения
	  int Subject1;  // Номер объекта/подобъекта объекта info2
	  Hobj Info2;
	  int Number2;   // За какой точкой объекта Info2
						  // находится точка пересечения
	  int Subject2;  // Номер объекта/подобъекта объекта info2
	}

	[CCode (cname = "NUMBERPOINT")]
	public struct NumberPoint
	{
	  MapPoint Point;   // Координаты точки
	  int Number;     // Номер точки метрики, за которой следует point
	  int Update;     // Предложение замены точки метрики
	  int Equal;      // Номер точки совпадения в метрике
	  int Reserve;
	}

	[CCode (cname = "TASKPARMEX", destroy_function = "")] // ПАРАМЕТРЫ ПРИКЛАДНОЙ ЗАДАЧИ
	public struct TaskParam
	{
		public int    Language;        // Код языка диалогов (1 - ENGLISH, 2 - RUSSIAN, ...)
		public int    Resource;        // Модуль ресурсов приложения
		public string HelpName;        // Полное имя файла ".hlp"
		public string IniName;         // Полное имя файла ".ini" приложения
		public string PathShell;       // Каталог приложения (exe,dll,...)
		public string ApplicationName; // Имя приложения
		public int    Handle;          // Идентификатор обработчика команд главного окна приложения
		public int    DocHandle;       // Идентификатор окна карты (документа)
		public int    StayOnTop;       // Признак выставления форме свойства StayOnTop
	}

	[CCode (cname = "BUILDMTW")]
	public struct BuiltMtw
	{
		uint StructSize;
		double   BeginX;
		double   BeginY;
		double   Width;
		double   Height;
		double   ElemSizeMeters;
		int ElemSizeBytes;
		int Unit;
		int ReliefType;
		int UserType;
		int Scale;
		int HeightSuper;
		int FastBuilding;
		int Method;
		int Extremum;
		int Border;	//HOBJ
		int LimitMatrixFrame;
		int NotUse3DMetric;
		int SurfaceSquare3DObject;
		int AltitudeMarksNet;
		int LimitMatrixByFramesOfSheets;
		char Reserve[20];
	}

	[CCode (cname = "MAPREGISTER")]
	public struct MapRegister
	{
		long        Length;
		char        Name[32];
		long        Scale;
		long        EllipsoideKind;
		long        HeightSystem;
		long        MaterialProjection;
		long        CoordinateSystem;
		long        PlaneUnit;
		long        HeightUnit;
		long        FrameKind;
		long        MapType;
		long        DeviceCapability;
		long        DataProjection;
		long        OrderViewSheetFlag;
		long        FlagRealPlace;
		long        ZoneNumber;

		double      FirstMainParallel;
		double      SecondMainParallel;
		double      AxisMeridian;
		double      MainPointParallel;
	}

	[CCode (cname = "MAPREGISTEREX")]
	public struct  MAPREGISTEREX
  {
    int         Length                 ; // Размер данной структуры

    char        Name[32]               ; // Имя района
    int         Scale                  ; // Знаменатель масштаба
    int         EPSGCode               ; // Код EPSG системы координат     // 12/06/15
    int         ProjectionFlag         ; // Резерв = 0
    int         EllipsoideKind         ; // Вид эллипсоида
    int         HeightSystem           ; // Система высот
    int         MaterialProjection     ; // Проекция исх. материала
    int         CoordinateSystem       ; // Система координат
    int         PlaneUnit              ; // Единица измерения в плане
    int         HeightUnit             ; // Единица измерения по высоте
    int         FrameKind              ; // Вид рамки
    int         MapType                ; // Обобщенный тип карты
    int         DeviceCapability       ; // Разрешающая способность прибора
                                         // Обычно равна 20 000
                                         // Для карт повышенной точности:  // 26/12/06
                                         // -1 - максимальная точность
                                         // -2 - хранить координаты в сантиметрах
                                         // -3 - хранить координаты в миллиметрах
                                         // -7 - хранить координаты в радианах     // 17/03/10
    int         DataProjection         ; // Наличие данных о проекции
    int         ZoneIdent              ; // Идентификатор района (для МСК 63: A-X или 0)
    int         FlagRealPlace          ; // Вид карты
                                         // (0 - MAP (устаревший), 1 - SIT, -1 - SIT c постоянной рамкой)
    int         ZoneNumber             ; // Заполняется системой при запросе
                                         // формуляра - номер зоны топокарты

                                         // В радианах
    double      FirstMainParallel      ; // Первая главная параллель  StandardParallel1
    double      SecondMainParallel     ; // Вторая главная параллель  StandardParallel2
    double      AxisMeridian           ; // Осевой меридиан (Долгота полюса проекции) CentralMeridian
    double      MainPointParallel      ; // Параллель главной точки (Широта полюса проекции) LatitudeOfOrigin
    double      PoleLatitude           ; // (Latitude of false origin, etc)
    double      PoleLongitude          ; // (Longitude of false origin, etc)
                                                                         // 21/09/09
    double      FalseEasting           ; // Смещение координат по оси Y
    double      FalseNorthing          ; // Смещение координат по оси X
    double      ScaleFactor            ; // Масштабный коэффициент на осевом меридиане (1.0 +\- ...)
    double      TurnAngle              ; // Угол разворота осей для локальных систем (МСК)

    double      Reserv2[4]             ; // Резерв = 0
  }

  [CCode (cname = "DATUMPARAM")]
  public struct DATUMPARAM
  {
    double DX;           // Сдвиги по осям в метрах
    double DY;
    double DZ;
    double RX;           // Угловые поправки в секундах
    double RY;
    double RZ;
    double M;            // Поправка масштаба
    int    Count;        // 3 или 7  (14 - признак пересчета через ПЗ-90.02 для СК42\95)
    int    Reserve;      // Равно 0
  }

  [CCode (cname = "ELLIPSOIDPARAM")]
  public struct ELLIPSOIDPARAM
  {
    double SemiMajorAxis;          // Длина большой полуоси эллипсоида
    double InverseFlattening;      // Полярное сжатие эллипсоида
  }

	[CCode (cname = "LISTREGISTER")]
	public struct ListRegister
	{
		long Length;
		char Nomenclature[32];
		char ListName[32];

		char FileName[260];

		double XSouthWest;
		double YSouthWest;
		double XNorthWest;
		double YNorthWest;
		double XNorthEast;
		double YNorthEast;
		double XSouthEast;
		double YSouthEast;

		double BSouthWestCoordinate;
		double LSouthWestCoordinate;
		double BNorthWestCoordinate;
		double LNorthWestCoordinate;
		double BNorthEastCoordinate;
		double LNorthEastCoordinate;
		double BSouthEastCoordinate;
		double LSouthEastCoordinate;

		long MaterialKind;
		long MaterialType;
		long ReliefHeight;
		char Date[12];

		double MagneticAngle;
		double YearMagneticAngle;
		double MeridianAngle;
		char DateAngle[12];
		long Reserv2[3];
		MapPoint BorderSW;
	}

  [CCode (cname = "CREATESITEUN")]
  public struct CREATESITE
  {
   long   Length;             // Длина записи структуры CREATESITE
   WChar  MapName[128];       // Имя района в кодировке UNICODE (для функций "Un")
   long   MapType;            // Обобщенный тип карты
   long   MaterialProjection; // Проекция исх. материала
   long   EllipsoideKind;     // Тип эллипсоида (1 - Красовского, 9 - WGS84,...)
   long   Scale;              // Знаменатель масштаба карты
   long   Reserve;            // Резерв (должен быть 0)

                                // В радианах
   double FirstMainParallel;    // Первая главная параллель
   double SecondMainParallel;   // Вторая главная параллель
   double AxisMeridian;         // Осевой меридиан
   double MainPointParallel;    // Параллель главной точки
   double PoleLatitude;         // Широта полюса проекции
   double PoleLongitude;        // Долгота полюса проекции
  }

	[CCode (cname = "TBUILDZONEVISIBILITY")]
	public struct ViewZoneData
	{
		MapPoint PointCenter;
		double RadiusMeter;
		double Azimuth;
		double Angle;
		double DeltaHight;
		double DeltaObservation;
		long VisionRst;
		long StyleRst;
		long ColorRst;
		long Inversion;
	}

  [CCode (cname = "LISTREGISTER")]
  public struct LISTREGISTER
  {
    int         Length                 ;  // Размер данной структуры (544)
    char        Nomenclature[32]       ;  // Номенклатура листа
    char        ListName[32]           ;  // Название листа
    char        FileName[260]          ;  // Имя файла по которому в районе будет
                                          // создан лист с данным именем
                                          // и расширениями HDR, DAT, SEM, DRW

    // Прямоугольные координаты листа в метрах
    double      XSouthWest             ;  // X ю-з
    double      YSouthWest             ;  // Y ю-з
    double      XNorthWest             ;  // X с-з
    double      YNorthWest             ;  // Y с-з
    double      XNorthEast             ;  // X с-в
    double      YNorthEast             ;  // Y с-в
    double      XSouthEast             ;  // X ю-в
    double      YSouthEast             ;  // Y ю-в

    // Геодезические координаты листа в радианах
    double      BSouthWestCoordinate   ;  // B ю-з
    double      LSouthWestCoordinate   ;  // L ю-з
    double      BNorthWestCoordinate   ;  // B с-з
    double      LNorthWestCoordinate   ;  // L с-з
    double      BNorthEastCoordinate   ;  // B с-в
    double      LNorthEastCoordinate   ;  // L с-в
    double      BSouthEastCoordinate   ;  // B ю-в
    double      LSouthEastCoordinate   ;  // L ю-в

    int         MaterialKind           ;  // Вид исходного материала
    int         MaterialType           ;  // Тип ИКМ
    int         ReliefHeight           ;  // Высота сечения рельефа в дециметрах
    char        Date[12]               ;  // Дата съемки "ГГГГММДД"

    double      MagneticAngle          ;  // Магнитное склонение
    double      YearMagneticAngle      ;  // Годовое магнитное склонение
    double      MeridianAngle          ;  // Среднее сближение меридианов
    char        DateAngle[12]          ;  // Дата склонения "ГГГГММДД"

    char        Reserve[28]            ;  // = 0
  }

	[CCode (cname = "MAPADJACENTSECTION")]
  public struct MapAdjacentSection //MAPADJACENTSECTION  // ОПИСАНИЕ СМЕЖНОГО УЧАСТКА
  {
    int number;                 // номер участка                          // 26/03/12
    int first;                  // первая точка участка
    int last;                   // последняя точка участка
  }

  [CCode(cname = "HSITE")]
	public struct Hsite : int
	{

  }

  [CCode(cname = "HRSC")]
	public struct Hrsc : int
	{

  }

  [CCode(cname = "HGML", destroy_function = "gmlClose", cprefix = "gml")]
	public struct Hgml : int
	{
    [CCode (cname = "gmlOpenUn")]
    Hgml(Hmap hmap, Hsite hSite, WChar * schemafilename, WChar * schemaURL = null);

    public int FeatureTypeCount();
    public int FeatureTypeName(int number);
    public int GetGmlBorder(WChar* name, ref Map_Frame dframe);
  }

  [CCode(cname = "HGMLCLASS", destroy_function = "gmlFreeGmlClassHandle", cprefix = "gml")]
  public struct Hgmlcl : int
	{
    [CCode (cname = "gmlCreateObjectsFromXml")]
    Hgmlcl(Hmap hMap, Hsite hSite, WChar* xmlname, int squareCode, int pointCode, int lineCode, int textCode);



    public int GetGmlCreateObjectsCount();
  }
	[CCode(cname = "HMAP", destroy_function = "mapCloseData", cprefix = "map")]
	public struct Hmap : int
	{
		[CCode (cname = "mapOpenDataUn")]
		Hmap(WChar *name, int mode = 0);

		public void GetMainNameUn(WChar *name, int size);
		public void GetMapNameUn(WChar *name, int size);
		public int PaintToXImage(XImageDesc *imagedesc, int x, int y, Map_Rect *rect);
		public int SetViewScale(long *xp, long *yp, float scale);
		public int GetShowScale();
		public int PlaneToMap(ref double x, ref double y);
		public int PictureToPlane(ref double x, ref double y);
		public int PlaneToGeo42(ref double Bx, ref double Ly);
		public int GeoToPlane(ref double Bx, ref double Ly);
		public int PlaneToPicture(ref double x, ref double y);
		public int PlaneToGeoWGS84(ref double Bx, ref double Ly);
		public int GeoWGS84ToPlane42(ref double Bx, ref double Ly);
		public int GeoWGS84ToGeo42(ref double Bx, ref double Ly);
		public int Geo42ToGeoWGS84(ref double Bx, ref double Ly);
		public int Geo42ToGeoEP90(ref double Bx, ref double Ly);
		public int GeoEP90ToGeo42(ref double Bx, ref double Ly);
		public int GeoWGS84ToPlane3D(ref double Bx, ref double Ly, ref double H);
		public int GetHeightValue(double x, double y);
		public int GetMapScale();
		public int RestoreMapState(ref MapPoint point);
		public int SaveMapState(ref MapPoint point);
		public int SetupTurn(double angle, double fixation);
		public int WhatObject(Hobj info,ref Map_Frame frame,int flag, int place = 3);
		public int WhatObjectBySelect(Hobj info,ref Map_Frame frame, Hselect hs, int flag, int place = 3);
		public int GetLayerCount();
		public int GetSiteViewSelect(Hmap hm, Hselect select);
		public int SetSiteViewSelect(Hmap hm, Hselect select);
		public int GetSiteSeekSelect(Hmap hm, Hselect select);
		public int SetSiteSeekSelect(Hmap hm, Hselect select);
		public int GetLayerNameUn(int number, WChar *name, int size);
		public int SetContrast(int contrast);
		public int GetContrast();
		public int SetBright(int bright);
		public int GetBright();
		public int GetListCount();
		public int GetListNameUn(int number, WChar* name, int size);
		public int GetObjectCount(int list);
		public int GetRealObjectCount(int list);
		public int ReadObjectByNumber(Hsite site, Hobj info, int list, int object);
		public int SetViewType(int type);
		public int GetViewType();
		public int SetBackColor(int color);
		public int GetBackColor();
		public Hsite CreateAndAppendTempSiteUn(WChar* rscname);
		public Hrsc GetRscIdent(Hsite hSite = 0);
		public int CloseSiteForMap(Hsite hSite = 0);
		public Hobj CreateSiteObject(Hsite hSite,int kind = 0x7FFC7FFC, int text = 0);
		public int SetSiteViewFlag(Hsite hSite, int flag);
		public int GetSiteViewFlag(Hsite hSite = 0);
		public int CreateListFrameObject(int list, Hobj info);
    public int CreateList(ref LISTREGISTER sheet);
		public int TotalSeekObject(Hobj info,int flag = 2);
		public int TotalSeekObjectCount();
		public int SelectArea(Hobj object, double distance = 0, int filter = 0, int inside = 1,
												 int visible = 1, int action = 0, int nmap = -1);
		public int	UnselectArea();
		public int SeekSelectObject(Hobj info, Hselect select, int flag = 2);
		public int SeekSelectObjectCount(Hselect select);
		public int SetTotalSelectFlag(int flag);
		public double DistancePointSubject(Hobj info, long subject,ref MapPoint point);
		public int SeekNearVirtualPoint(Hobj info, ref MapPoint pointin, ref MapPoint pointout);
		public int GetTotalBorder(ref Map_Frame dframe, int place = 3);
    public int GetSiteBorder(Hsite hSite, ref Map_Frame dframe, int place = 3);
		public int BuildMtwUn(WChar *mtrname, string * filtername , ref BuiltMtw mtrparm, MessageHandler handle);
		public int OpenMtrForMap(string * mtrname, int mode = 0);
		public int OpenMtrForMapUn(WChar * mtrname, int mode = 0);
		public int CloseMtrForMap(int number = 0);
		public int SetMtrShadow(int set_value);
		public int GetMtrShadow();
		public int SetMtrShadowIntensity(int set_value);
		public int GetMtrShadowIntensity();
		public int SetMtrView(int number, int set_value);
		public int GetMtrView(int number = 1);
		public int SetMtrColorStyle(int style = 1);
		public int GetMtrColorStyle();
		public int GetMtrStandardPalette(out int palette, int count);
		public int SetMtrPalette(ref int palette, int count);
		public double GetMapX1();
		public double GetMapY1();
		public double GetMapX2();
		public double GetMapY2();
		public double GetMapInfo(int sheetnumber, ref MapRegister map, ref ListRegister sheet);
		public int VisibilityZoneUn(WChar *name, ref ViewZoneData zone_data);
		public int CloseRstForMap(int number = 0);
		public int OpenRstForMap(string *name, int mode = 0);
		public int GetMtrCount();
		public int GetRstCount();
		public int GetSiteCount();
		public int GetMapType();
		public int SetTextPlace(int place);
    public int GeneralFilterInMap(Hmap hm,  int list, double precision = 10, int hwnd = 0);
    public int SetGridActive(int active);
    public int IsGridActive();
    public Hsite OpenSiteForMapUn(WChar *sitename, int mode = 0);
    public Hsite OpenSiteForMap(string sitename, int mode = 0);
    public int SetNodeView(int mode);
    public int GetWorkSystemParametersEx( ref MAPREGISTEREX parm, ref DATUMPARAM datum, ref ELLIPSOIDPARAM ellipsoid);
    public int GetMapInfoEx(int sheetnumber, ref MAPREGISTEREX map, ref ListRegister list);
    public int ReadObjectByKey(Hsite hSite, Hobj info, int list, int key);
    public int SeekSiteSelectCount(Hsite hSite, Hselect select);
    public int RscObjectCountInLayer(int layer);
    public int RscObjectNameInLayer(int layer, int number);
    public int GetSiteNumber(Hsite site);
    public int GetSiteIdentByNameUn(WChar *name);
    public int DeleteMap();
    public string RscSemanticName(long code);
    public int RscSemanticCode(int number);
    public int AppendDataUn(WChar *name, int mode = 0);
    public int CloseSiteForMapByNameUn(WChar *name);
    public int GetRstNumberByNameUn(WChar* name);
    public int GetMtrNumberByNameUn(WChar *name);
    public int DeleteRst(int number);
    public int SetRstView(int number, int view);
    public int SetRstViewOrder(int number, int order);
    public int SetRstTransparent(int number, int transparent);
    public int GetRstLocation(int number, ref MapPoint location);
    public int GetMtrLocation(int number, ref MapPoint location);
    public int GetActualRstFrame(ref Map_Frame dframe, int number);
    public int GetActualMtrFrame(ref Map_Frame dframe, int number);
    public int GetSiteRangeScaleVisible(int number, out int bottomScale, out int topScale);
    public int SetSiteRangeScaleVisible(int number, int bottomScale, int topScale);
    public int GetMtrRangeScaleVisible(int number, out int bottomScale, out int topScale);
    public int SetMtrRangeScaleVisible(int number, int bottomScale, int topScale);
    public int GetRstRangeScaleVisible(int number, out int bottomScale, out int topScale);
    public int SetRstRangeScaleVisible(int number, int bottomScale, int topScale);
    public int SeekAdjacentObject(Hobj info, Hobj target, ref MapAdjacentSection section, Hselect select, double delta = 0.0, int flag = 0, int subject = 0);
    public double DistancePointObject(Hobj info, ref MapPoint point);
    public Hobj CreateCopyObject(Hobj info);
    public int WhatListNumber(double x, double y, int number = 1, int place = 3);
    public int DeleteObjectByNumber(int list, int number);
    public int GetObjectCenter(Hobj info, ref double x, ref double y);
    public int GetMtrPaletteCount();
    public int GetSiteObjectCount(Hsite hSite);
    public int GetMtrPalette([CCode (array_length_cname = "count", array_length_pos = 1.5, array_length_type = "int")] int[] palette);
    public int GetReliefRange(out double minvalue, out double maxvalue);
    public int CreateArc(Hobj info, ref MapPoint point1, ref MapPoint point2, ref MapPoint point3, double radius);
    public int GetHeightArray([CCode (array_length_cname = "HeightCount", array_length_pos = 1.5, array_length_type = "int")] double[] HeightArray, ref MapPoint FirstPoint, ref MapPoint SecondPoint);
    public int CopySiteUn(Hsite hSite, WChar *newname);
    public int ClearSite(Hsite hSite);
    public int SetSiteBorder(Hsite hSite, ref Map_Frame dframe, int place = PPLACE.PLANE);
    public int DeleteList(int list);
    public int SetAxisMeridianByMeridian(double meridian);
    public int StructControl(Hsite hSite, int mode, MessageHandler handle);
    public int GetListNumberByNameUn(WChar *name);
    public int SetRealShowScale(double scale);
    public int LogAccess(Hsite hSite, int mode = 0);
    public int GetLogAccess(Hsite hSite);
    public int PaintExampleObjectByFuncToXImageUn(XImageDesc *imagedesc, Map_Rect *rect, int func, char *parm, int colors, uint *palette, WChar *text, int local);
    public int GetSiteColorsCount(Hsite hSite);
    public int GetSitePalette(Hsite hSite, uint *colors, int count = 256);
	}

	[CCode(cname = "HOBJ", destroy_function = "mapFreeObject", cprefix = "map")]
	public struct Hobj : int
	{
		[CCode (cname = "mapCreateObject")]
		Hobj(Hmap hmap, int sheetnumber = 1, Kind kind = Kind.FLOAT2, int text = 0);

		public int ObjectNameUn(WChar *name, int size);
		public int PointCount(int subject = 0);
		public double XPlane(int number, int subject = 0);
		public double YPlane(int number, int subject = 0);
		public int SetXPlane(double x, int number, int subject = 0);
		public int SetYPlane(double y, int number, int subject = 0);
		public int ObjectLocal();
		public double Square();
		public double Length();
		public double Perimeter();
		public int SemanticAmount();
    public int SemanticCode(int number);
		public int DeleteSemantic(int number);
    public int AppendSemanticUnicode(int code, WChar *value, int size = 256);
		public int SemanticValueFullNameUn(int number, WChar *place,int maxsize);
		public int SemanticValueNameUn(int number, WChar *place,int maxsize);
    public int SemanticCodeValueUn(int code, WChar *place, int maxsize, int number);
    public int SemanticValueUn(int number, WChar *place, int size);
    public int SemanticLongValue(int number);
    public int SemanticNumber(int code);
    public int SemanticNameUn(int number, WChar *name, int size);
    public double SemanticCodeDoubleValue(int code, int number);
		public int ClearObject();
		public int RegisterObject(int excode, int local);
    public int RegisterObjectByKey(string key);
		public int AppendPointGeo(double b, double l, int subject = 0);
		public int AppendPointPlane(double x, double y, int subject = 0);
		public int UpdatePointGeo(double b,double l, int number, int subject = 0);
		public int UpdatePointPlane(double x,double y, int number, int subject = 0);
		public int InsertPointGeo(double b,double l, int number, int subject = 0);
		public int InsertPointPlane(double x,double y, int number, int subject = 0);
		public int DeletePointPlane(int number, int subject = 0);
		public int DeleteSubject(int subject = -1);
		public int Abrige(double delta = 0.2);
		public int Commit();
		public int CommitWithPlace();
		public int IsDirtyObject();
		public int ObjectKey();
    public unowned string ObjectRscKey();
    public long SetObjectKind(Kind kind);
		public int SetObjectMap(int new_map);
		public int CrossCutAndSubject(MapPoint xy1, MapPoint xy2,
                             int first, int last,
                             NumberPoint point1,
                             NumberPoint point2, int subject,
                             double precision);
		public int CheckInsidePoint(int subject, ref MapPoint point);
    public int ObjectTopScale();
    public int ObjectBotScale();
    public int LinearFilter(double precision);
    public int DeleteEqualPoint(double precision = 1.0, int height = 0);
    public int ObjectCode();
    public int GetObjectNumber();
    public int GetTextUn(WChar *text, int size, int subject = 0);
    public int BendSpline(int press = 10, int smooth = 10, double precision = -1);
    public int PolyCount();
    public int SeekNearPoint(MapPoint point, int subject = 0);
    public int ObjectRscKeyUn(WChar *key, int size);
    public int GetListNumber();
    public int SegmentNumber();
    public int SegmentNameUn(WChar *name, int size);
    public int CommitWithPlaceAsNew();
    public int CommitWithPlaceForList(int list);
    public int PutTextUnicode(WChar *text, int subject = 0);
    public int GetTextUnicode(WChar *text, int size = 256, int subject = 0);
    public int InsertPointCross(Hobj obj);
    public int ChangeObjectMap(Hmap hMap, Hsite hSite);
    public int CheckInsideObject(Hobj info2);
    public int SeekVirtualPointByDistance(int number, double distance, ref MapPoint point, int subject = 0);
    public int GetTextLengthMkm(int subject = 0);
    public int GetTextHeightMkm();
    public int IsMultiPolygon();
    public int GetSubjectMultiFlag(int subject = 0);
    public int DeleteObject();
    public int FreeObject();
	}

	[CCode(cname = "HSELECT", destroy_function = "mapDeleteSelectContext", cprefix = "map")]
	public struct Hselect : int
	{
		[CCode (cname = "mapCreateMapSelectContext")]
		Hselect(Hmap hmap);
		public int CheckLayer(int layer);
		public int CheckObject(int object);
		public int CheckLocal(int local);
		public int CheckList(int list);
		public int SelectLayer(int layer, int check);
		public int SelectLocal(int object, int check);
    public int SelectObject(int object, int check);
    public int SelectList(int list, int check);

		public int SelectSeekArea(int object,
                                     double distance = 0.0,
                                     int filter = 0,
                                     int inside = 1,
                                     int visible = 0,
                                     int action = 0);
    public int SelectSeekAreaFrame(  ref Map_Frame frame,
                                     double distance = 0.0,
                                     int filter = 0,
                                     int inside = 1,
                                     int visible = 0,
                                     int action = 0);
    public int GetSampleCount(int list);
    public int GetSampleByNumber(int list, int number);
    public int SetSampleAllObjects(int list);
    public int ClearSelectContext();
    public int SelectTitleUn(WChar *value);
    public int UnselectSeekArea();
    public int SelectAndSelect(Hselect source);
	}


	[CCode (cname = "DOUBLEPOINT", has_destroy_function = false)]
	public struct MapPoint	   //ОПИСАНИЕ ЭЛЕМЕНТА СПИСКА РАЙОНОВ РАБОТ
	{
	  public double X;
	  public double Y;
	}

	[CCode(cname = "HCROSS", destroy_function = "mapFreeCrossPoints", cprefix = "map")]
	public struct Hcross : int
	{
		[CCode (cname = "mapCreateObjectCrossPoints")]
		Hcross(Hobj first_obj, Hobj second_obj);
		public int GetNextCross(Hobj result_info);
		public int GetCrossCount();
		public int GetCrossPoint(int number,ref CrossPoint point);
	}

  [CCode (cheader_filename = "netapi.h", cname = "SEMNETNUMBER")]
	public const int SEM_NET_NUMBER;

	[CCode(cname = "WCHAR", cprefix = "")]
	public struct WChar { }

  [CCode(cname = "gssLink")]
  public int gssLink();
	[CCode(cname = "mapScaleToRoundScale")]
	  public int ScaleToRoundScale(float scale);
	[CCode(cname = "mapScreenMeter2Pixel")]
		public int ScreenMeter2Pixel(double metric);
	[CCode(cname = "mapGeo42ToPlaneByOwnZone")]
		public void Geo42ToPlaneByOwnZone(ref double Bx, ref double Ly);
	[CCode(cname = "mapDistance")]
		public double Distance(ref MapPoint point1, ref MapPoint point2);
	[CCode(cname = "mapCheckFileExUn")]
		public int CheckFile(WChar *name);
	[CCode(cname = "mapGetRscFileNameUn")]
		public int GetRscFileNameUn(Hrsc hRsc, WChar *target, int size);
	[CCode(cname = "mapGetRscObjectCountInLayer")]
    public int GetRscObjectCountInLayer(Hrsc hRsc, int layer);
	[CCode(cname = "mapGetRscObjectFunction")]
    public int GetRscObjectFunction(Hrsc hRsc, int incode);
	[CCode(cname = "mapGetRscObjectParameters")]
    public char *GetRscObjectParameters(Hrsc hRsc, int incode);
	[CCode(cname = "mapGetRscObjectNameInLayerUn")]
    public int GetRscObjectNameInLayerUn(Hrsc hRsc, int layer, int number, WChar *name, int size);
	[CCode(cname = "mapGetRscObjectKeyIncode")]
		public int GetRscObjectKeyIncode(Hrsc hRsc, string key);
	[CCode(cname = "mapGetRscObjectCodeByNumber")]
		public int GetRscObjectCodeByNumber(Hrsc hRsc, int excode, int local, int number = 1);
	[CCode(cname = "mapGetRscObjectExcodeInLayer")]
		public int GetRscObjectExcodeInLayer(Hrsc hRsc, int layer, int number);
	[CCode(cname = "mapGetRscObjectLocalInLayer")]
		public int GetRscObjectLocalInLayer(Hrsc hRsc, int layer, int number);
	[CCode(cname = "mapGetRscSemanticKey")]
		public unowned string GetRscSemanticKey(Hrsc hRsc, long code);
  [CCode(cname = "mapOpenRscUn")]
    public Hrsc OpenRscUn(WChar* name);
  [CCode(cname = "mapCloseRsc")]
    public int CloseRsc(Hrsc hRsc);
  [CCode(cname = "GetRscNameFromAnySxfUn")]
    public int GetRscNameFromAnySxfUn(WChar* name, WChar* rscname, long size = 256);
	[CCode(cname = "mapCreateTempSiteUn")]
		public Hmap CreateTempSite(WChar *rscname);
	[CCode(cname = "mapCreateTempSiteExUn")]
		public Hmap CreateTempSiteExUn(WChar *rscname, ref MAPREGISTEREX mapreg, ref DATUMPARAM datum, ref ELLIPSOIDPARAM ellipsoid);
  [CCode(has_target = false)]
    public delegate long MessageHandler(long hwnd, long code, long p1, long p2, long typemsg);
  [CCode(cname = "LoadDirToMap")]
    public int LoadDirToMap(string namedir, string namemap, MessageHandler message_handler);
	[CCode(cname = "SaveDirSxfFromMap")]
		public int SaveDirSxfFromMap(string namemap, string namedir, MessageHandler message_handler);
	[CCode(cname = "ExportToDir")]
		public int ExportToDir(Hmap hm, string dirname, int type = 0, int flag = 0, int total = 1, int precision = 0, int handle = 0, Hselect select = 0);
	[CCode(cname = "ImportFromAnySxfPro")]
		public int ImportFromAnySxfPro( Hmap hmap,
                                    WChar * namesxf,
                                    WChar * namersc,
                                    WChar * namemap,
                                    int      size,
                                    MessageHandler message_handler,
                                    Hselect      select = 0,
                                    int frscfromsxf = 0,
                                    int typesit = 1,
                                    WChar * password = null,
                                    int psize = 0);
	[CCode(cname = "ImportFromAnySxfEx")]
	public int	ImportFromAnySxfEx(Hmap hmap,
                                 string sxfname,
                                 string rscname,
                                 string mapname,
                                 int size = 256,
                                 int handle = 0,
                                 Hselect select = 0,
                                 int frscfromsxf = 0);

  [CCode(cname = "ExportToDirUn")]
  public int	ExportToDirUn(Hmap hmap,
                            WChar * dirname,
                            int type, int flag, int total,
                            int precision, MessageHandler message_handler,
                            Hselect select = 0, int frsc = 0, int isutf8 = 0);

	[CCode(cname = "mapMessageEnable")]
		public int MessageEnable(int enable);
	[CCode(cname = "mapSetMapAccessLanguage")]
		public int SetMapAccessLanguage(int code);
	[CCode(cname = "mapPlaneToPlaneByZone")]
		public int PlaneToPlaneByZone(int source,int target, ref double x, ref double y);
	[CCode(cname = "mapPlaneToGeo42ByZone")]
		public int PlaneToGeo42ByZone(int zone, ref double x, ref double y);
	[CCode(cname = "mapGetZoneByMeridian")]
		public int GetZoneByMeridian(double meridian);
	[CCode(cname = "mapGetAxisMeridianByZone")]
		public int GetAxisMeridianByZone(int zone);
	[CCode(cname = "mapGetObjectsCross")]
		public int GetObjectsCross(Hobj first_obj, Hobj second_obj);
	[CCode(cname = "mapCopyObjectAsNew")]
		public int CopyObjectAsNew(int dest, int src);
	[CCode(cname = "mapReadCopyObject")]
		public int ReadCopyObject(Hobj dest, Hobj src);
	[CCode(cname = "mapReadCopySubject")]
		public int ReadCopySubject(Hobj dest, Hobj src, int subject);
  [CCode(cname = "mapSetTextQuality")]
    public int SetTextQuality(int flag);
  [CCode(cname = "onGetGraphSiteIdent")]
    public Hsite onGetGraphSiteIdent(Hmap hm);
	[CCode(cname = "mapCopyMap")]
		public int CopyMap(string *name_src, string *name_dest);
	[CCode(cname = "onCreateNetDlgEx")]
		public Hsite  CreateNet(Hmap hmap, TaskParam? parm, string namesit, string namersc, int flag = 0);
  [CCode(cname = "onSeekPathByPoint")]
  	public int onSeekPathByPoint(Hmap hmap, Hsite site, Hobj info, TaskParam? parm, ref MapPoint point1, ref MapPoint point2);
  [CCode(cname = "onSeekPathByPointEx")]
  	public int onSeekPathByPointEx(Hmap hmap, Hsite site, Hobj info, TaskParam? parm, ref MapPoint point1, ref MapPoint point2, Hselect hs = 0, int foneway = 0);
  [CCode(cname = "Unicode8ToUnicode")]
    public int Unicode8ToUnicode(string src, WChar *dest, int bytes);
  [CCode(cname = "UnicodeToUnicode8")]
    public int UnicodeToUnicode8(WChar *src, string dest, int bytes);
  [CCode(cname = "Unicode8ToString")]
    public int Unicode8ToString(string src, string dest, int bytes);
  [CCode(cname = "mapPlaneToStringUn")]
    public int PlaneToString(ref double x, ref double y, ref double h, WChar *place, int size, int maptype);
  [CCode(cname = "mapIsMapSite")]
    public int IsMapSite(Hmap map);
  [CCode(cname = "mapIsTempSite")]
    public int IsTempSite(Hmap map, Hsite site);
  [CCode(cname = "mapDeleteMapByNameUn")]
    public int DeleteMapByNameUn(WChar *name);
  [CCode(cname = "mapDeleteSiteByNameUn")]
    public int DeleteSiteByNameUn(WChar *name);
  [CCode(cname = "mapDeleteMtrFileUn")]
    public int DeleteMtrFileUn(WChar *name);
  [CCode(cname = "mapDeleteRstFileUn")]
    public int DeleteRstFileUn(WChar *name);
  [CCode(cname = "mapCopyRstFileUn")]
    public int CopyRstFileUn(WChar *oldname, WChar *newname, int exist = 0);
  [CCode(cname = "mapCopyMtrFileUn")]
    public int CopyMtrFileUn(WChar *oldname, WChar *newname, int exist = 0);
  [CCode(cname = "mapCopyMapUn")]
    public int CopyMapUn(WChar *oldname, WChar *newname);
  [CCode(cname = "DeleteTheFile")]
    public int DeleteTheFile(WChar *name);
  [CCode(cname = "mapDeleteMapByNameEx")]
    public int DeleteMapByNameEx(WChar *name, int rsc_delete = 1);
  [CCode(cname = "GetDirItemCountUn")]
    public int GetDirItemCountUn(WChar *dirname);
  [CCode(cname = "SxfCheckSumUn")]
    public int SxfCheckSumUn(WChar *name);
  [CCode(cname = "GetSxfCheckSumUn")]
    public int GetSxfCheckSumUn(WChar *name);
  [CCode(cname = "picexPaintDataToFileUn")]
    public int picexPaintDataToFileUn(int handle, WChar * inputFileName, WChar * outputFileName, int width = 640, int height = 480, int messageEnable = 1);
  [CCode(cname = "picexPaintDataToFile")]
    public int picexPaintDataToFile(int handle, string inputFileName, string outputFileName,int width = 640, int height = 480, int messageEnable = 1);
  [CCode(cname = "LoadRstMirrorUn")]
    public int LoadRstMirrorUn(int handle, Hmap map, WChar* name, WChar* newname, int mirrortype = 0);
  [CCode(cname = "picexSaveRswToPngUn")]
    public int picexSaveRswToPngUn(int handle, WChar* rswName, WChar* pngName);
  [CCode(cname = "mapZoneObject")]
  public int ZoneObject(double radius, Hobj info, int subject = 0, int form = 0);
  [CCode(cname = "gmlGetGmlBorder")]
  public int GetGmlBorder(WChar* name, ref Map_Frame dframe);
  [CCode(cname = "gmlCreateObjectsFromJSON")]
  public int CreateObjectsFromJSON(Hmap hMap, Hsite hSite, WChar* xmlname, int squareCode, int pointCode, int lineCode, int textCode);
  [CCode(cname = "mapCreateSiteUn")]
  public Hmap CreateSiteUn(WChar* mapname, WChar* rscname, ref CREATESITE createsite);
  [CCode(cname = "mapGetRscSegmentSemanticCode")]
  public int GetRscSegmentSemanticCode(Hrsc hrsc,int layer, int number);
  [CCode(cname = "MapSortProcess")]
  public int SortProcess(Hmap hmap, MessageHandler handle, int mode = 0);
  [CCode(cname = "mapSetExclusiveAccess")]
  public int SetExclusiveAccess(int access = 1);

  [CCode(cprefix = "ID")]
  public enum Kind                 // ВИДЫ ФОРМАТОВ МЕТРИКИ
  {
    SHORT2  = 0x7FFF7FFF, // двухбайтовая целочисленная
    LONG2   = 0x7FFE7FFE, // четырехбайтовая целочисленная
    FLOAT2  = 0x7FFD7FFD, // с плавающей запятой
    DOUBLE2 = 0x7FFC7FFC, // с плавающей запятой двойной точностью
    SHORT3  = 0x7FFB7FFB, // двухбайтовая целочисленная трехмерная
    LONG3   = 0x7FFA7FFA, // четырехбайтовая целочисленная трехмерная
    FLOAT3  = 0x7FF97FF9, // с плавающей запятой трехмерная
    DOUBLE3 = 0x7FF87FF8, // с плавающей запятой двойной точностью трехмерная
    BAD     = 0x7FF87FF7  // неизвестный вид
  }

	enum PPLACE           // ПРИМЕНЯЕМАЯ СИСТЕМА КООРДИНАТ
	{
	   MAP     = 1,    // КООРДИНАТЫ ТОЧЕК В СИСТЕМЕ КАРТЫ В ДИСКРЕТАХ
	   PICTURE = 2,    // КООРДИНАТЫ ТОЧЕК В СИСТЕМЕ ИЗОБРАЖЕНИЯ В ПИКСЕЛАХ
	   PLANE   = 3,    // КООРДИНАТЫ ТОЧЕК В ПЛОСКОЙ ПРЯМОУГОЛЬНОЙ СИСТЕМЕ
						  // НА МЕСТНОСТИ В МЕТРАХ
	   GEO     = 4,    // КООРДИНАТЫ ТОЧЕК В ГЕОДЕЗИЧЕСКИХ КООРДИНАТАХ
						  // В РАДИАНАХ
	}

	enum SEEKTYPE        // ПОРЯДОК ПОИСКА ОБ'ЕКТОВ
	{
	   WO_FIRST  = 0,    // Первый в цепочке
	   WO_LAST   = 2,    // Последний в цепочке
	   WO_NEXT   = 4,    // Следующий за найденным ранее
	   WO_BACK   = 8,    // Предыдущий от ранее найденного
	   WO_CANCEL = 16,   // Включая удаленные объекты
	   WO_INMAP  = 32,   // Только по одной карте (соответствующей HSELECT)
	   WO_VISUAL = 64,   // Поиск только среди видимых объектов
	}

	[CCode(cprefix = "FILE_")]
	enum MapType                         // ИДЕНТИФИКАТОРЫ ФАЙЛОВ (Intel)
	{
		SXF  = 0x00465853,   // ДВОИЧНЫЙ SXF ФАЙЛ
		TXT  = 0x4658532E,   // ТЕКСТОВЫЙ SXF ФАЙЛ (.SXF)
		DIR  = 0x524944,     // ТЕКСТОВЫЙ DIR ФАЙЛ
		PCX  = 0x10,         // ФАЙЛ PCX (ОПРЕДЕЛЯЕТСЯ НЕ ЧЕТКО)
		BMP  = 0x4D42,       // ФАЙЛ BMP
		TIFF = 0x4949,       // ФАЙЛ TIFF
		JPEG = 0xD8FF,       // ФАЙЛ JPEG
		MAP  = 0x00505350,   // ПАСПОРТ РАЙОНА РАБОТ
    MAPSIT = 0x00544953, // ПАСПОРТ РАЙОНА РАБОТ SIT
		RST  = 0x00545352,   // ФАЙЛ RST
		RSW  = 0x00575352,   // ФАЙЛ RSW
		MTR  = 0x0052544D,   // ФАЙЛ MTR
		MTW  = 0x0057544D,   // ФАЙЛ MTW
		MTL  = 0x004C544D,   // ФАЙЛ MTL
		SIT  = 0x5449532E,   // ТЕКСТОВЫЙ SXF(.SIT) ФАЙЛ
		DXF  = 0x00465844,   // ФАЙЛ DXF
		MIF  = 0x0046494D,   // ФАЙЛ MAPINFO
		S57  = 0x00003000,   // ФАЙЛ S57(КАТАЛОГ)
		DGN  = 0x004E4744,   // ФАЙЛ Macrostation(DGN)   // 23/03/04 Sh.
		MPT  = 0x0054504D,   // ФАЙЛ MPT(ПРОЕКТ)
		RSC  = 0x00435352,   // ФАЙЛ RSC
		MTQ  = 0x0051544D,   // ФАЙЛ MTQ
		PLN  = 0x004E4C50,   // ФАЙЛ PLN (Talka)    // 15/12/02
		SHP  = 0x00504853,   // ФАЙЛ SHP (ArcView)  // 23/07/03
		PLS  = 0x00534C50,   // ФАЙЛ PLS (Список растров)          // 05/07/04
		TEXT = 0x00545854,   // Файл TXT(геодезические приборы)    // 23/11/04
		GPS  = 0x47504724,   // Файл GPS/NMEA ($GPG)               // 07/12/04
		GRD  = 0x00445247,   // ФАЙЛ GRD (матрицы формата FOTOMOD) // 10/12/04
		DBF  = 0x00464244,   // ФАЙЛ DBF (База данных)             // 16/12/04 Sh.
		TIN  = 0x004E4954,   // ФАЙЛ TIN                           // 31/03/05


								  // ИДЕНТИФИКАТОРЫ ФАЙЛОВ (Sparc, Mips)

		MAP_TURN = 0x50535000, // ПАСПОРТ РАЙОНА РАБОТ
		MTW_TURN = 0x4D545700, // ФАЙЛ MTW
		SXF_TURN = 0x53584600, // ДВОИЧНЫЙ SXF ФАЙЛ    //03/11/03
		DIR_TURN = 0x44495200, // ТЕКСТОВЫЙ DIR ФАЙЛ   //05/12/03
		RSW_TURN = 0x52535700, // ФАЙЛ RSW  // 20/01/04
		RSC_TURN = 0x52534300, // ФАЙЛ RSC
	}

  [CCode(cprefix = "VT_")]
  enum VTYPE               // ТИП ОТОБРАЖЕНИЯ КАРТЫ     // 13/09/00
  {
     // ЭКРАННЫЙ ВЫВОД
     SCREEN          = 1, // ЭКРАННЫЙ (ЧЕРЕЗ DIB)
     SCREENCONTOUR   = 2, // ЭКРАННЫЙ КОНТУРНЫЙ

     // ПРИНТЕРНЫЙ ВЕКТОРНЫЙ ВЫВОД
     PRINT           = 3, // ПРИНТЕРНЫЙ ВЕКТОРНЫЙ (ЧЕРЕЗ WIN API)
     PRINTGLASS      = 4, // ПРИНТЕРНЫЙ БЕЗ ЗАЛИВКИ ПОЛИГОНОВ
     PRINTCONTOUR    = 5, // ПРИНТЕРНЫЙ КОНТУРНЫЙ, БЕЗ УСЛОВНЫХ ЗНАКОВ

     // ПРИНТЕРНЫЙ РАСТРИЗОВАННЫЙ ВЫВОД
     PRINTRST        = 6, // ПРИНТЕРНЫЙ РАСТРИЗОВАННЫЙ (ЧЕРЕЗ WIN API)
     PRINTGLASSRST   = 7, // ПРИНТЕРНЫЙ БЕЗ ЗАЛИВКИ ПОЛИГОНОВ
     PRINTCONTOURRST = 8, // ПРИНТЕРНЫЙ КОНТУРНЫЙ, БЕЗ УСЛОВНЫХ ЗНАКОВ

     // ПРИНТЕРНЫЙ РАСТРИЗОВАННЫЙ (СПЕЦИАЛЬНЫЙ) ВЫВОД        // 13/02/03
     PRINTRSTSQUARE  = 9, // ПРИНТЕРНЫЙ (ЗАЛИВКИ ПЛОЩАДНЫХ, ПОДПИСИ С ФОНОМ,
                             //             РАСТРЫ, МАТРИЦЫ)
     PRINTRSTLINE    =10, // ПРИНТЕРНЫЙ (ЛИНИИ, ТОЧЕЧНЫЕ, ВЕКТОРНЫЕ,
                             //             ПОДПИСИ С ФОНОМ, ПОЛЬЗОВАТЕЛЬСКИЕ)
     PRINTRSTTEXT    =11, // ПРИНТЕРНЫЙ (ПОДПИСИ, ШАБЛОНЫ)

     // ПРИНТЕРНЫЙ ВЕКТОРНЫЙ (СПЕЦИАЛЬНЫЙ) ВЫВОД. ИСПОЛЬЗУЕТСЯ ДЛЯ ЭКСПОРТА
     // ГРАФИКИ (POSTSCRIPT, WMF, EMF)                       // 16/12/04
     PRINTEX         =15, // ПРИНТЕРНЫЙ ВЕКТОРНЫЙ (ЧЕРЕЗ WIN API)
  }


  //  ИДЕНТИФИКАТОРЫ КОМАНД (WM_COMMAND) -->
    [CCode(cprefix = "WM_")]
    enum WMType
    {
      ERRORCOORD   = 0x583,    // Информация о ошибках паспорта
      INFOLIST     = 0x584,    // Информация о листе
      OBJECT       = 0x585,    // Смена объекта
      LIST         = 0x586,    // Смена листа
      ERROR        = 0x587,    // Информация об ошибках
      MAP          = 0x588,    // Смена текущей карты
      ERRORSXF     = 0x589,    // Информация об ошибках обработки SXF

      PROGRESSBAR  = 0x590,    // Сообщение о соcтоянии процесса
      MAPEVENT     = 0x591,    // Сообщение о событиях карты
      PROGRESSICON = 0x592,    // Сообщение о соcтоянии процесса
                               // для отображения в иконке программы
                               // при свернутом главном окне
      PROGRESSBARW = 0x593,    // Сообщение о соcтоянии процесса в UNICODE
    }
  //  ИДЕНТИФИКАТОРЫ КОМАНД (WM_COMMAND) <--
}

