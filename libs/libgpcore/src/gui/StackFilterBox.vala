using Gtk;
using GLib;
using Cairo;


namespace Gp
{
  #if VALA_0_26
  public class StackFilterBox : Gp.Barista
  {
    private StackFilter stack = null;
    private Type[] types = null;  //Доступные типы фильтров
    private int type_id = 0;      //Тип фильтра из списка, 0 - не выбран

    private Gtk.Stack wdg_stk = null;  //Переключатель режимов настройки
    private Box common_vbox = null;    //Вкладка ручной настройки, сюда добавляем фреймы с фильтрами
    private Box auto_vbox = null;      //Вкладка авто-настройки
    private Scale auto_scale = null;   //Шкала авто-настройки

    public const uint SERIALIZE_API_VERSION = 2; // Версия формата сериализации.

    private bool auto_mode = false;   //Признак использования авто-фильтра
    private float auto_param = 0.0f;  //Значение шкалы авто-фильтра (0-100), 0 - авто-фильтр не используется
    private const float AUTO_PARAM_MAX = 100.0f;  //Максимальное значение на шкале авто-настройки


    public bool set_auto_param(float p)
    {
      if (p < 0.0 || p > AUTO_PARAM_MAX) return false;
      auto_param = p;
      return true;
    }


    /**
    * Сериализация.
    * Формат 1 -- API_VERSION::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
    * Формат 2 -- API_VERSION::AUTO_MODE:AUTO_PARAM::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
    */
    public string to_string()
    {
      string rval = SERIALIZE_API_VERSION.to_string();

      auto_mode = (wdg_stk.get_visible_child() == auto_vbox);

      if (SERIALIZE_API_VERSION > 1)
        rval += "::" + auto_mode.to_string() + ":" + ((uint)auto_param).to_string();

      for( uint i=0; i<stack.get_amount(); i++)
      {
        unowned OneFilter? oflt = stack.get_filter( i);

        Type tp = oflt.filter.get_type();
        rval += "::" + tp.name() + ":" + oflt.active.to_string();

        ObjectClass ocl = (ObjectClass)tp.class_ref();
        foreach( ParamSpec spec in ocl.list_properties())
        {
          Value val;
          if (spec.value_type == typeof(bool))
          {
            val = Value( typeof(bool));
            (oflt.filter as Object).get_property( spec.get_name(), ref val);
            rval += ":" + val.get_boolean().to_string();
          }
          else
          {
            val = Value( typeof(float));
            (oflt.filter as Object).get_property( spec.get_name(), ref val);
            rval += ":" + val.get_float().to_string();
          }
        }
      }

      return rval;
    }


    /**
     * Метод разбирает строку параметров из ini-файла
     * Метод не зависит от Gtk, можно использовать в любом потоке.
     *
     * @param p строка в формате:
     * 1 -- API_VERSION::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
     * 2 -- API_VERSION::AUTO_MODE:AUTO_PARAM::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
     * @param outstack - стэк-фильтр ручной настройки.
     * @param auto - стэк-фильтр авто-настройки.
     * @param param - коэффициент авто-усиления (0-100), 0 - авто-фильтрование применено не будет.
     **/
    public static bool static_parse(string? p, out StackFilter outstack, out bool auto, out float param)
    {
      outstack = new StackFilter();
      auto = false;
      param = 0;

      if (p == null) return false;

      uint version = 0;
      int cnt = 0;
      string str = p;

      while( true)
      {
        string[] lines = str.split("::",2);
        str = lines[0];

        if (cnt == 0)
        {
          //SERIALIZE_API_VERSION
          str.scanf( "%u", out version);
          if (version > SERIALIZE_API_VERSION)
          {
            warning( "Unknown filter settings API version: %u", version);
            break;
          }
        }
        else if (cnt == 1 && version == SERIALIZE_API_VERSION)
        {
          //Настройки авто-фильтра
          string[] param_str = new string[0];

          while( true)
          {
            string[] params = str.split(":",2);
            param_str += params[0];
            if (params[1] == null) break;
            str = params[1];
          }

          if (param_str.length > 0)
          {
            auto = bool.parse( param_str[0]);
            if (param_str.length > 1)
            {
              uint d;
              param_str[1].scanf( "%u", out d);
              param = float.min((float)d, AUTO_PARAM_MAX);
            }
          }
        }
        else
        {
            // ---> Разбираем кусок строки для отдельного фильтра
            string[] param_str = new string[0];

            while( true)
            {
              string[] params = str.split(":",2);
              param_str += params[0];
              if (params[1] == null) break;
              str = params[1];
            }
            // <--- Разбираем кусок строки для отдельного фильтра

            // ---> Создаем фильтр и размещаем группу виджетов на панели
            if (param_str.length > 0)
            {
              var type = Type.from_name(param_str[0]);

              if(type != 0)
              {
                var obj = Object.@new(type);

                bool active = false;
                if (param_str.length > 1) active = bool.parse( param_str[1]);
                int icnt = 2;

                Type tp = obj.get_type();
                ObjectClass ocl = (ObjectClass)tp.class_ref();

                foreach( ParamSpec spec in ocl.list_properties())
                {
                  if (icnt >= param_str.length) break;
                  if (spec.value_type == typeof(bool))
                  {
                    obj.set_property( spec.get_name(), bool.parse( param_str[icnt]));
                  }
                  else
                  {
                    float pf;
                    param_str[icnt].scanf( "%f", out pf);
                    obj.set_property( spec.get_name(), pf);
                  }
                  icnt++;
                }

                outstack.add( obj as Filter);
                outstack.set_active( obj as Filter, active);
              }
              else
                warning("Unknown type: %s", param_str[0]);
            }
            // <--- Создаем фильтр и размещаем группу виджетов на панели
        }

        if (lines[1] == null) break;
        str = lines[1];
        cnt++;
      }

      return true;
    }


