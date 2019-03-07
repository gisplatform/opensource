 /**
   * Виджет для отображения палитры.
   * */
using GLib;
using Gtk;
using Cairo;

namespace Gp
{
  public enum PaletteType
  {
    NONE,
    ADVANCED,
    SIMPLE,
    NUM
  }

  public enum ScaleType
  {
    CUSTOM = 0,
    RAINBOW,
    GRAY,
    BRONZE,
    FIX,
    BLUE,
    AMOUNT;

    public string to_string()
    {
      switch(this)
      {
        case CUSTOM:  return _("Custom scale");
        case RAINBOW: return _("Rainbow scale");
        case GRAY:    return _("Gray scale");
        case BRONZE:  return _("Bronze scale");
        case FIX:     return _("Use fixed scale");
        case BLUE:    return _("Blue scale");
        default:      return _("Unknown Scale");
      }
    }

    public int acu_list_cnt()
    {
      switch(this)
      {
        case GRAY:    return 0;
        case BRONZE:  return 1;
        case BLUE:    return 2;
        case CUSTOM:
        case RAINBOW:
        case FIX:
        default:      return 0;
      }
    }
  }

  public struct Palette
  {
    private const uint SERIALIZE_API_VERSION = 2; //Версия структуры палитры
    internal const float AUTO_VALUE_RANGE = 0.3f;  //Разброс значений от референсных данных при работе с auto_min_max.

    public bool auto_limits;  //Признак использования авто минмакс.
    public float d1;  //Минимальное значение для отображения
    public float d2;  //Максимальное значение для отображения
    public uint c1;   //Цвет для мин., % (от красного к синему)
    public uint c2;   //Цвет для макс., % (от красного к синему)
    public uint8 br;  //Яркость (0-255)
    public uint n;    //Количество градаций в дискретной шкале (0 -- непрерывная шкала)
    public int cur_scale;  //Шкала
    public float bad_data_limit;  //предел значений

    private const uint[] scale = { 0x3F0106, 0x790000, 0x9F0100, 0xBE0300, 0xE20100, 0xFB4400, 0xFD6D00,
                            0xFAA100, 0xF8BD00, 0x00D201, 0x00C000, 0x00AB00, 0x029600, 0x008103,
                            0x007000, 0x005B05, 0x007B97, 0x026096, 0x1D3D6C, 0x0F235E, 0x05071E };

    public void set_default_plt( PaletteType type = PaletteType.ADVANCED)
    {
      this.auto_limits = true;
      this.d1    = 5.0f;
      this.d2    = 12.0f;
      this.c1    = 0;
      this.c2    = 100;
      this.br    = 50;
      this.n     = 0;
      this.bad_data_limit     = 0.00001f;

      if (type == PaletteType.ADVANCED)
        this.cur_scale = ScaleType.FIX;
      else
        this.cur_scale = ScaleType.GRAY;
    }

    public string ? params_palette_to_string()
    {
      int d1_cm = (int)(this.d1 * 100.0);
      int d2_cm = (int)(this.d2 * 100.0);

      string ? res = "%d:%d:%d:%d:%d:%d:%d:%d:%d".printf((int)Palette.SERIALIZE_API_VERSION, (int)this.auto_limits, (int)d1_cm, (int)d2_cm, (int)this.c1, (int)this.c2, (int)this.br, (int)this.n, this.cur_scale);
      return res;
    }

    public bool parse_params_palette_from_string(string ? str)
    {
      if(str != null)
      {
        int arr[9];
        if(str.scanf("%d:%d:%d:%d:%d:%d:%d:%d:%d", out arr[0], out arr[1], out arr[2], out arr[3], out arr[4], out arr[5], out arr[6], out arr[7], out arr[8]) > 0)
        {
          this.auto_limits = (bool)arr[1];
          this.d1     = (float)arr[2] / (float)100.0;
          this.d2     = (float)arr[3] / (float)100.0;
          this.c1     = (uint)arr[4];
          this.c2     = (uint)arr[5];
          this.br     = (uint8)arr[6];
          this.n      = (uint)arr[7];
          this.cur_scale = (int)arr[8];

          return true;
        }
      }

      this.set_default_plt();
      this.auto_limits  = true;

      return false;
    }