    /**
     * Метод разбирает строку параметров из ini-файла
     *
     * @param p строка в формате:
     * 1 -- API_VERSION::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
     * 2 -- API_VERSION::AUTO_MODE:AUTO_PARAM::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2::FILTER_CLASS_NAME:ACTIVE:PARAM1:PARAM2
     **/
    public bool parse(string? p)
    {
      StackFilter outstack;
      bool mode;
      float param;

      if(static_parse(p, out outstack, out mode, out param) == false)
        return false;

      this.stack = outstack;
      this.auto_mode = mode;
      this.auto_param = param;

      return true;
    }


    public bool stack_to_filters(StackFilter? sflt = null)
    {
      if (sflt != null) stack = sflt;

      //Ручные настроечки
      for( uint i=0; i<stack.get_amount(); i++)
      {
        unowned OneFilter? oflt = stack.get_filter( i);
        if (oflt != null) map_filter( oflt.filter as Object, oflt.active);
      }

      //Авто-настроечки
      if (auto_mode == true)
        wdg_stk.set_visible_child( auto_vbox);
      else
        wdg_stk.set_visible_child( common_vbox);

      auto_scale.set_value( auto_param);

      this.show_all();

      return true;
    }


    //Разместить на панели группу виджетов для одного фильтра
    private void map_filter(Object obj, bool active)
    {
      Type tp = obj.get_type();

      var filter_frame = new Frame( (obj as Filter).get_name());
      common_vbox.pack_start( filter_frame, false, false, 1);
      var filter_hbox = new Box( Orientation.HORIZONTAL, 0);
      filter_frame.add( filter_hbox);

      var vbx_1 = new Box( Orientation.VERTICAL, 0);
      filter_hbox.pack_start( vbx_1, false, false, 1);
      var vbx_2 = new Box( Orientation.VERTICAL, 0);
      filter_hbox.pack_start( vbx_2, true, true, 1);
      var vbx_3 = new Box( Orientation.VERTICAL, 0);
      filter_hbox.pack_end( vbx_3, false, false, 1);

      int padding = 0;

      ObjectClass ocl = (ObjectClass)tp.class_ref();
      if (ocl.list_properties().length != 0) padding = 10;

      //Кнопка "Удалить выбранный фильтр"
      var rm_btn = new Button.with_label("-");
      rm_btn.clicked.connect(() => {
        stack.remove( obj as Filter);
        filter_frame.destroy();
      });
      vbx_1.pack_start( rm_btn, false, false, padding);

      //print("%s\n", tp.name());

      //Установка параметров для фильтра
      foreach( ParamSpec spec in ocl.list_properties())
      {
        //print("%s : %s\n", spec.get_name(), spec.value_type.name());
        if(spec.get_name() != "max-val")
        {
          if (spec.value_type == typeof(bool))
          {
            var tb = new ToggleButton.with_label( spec.get_name());

            Value val = Value( typeof(bool));
            obj.get_property( spec.get_name(), ref val);
            tb.active = (bool)val;

            tb.toggled.connect(() => {
              obj.set_property( spec.get_name(), tb.active);
            });

            vbx_2.pack_start( tb, false, false, 5);
            continue;
          }

          var spec_frm = new Frame( _(spec.get_nick()));
          vbx_2.pack_start( spec_frm, false, false, 0);

          SpinButton sp = null;

          if (spec.value_type == typeof(double))
          {
            sp = new SpinButton.with_range( ((ParamSpecDouble*)spec)->minimum, double.min( ((ParamSpecDouble*)spec)->maximum, 65535.0), 0.05);
            sp.set_increments( 0.05, 1.0);
            sp.set_digits( 2);
          }
          if (spec.value_type == typeof(float))
          {
            sp = new SpinButton.with_range( ((ParamSpecFloat*)spec)->minimum, float.min( ((ParamSpecFloat*)spec)->maximum, 65535), 0.05);
            sp.set_increments( 0.05, 1.0);
            sp.set_digits( 2);
          }
          else if (spec.value_type == typeof(int))
          {
            sp = new SpinButton.with_range( ((ParamSpecInt*)spec)->minimum, ((ParamSpecInt*)spec)->maximum, 1);
            sp.set_increments( 1, 10);
            sp.set_digits( 0);
          }
          else if (spec.value_type == typeof(uint))
          {
            sp = new SpinButton.with_range( ((ParamSpecUInt*)spec)->minimum, ((ParamSpecUInt*)spec)->maximum, 1);
            sp.set_increments( 1, 10);
            sp.set_digits( 0);
          }
          else
          {
            warning( "Unexpected property type: %s", spec.value_type.name());
            continue;
          }

          Value val = Value( typeof(double));
          obj.get_property( spec.get_name(), ref val);
          sp.set_value( val.get_double());

          sp.value_changed.connect(() => {
            obj.set_property( spec.get_name(), sp.get_value());
          });

          spec_frm.add( sp);
        }
      }

      //Чек-бокс применить/отменить использование фильтра
      var cbx = new CheckButton();
      cbx.active = active;
      vbx_2.set_sensitive( cbx.active);

      cbx.toggled.connect(() => {
        stack.set_active( obj as Filter, cbx.active);
        vbx_2.set_sensitive( cbx.active);
      });
      vbx_3.pack_start( cbx, false, false, padding);

    }


    // Конструктор
    public StackFilterBox(Type[]? filter_types)
    {
      types = filter_types;
      stack = new StackFilter();
      auto_param = 0;
      auto_mode = true;

      var top_vbox = new Box( Orientation.VERTICAL, 0);
      var sw_vbox = new Box( Orientation.HORIZONTAL, 0);
      sw_vbox.set_halign(Align.CENTER);
      top_vbox.pack_start( sw_vbox, false, false, 5);

      this.scrolled.set_policy( PolicyType.NEVER, PolicyType.AUTOMATIC);
      this.scrolled.add( top_vbox);

      //Стек-переключатель
      wdg_stk = new Gtk.Stack();
      wdg_stk.set_transition_type( StackTransitionType.OVER_LEFT_RIGHT);
      var switcher = new Gtk.StackSwitcher();
      switcher.set_stack( wdg_stk);
      top_vbox.pack_start( wdg_stk, false, false, 5);
      sw_vbox.pack_start( switcher, false, false, 5);

      //Вкладка "Авто-управление"
      auto_vbox = new Box( Orientation.VERTICAL, 0);
      wdg_stk.add_titled( auto_vbox, "Auto", _("Auto"));

      auto_scale = new Scale.with_range( Orientation.HORIZONTAL, 0.0, AUTO_PARAM_MAX, 1.0);
      auto_scale.set_value( auto_param);
      auto_scale.value_changed.connect(() => {
        auto_param = (float)auto_scale.get_value();
      });
      auto_vbox.pack_start( auto_scale, false, false, 1);

      //Вкладка "Ручное управление" --->
      common_vbox = new Box( Orientation.VERTICAL, 0);
      wdg_stk.add_titled( common_vbox, "Manual", _("Manual"));

      if (auto_mode == true)
        wdg_stk.set_visible_child( auto_vbox);
      else
        wdg_stk.set_visible_child( common_vbox);

      //Меню добавления фильтров
      var frame = new Frame( _("Add Filter"));
      common_vbox.pack_start( frame, false, false, 5);
      var hbox = new Box( Orientation.HORIZONTAL, 0);
      frame.add( hbox);

      var ls = new Gtk.ListStore(2, typeof(string), typeof(int));
      if (types.length != 0)
      {
        TreeIter it;
        for( int i=0; i<types.length; i++)
        {
          var obj = Object.@new( types[i]);
          ls.append( out it);
          ls.set( it, 0, (obj as Filter).get_name(), 1, i+1);
        }
        type_id = 1;
      }

      //Комбо-бокс для выбора нового фильтра
      var type_combo_box = new ComboBox.with_model( ls);
      var renderer = new CellRendererText();
      type_combo_box.pack_start( renderer, true);
      type_combo_box.add_attribute( renderer, "text", 0);
      if (type_id != 0) type_combo_box.active = type_id-1;
      hbox.pack_start( type_combo_box, true, true, 1);

      type_combo_box.changed.connect(() => {
        Value val;
        TreeIter it;
        type_combo_box.get_active_iter( out it);
        ls.get_value( it, 1, out val);
        type_id = (int)val;
      });

      //Кнопка "Добавить фильтр"
      var add_fltr_btn = new Button.with_label("+");
      hbox.pack_end( add_fltr_btn, false, false, 1);

      add_fltr_btn.clicked.connect(() => {
        if (type_id != 0)
        {
          Type tp = types[type_id-1];
          var obj = Object.@new( tp);
          stack.add( obj as Filter);
          map_filter( obj, true);
          this.show_all();
        }
      });

    }

  } //end class StackFilterBox
  #endif
} //end namespace