    /**
    * Заполняет массив "pixel" из четырех элементов (RGBA)
    * значением в соответствии с алгоритмом палитры.
    *
    * @param val значение в данной точке.
    * @param pixel Полученный цвет.
    * @param min Минимум палитры(для ручного режима).
    * @param max Максимум палитры(для ручного режима).
    */

    public uint32 get_pixel( float val, PaletteType plt_type, [CCode(array_length = 4)] uint8[]? pixel = null, double min = 0, double max = 100)
    {
      if(plt_type == PaletteType.ADVANCED)
      {
        if(val < this.d1 || val > this.d2)
        {
          if(pixel != null)
            Memory.set(pixel, 0, 4);

           return 0;
        }
      }

      uint32 i, color=0;

      switch(this.cur_scale)
      {
        case ScaleType.FIX:     //Палитра с заданной шкалой
        default:
        {
          this.n = Palette.scale.length;
          i = val_to_n( val, this.d1, this.d2, this.n);
          color = Palette.scale[i];
        }
        break;
        case ScaleType.BRONZE:  //Бронзовая
        {
          if (this.n != 0)
            val = val_scaling( val, this.d1, this.d2, this.n);

          i = (uint)(255.0 * (val - this.d1) / (this.d2 - this.d1));

          uint r = (uint)(32.0 + (255.0 - 32.0) * (float)(255 - i) / 256.0);
          uint g = (uint)(192.0 * (float)(255 - i) / 256.0);
          uint b = (uint)(64.0 - (64.0 * (float)(255 - i)) / 256.0);
          color = (0xFF0000 & (r<<16)) + (0xFF00 & (g<<8)) + (0xFF & b);
        }
        break;
        case ScaleType.GRAY: //Серая
        {
          if (this.n != 0)
            val = val_scaling( val, this.d1, this.d2, this.n);

          i = (uint)(255.0 * (val - this.d1) / (this.d2 - this.d1));

          uint r = 255 - i;
          uint g = 255 - i;
          uint b = 255 - i;
          color = (0xFF0000 & (r<<16)) + (0xFF00 & (g<<8)) + (0xFF & b);
        }
        break;
        case ScaleType.CUSTOM:
        {
          if(this.n == 0) //непрерывная палитра, от c1 до c2 % цвета
          {
            i = val_to_linear( val, this.d1, this.d2, this.c1, this.c2);
            color = linear_to_rgb( i, 48);
          }
          else  //палитра с n градациями, от c1 до c2 % цвета
          {
            val = val_scaling( val, this.d1, this.d2, this.n);
            i = val_to_linear( val, this.d1, this.d2, this.c1, this.c2);
            color = linear_to_rgb( i, this.br);
          }
        }
        break;
        case ScaleType.RAINBOW:
        {
          //палитра с n градациями цвета
          {
            if(this.n != 0)
              val = val_scaling( val, this.d1, this.d2, this.n);

            i = 255 - (uint)(0.5 + 255.0 * (val - this.d1) / (this.d2 - this.d1));

            uint r = 0;
            uint g = 0;
            uint b = 0;

            if(i < 64)
            {
              r = 63 - i;
              g = 2 * i;
              b = 63 + 3 * i;
            }
            else if(i < 128)
                  {
                    r = 0;
                    g = 63 + i;
                    b = 511 - 4 * i;
                  }
                  else if( i < 192)
                        {
                          r = 4 * i - 511;
                          g = 63 + i;
                          b = 0;
                        }
                        else if(i < 256)
                              {
                                r = 255;
                                g = 255 - 4 * (i - 192);
                                b = 0;
                              }
            color = (0xFF0000 & (r<<16)) + (0xFF00 & (g<<8)) + (0xFF & b);
          }
        }
        break;
        case ScaleType.BLUE:  //Рукопожатная
        {
          if (this.n != 0)
            val = val_scaling( val, this.d1, this.d2, this.n);

          i = (uint)(255.0 * (val - this.d1) / (this.d2 - this.d1));

          uint r = 0;
          uint g = 192 - 3*i/4;
          uint b = 255 - i;
          color = (0xFF0000 & (r<<16)) + (0xFF00 & (g<<8)) + (0xFF & b);
        }
        break;
      }

      // --> ARGB32
      if (pixel != null)
      {
///Pixel заполняется неправильно!!!!
        pixel[0] = 0xFF & color;          //B
        pixel[1] = 0xFF & (color >> 8);   //G
        pixel[2] = 0xFF & (color >> 16);  //R
        pixel[3] = 0xFF;  //A

        color |= (uint32)0xFF000000;  //ARGB

///!!!!!!!!!!!!!!!!
//~         color |= (uint32)(0xFF000000 & (pixel[3] << 24));  //ARGB
//~         if (val < this.bad_data_limit || val > this.d2)
///!!!!!!!!!!!!!!!!

        if (val < 0.00001f || val > this.d2)
        {
          Memory.set(pixel, 0, 4);
          color = 0;
        }
        else
          pixel[3] = 0xFF;  //A
      }
      else
        color |= (uint32)0xFF000000;  //ARGB


      return color;
    }


    private uint val_to_n( float val, float dmin, float dmax, uint nmax)
    {
      uint n;

      if (val < dmin) n = 0;
      else if (val > dmax) n = nmax+1;
      else
      {
        val = nmax * (val - dmin) / (dmax - dmin);
        n = (uint)val;
      }

      return n + 1;
    }


    // dmin/dmax -- заданные границы значений
    // n -- количество цветовых градаций в шкале
    // Результат -- значение выровненное на соответствующую градацию
    public float val_scaling( float val, float dmin, float dmax, uint n)
    {
      float d = val, step = 1;
      uint s;

      if (val >= dmin && val <= dmax)
      {
        step = ((dmax - dmin) / n);
        d = (val - dmin) / step;
        s = (uint)(d + 0.5);
        d = dmin + s * step;
      }
      else if(d < dmin && d > 1.0)
        d = dmin;
      else if(d > dmax && d > 1.0)
        d = dmax;

      return d;
    }


    // dmin/dmax -- заданные границы значений
    // cmin/cmax -- заданные границы цвета, %
    // Результат -- единици измерения величины -> условные единицы (0-6*256)
    private uint val_to_linear( float val, float dmin, float dmax, uint cmin, uint cmax)
    {
      uint linear = 0;

      cmin *= 6 * 255 / 100;
      cmax *= 6 * 255 / 100;

      if (val < dmin) linear = cmin;
      else if (val > dmax) linear = cmax;
      else linear = (uint)((cmax - cmin) * (val - dmin) / (dmax - dmin) + cmin);

      return linear;
    }


    // linear: 0-6*256 -> #RRGGBB
    private uint linear_to_rgb( uint linear, uint8 intensity)
    {
      uint8 c1 = intensity;
      uint8 c2 = 0xFF - c1;
      uint r=0, g=0, b=0, rgb;

      if (linear < 256)  // #FF(00-FF)00
      {
        r = c2;
        g = linear;
        b = c1;
      }
      else if (linear < 2*256)  // #(FF-00)FF00
      {
        r = 2*255 - linear;
        g = c2;
        b = c1;
      }
      else if (linear < 3*256)  // #00FF(00-FF)
      {
        r = c1;
        g = c2;
        b = linear - 2*255;
      }
      else if (linear < 4*256)  // #00(FF-00)FF
      {
        r = c1;
        g = 4*255 - linear;
        b = c2;
      }
      else if (linear < 5*256)  // #(00-FF)00FF
      {
        r = linear - 4*255;
        g = c1;
        b = c2;
      }
      else if (linear < 6*256)  // #FF00(FF-00)
      {
        r = c2;
        g = c1;
        b = 6*255 - linear;
      }

      rgb = (0xFF0000 & (r<<16)) + (0xFF00 & (g<<8)) + (0xFF & b);

      return rgb;
    }

  } //< Class.


  public class PaletteBox : Gp.Barista, Gp.Prefable
  {
    public PaletteType type = PaletteType.ADVANCED;// { public get; private set; }

    private const uint DEFAULT_SCALE_MIN = 3;
    private const uint DEFAULT_SCALE_MAX = 65;

    private string palette_box_name = null;
    private Palette pl;
    private Frame frame_scale         = null;
    private Frame color_frame         = null;
    private Frame step_frame          = null;
    private SpinButton min_val_spin = null;  //Минимальное значение
    private SpinButton max_val_spin = null;  //Максимальное значение
    private SpinButton min_color_spin = null;  //Минимальное значение цвета при настройке пользовательской шкалы (0-100%)
    private SpinButton max_color_spin = null;  //Максимальное значение цвета при настройке пользовательской шкалы (0-100%)
    private SpinButton scale_spin     = null;  //Калибровка цветовой шкалы (количество градаций)
    private CheckButton use_auto      = null;  //Авто-настройка значений
    private DrawingArea darea         = null;
    private ComboBoxText combo_box    = null;  //Выбор цветовой шкалы из списка
    private Button apply_btn          = null;  //Кнопка применить настройки


    /**
     * Сигнал о загрузке конфига.
     *
     */
    public signal void config_loaded();

    ///Получить указатель на кнопку "Применить"
    public unowned Button get_apply_btn()
    {
      return this.apply_btn;
    }

    ///Получить текущую палитру
    public Palette get_palette()
    {
      Palette parent_plt;
      parent_plt = this.pl;
      return parent_plt;
    }

    ///Установить текущую палитру
    public void set_palette( Palette plt)
    {
      this.pl = plt;
      this.update_info();
    }

    ///Получить границы шкалы калибровки
    public void get_scale_limits(out uint cur_min, out uint cur_max)
    {
      double min, max;
      scale_spin.get_range(out min, out max);
      cur_min = (uint)min;
      cur_max = (uint)max;
    }

    ///Установить границы шкалы калибровки
    public void set_scale_limits(uint new_min, uint new_max)
    {
      scale_spin.set_range((double)new_min, (double)new_max);
    }

    ///Установить предел значений
    public void set_data_limit(float data_limit)
    {
      this.pl.bad_data_limit = data_limit;
    }

    ///Установить границы значений
    public bool set_min_max(string ? traverse_name, float min, float max)
    {
      if (this.type != PaletteType.ADVANCED) return false;

      if(this.pl.auto_limits == true)
      {
        this.pl.d1 = min - Palette.AUTO_VALUE_RANGE * min;
        this.pl.d2 = max + Palette.AUTO_VALUE_RANGE * max;

        min_val_spin.set_numeric(true);
        max_val_spin.set_numeric(true);

        min_val_spin.set_value(this.pl.d1);
        max_val_spin.set_value(this.pl.d2);

        if(traverse_name != null)
          frame_scale.set_label(_("Scale") + _(" for ") + traverse_name);

        this.darea.get_window().invalidate_rect(null, false);

        return true;
      }
      return false;
    }

    ///Обновить показания виджетов согласно значениям текущей палитры
    public void update_info()
    {
      if(this.type == PaletteType.ADVANCED)
      {
        if(this.use_auto.get_active() != this.pl.auto_limits)
          this.use_auto.set_active(this.pl.auto_limits);

        this.min_val_spin.set_value( this.pl.d1);
        this.max_val_spin.set_value( this.pl.d2);
        this.min_color_spin.set_value( this.pl.c1);
        this.max_color_spin.set_value( this.pl.c2);

        if(this.pl.n != 0)
        {
          this.scale_spin.set_value(this.pl.n);
        }
        else
        {
          this.scale_spin.set_value(scale_spin.adjustment.upper);
          scale_spin.set_numeric(false);
          (scale_spin as Entry).set_text(_("Continuous scale"));
        }

        this.combo_box.set_active(this.pl.cur_scale);
      }
      else
      {
        this.combo_box.set_active(((ScaleType)this.pl.cur_scale).acu_list_cnt());
      }

      //this.darea.get_window().invalidate_rect(null, false);
    }


    ///Конструктор
    public PaletteBox( Palette? plt, PaletteType intype = PaletteType.ADVANCED)
      requires(intype < PaletteType.NUM)
    {
      this.type = intype;

      //Основной бокс для виджетов с настройками
      var main_vbox = new Box( Orientation.VERTICAL, 0);
      main_vbox.set_orientation( Orientation.VERTICAL);
      //main_vbox.hexpand = true;

      this.scrolled.set_policy( PolicyType.NEVER, PolicyType.AUTOMATIC);
      this.scrolled.add( main_vbox);

      //Кнопка "Применить" в тулбаре
      ToolItem item = new Gtk.ToolItem();
      this.toolbar.add( item);
      var btn_hbox = new Box( Orientation.HORIZONTAL, 10);
      item.add( btn_hbox);
      this.apply_btn = new Button.with_label(_("Apply"));
      btn_hbox.pack_start( this.apply_btn, true, true, 0);

      //Инициализация палитры
      if(plt != null)
        this.pl = plt;
      else if(this.type == PaletteType.ADVANCED)
        this.pl.set_default_plt();
      else
        this.pl.set_default_plt(PaletteType.SIMPLE);


      if (this.type == PaletteType.ADVANCED)
      {
        //Границы значений (Val Limit Spin Buttons) --->
        var val_frame = new Frame(_("Limits"));

        var vbox = new Box( Orientation.VERTICAL, 0);

        use_auto = new CheckButton.with_label(_("Use auto min-max"));
        use_auto.set_active(this.pl.auto_limits);

        vbox.pack_start(use_auto, false, false, 1);
        use_auto.toggled.connect (() =>
        {
          this.pl.auto_limits = use_auto.get_active();

          min_val_spin.set_sensitive(!this.pl.auto_limits);
          max_val_spin.set_sensitive(!this.pl.auto_limits);

          min_val_spin.set_numeric(!this.pl.auto_limits);
          max_val_spin.set_numeric(!this.pl.auto_limits);

          if(this.pl.auto_limits == true)
          {
            (min_val_spin as Entry).set_text(_("Select source"));
            (max_val_spin as Entry).set_text(_("Select source"));
          }
          else
          {
            frame_scale.set_label(_("Scale"));
            min_val_spin.set_value(this.pl.d1);
            max_val_spin.set_value(this.pl.d2);
          }

          this.darea.get_window().invalidate_rect(null, false);
        });

        val_frame.add( vbox);

        min_val_spin = new SpinButton.with_range(0.0, 100.0, 1.0);
        min_val_spin.set_digits(1);
        min_val_spin.set_sensitive(!this.pl.auto_limits);
        min_val_spin.set_increments( 1.0, 10.0);
        vbox.pack_start( min_val_spin, false, false, 1);

        max_val_spin = new SpinButton.with_range(1.0, 100.0, 1.0);
        max_val_spin.set_digits(1);
        max_val_spin.set_sensitive(!this.pl.auto_limits);
        max_val_spin.set_increments( 1.0, 10.0);
        vbox.pack_start( max_val_spin, false, false, 1);

        min_val_spin.value_changed.connect (() => {
          this.pl.d1 = (float)min_val_spin.get_value();
          if (this.pl.d1 >= this.pl.d2)
          {
            this.pl.d2 = this.pl.d1 + (float)1.0;
            max_val_spin.set_value( this.pl.d2);
          }
          this.darea.get_window().invalidate_rect(null, false);
        });

        max_val_spin.value_changed.connect (() => {
          this.pl.d2 = (float)max_val_spin.get_value();
          if (this.pl.d2 <= this.pl.d1)
          {
            this.pl.d1 = this.pl.d2 - (float)1.0;
            min_val_spin.set_value( this.pl.d1);
          }
          this.darea.get_window().invalidate_rect(null, false);
        });

        val_frame.set_sensitive( true);
        main_vbox.pack_start( val_frame, false, false, 0);
        //Границы значений <---

        //Границы цвета (Color Limit Spin Buttons) --->
        color_frame = new Frame(_("Color limits (%)"));

        vbox = new Box( Orientation.VERTICAL, 0);
        color_frame.add( vbox);

        min_color_spin = new SpinButton.with_range(0, 100, 1);
        min_color_spin.set_increments( 1, 10);
        vbox.pack_start( min_color_spin, false, false, 1);

        max_color_spin = new SpinButton.with_range(1, 100, 1);
        max_color_spin.set_increments( 1, 10);
        vbox.pack_start( max_color_spin, false, false, 1);

        min_color_spin.value_changed.connect (() =>
        {
          this.pl.c1 = (uint)min_color_spin.get_value();
          if (this.pl.c1 >= this.pl.c2)
          {
            this.pl.c2 = this.pl.c1 + 1;
            max_color_spin.set_value( this.pl.c2);
          }
          this.darea.get_window().invalidate_rect(null, false);
        });

        max_color_spin.value_changed.connect (() =>
        {
          this.pl.c2 = (uint)max_color_spin.get_value();
          if (this.pl.c2 <= this.pl.c1)
          {
            this.pl.c1 = this.pl.c2 - 1;
            min_color_spin.set_value( this.pl.c1);
          }
          this.darea.get_window().invalidate_rect(null, false);
        });

        if (this.pl.cur_scale == ScaleType.FIX)
          color_frame.set_sensitive(false);
        else
          color_frame.set_sensitive(true);

        main_vbox.pack_start( color_frame, false, false, 0);
        //Границы цвета <---

        //Калибровка цветовой шкалы --->
        step_frame = new Frame(_("Calibration"));

        scale_spin = new SpinButton.with_range(DEFAULT_SCALE_MIN, DEFAULT_SCALE_MAX, 1);
        scale_spin.set_increments( 1, 1);
        scale_spin.set_update_policy(SpinButtonUpdatePolicy.IF_VALID);
        step_frame.add( scale_spin);

        scale_spin.value_changed.connect (() =>
        {
          if((uint)scale_spin.get_value() == (uint)scale_spin.adjustment.upper)
          {
            scale_spin.set_numeric(false);
            (scale_spin as Entry).set_text(_("Continuous scale"));

            this.pl.n = 0;
            this.darea.get_window().invalidate_rect(null, false);
          }
          else
          {
            this.pl.n = (uint)scale_spin.get_value();
            this.darea.get_window().invalidate_rect(null, false);
          }
        });

        main_vbox.pack_start( step_frame, false, false, 0);
        //Калибровка цветовой шкалы <---
      } //<-- PaletteType.ADVANCED


      //Выбор цветовой шкалы из списка --->
      var combo_frame = new Frame(_("Palette in use"));
			combo_box = new ComboBoxText();
			combo_frame.add(combo_box);

      Array<ScaleType> acu_list = new Array<ScaleType>();

      if (this.type == PaletteType.ADVANCED)
      {
        combo_box.append_text(ScaleType.CUSTOM.to_string());
        combo_box.append_text(ScaleType.RAINBOW.to_string());
        combo_box.append_text(ScaleType.GRAY.to_string());
        combo_box.append_text(ScaleType.BRONZE.to_string());
        combo_box.append_text(ScaleType.FIX.to_string());
      }
      else
      {
        ScaleType sc = ScaleType.GRAY;
        combo_box.append_text(sc.to_string());
        acu_list.append_val(sc);
        sc = ScaleType.BRONZE;
        combo_box.append_text(sc.to_string());
        acu_list.append_val(sc);
        sc = ScaleType.BLUE;
        combo_box.append_text(sc.to_string());
        acu_list.append_val(sc);
      }

      combo_box.changed.connect(() =>
			{
        ScaleType sc = (ScaleType)combo_box.get_active();

        if (this.type == PaletteType.ADVANCED)
        {
          bool has_color = (sc != ScaleType.CUSTOM) ? false : true;
          color_frame.set_sensitive( has_color);

          bool has_scale = (sc == ScaleType.FIX) ? false : true;
          step_frame.set_sensitive( has_scale);

          if (sc != ScaleType.FIX)
          {
            uint steps = (uint)scale_spin.get_value();
            pl.n = (steps == (uint)scale_spin.adjustment.upper) ? 0 : steps;
          }
        }
        else
        {
          sc = acu_list.index(combo_box.get_active());
          //step_frame.set_sensitive( true);
        }

        this.pl.cur_scale = sc;
        this.darea.get_window().invalidate_rect(null, false);
      });

      main_vbox.pack_start( combo_frame, false, false, 0);
      //Выбор цветовой шкалы из списка <---


      //Отрисовка цветовой шкалы на панели настроек --->
      frame_scale = new Frame(_("Scale"));
      darea = new DrawingArea();
      darea.set_size_request( 10, 100);

      darea.map.connect(() =>
      {
        this.update_info();
      });

      darea.draw.connect ((ctx) =>
      {
        double w = darea.get_allocated_width();
        double h = darea.get_allocated_height();

        float delta = (pl.d2 - pl.d1) / (float)h;

        double offset = 0.5;
        uint n = pl.n;
        if(n == 0)
        {
          n = (int)h;
          offset = 0.0;
        }

        double d_h = h / n;

        for(uint i = 0; i < n; i ++)
        {
          uint8 pixel[4];
          pl.get_pixel(pl.d1 + (float)(i * d_h) * delta, this.type, pixel, -1.0, -1.0);
          ctx.set_source_rgb( (double)pixel[2] / 256.0, (double)pixel[1] / 256.0, (double)pixel[0] / 256.0);
          ctx.rectangle( (double)w * 0.2, i * d_h, (double)w * 0.6, d_h - offset);
          ctx.fill();
        }

        if(max_val_spin != null && max_val_spin.get_numeric() == true)
        {
          n = uint.max(pl.n, 5);
          int step = 1;

          if(n > 15)
            step = 5;

          ctx.set_source_rgb( 0.98, 0.98, 0.98);
          TextExtents te;

          string str = "%.1f>".printf(pl.d1);
          ctx.text_extents (str, out te);
          ctx.move_to(0.05 * w, te.height);
          ctx.show_text(str);

          for(uint i = 1; i < n - 1; i += step)
          {
            str = "%.1f>".printf(pl.d1 + (pl.d2 - pl.d1) / n * i);
            ctx.text_extents (str, out te);
            ctx.move_to(0.05 * w, (h / n) * i + (h / n) / 2 + te.height / 2);
            ctx.show_text(str);
          }

          str = "%.1f>".printf(pl.d2);
          ctx.text_extents (str, out te);
          ctx.move_to(0.05 * w, h);
          ctx.show_text(str);
        }
        return false;
      });

      frame_scale.add( darea);
      main_vbox.pack_start( frame_scale, true, true, 5);
      //Отрисовка цветовой шкалы на панели настроек <---

    }


    /**
     * Метод получения ссылки на виджет с настройками.
     * @return Виджет с настройками.
    */
    public unowned Widget? get_widget()
    {
      return this;
    }

    /**
     * Метод устанавливает название для виджета настроек.
     * @param name cтрока с названием виджета.
    */
    public void set_widget_name(string ? name)
    {
      this.palette_box_name = name;
    }

    /**
     * Метод возвращает строку с названием для виджета настроек.
     * @return Строка с названием виджета.
    */
    public unowned string? get_widget_name()
    {
      return this.palette_box_name;
    }

    /**
     * Метод сохранения настроек виджета в ini-файл.
     * @param kf Ini-файл.
    */
    public void config_save( KeyFile kf)
    {
      var gr = this.config_group();

      kf.set_integer(gr, "gradation", (int)this.pl.n);
      kf.set_integer(gr, "scale_type", (int)this.pl.cur_scale);
    }

    /**
     * Метод загрузки настроек виджета из ini-файла.
     * @param kf Ini-файл.
    */
    public void config_load( KeyFile kf) throws KeyFileError
    {
      var gr = this.config_group();

      try
      {
        this.pl.n = (uint)kf.get_integer(gr, "gradation");
        this.pl.cur_scale = (ScaleType)kf.get_integer(gr, "scale_type");
        config_loaded();
//~         if (this.shared_data != null)
//~           this.shared_data.set_acu_data( this.pl.n, (int)this.pl.cur_scale);
      }
      catch( KeyFileError e)
      {
//~         if (this.shared_data != null)
//~           this.shared_data.set_acu_data( this.pl.n, (int)this.pl.cur_scale);
      }
    }

  } //end class PaletteBox
}
