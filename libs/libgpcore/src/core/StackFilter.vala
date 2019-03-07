using GLib;

namespace Gp
{

  /*! Структура OneFilter хранит фильтр и его свойства в контексте StackFilter.*/
  public struct OneFilter
  {
    Filter filter;  /*!< Объект GpFilter.*/
    bool active;    /*!< Флаг активности фильтра.*/
  }

  /**
   * Объект StackFilter представляет собой реализацию стека фильтров.
   * Обработка данных производится фильтрами по порядку добавления в стек фильтров.
   * Параметры и настройки фильтров устанавливаются отдельно для каждого фильтра.
   */
  public class StackFilter : Object, Filter
  {
    private GenericArray<OneFilter?> filters;  /*! Массив указателей на фильтры (объекты типа OneFilter).*/


    /**
     * Создать объект стек фильтров.
     */
    public StackFilter()
    {
      filters = new GenericArray<OneFilter?> ();
    }

    /**
     * Добавить фильтр в стек.
     */
    public void add( Filter filter)
    {
      var oflt = OneFilter();
      oflt.filter = filter;
      oflt.active = true;

      filters.add( oflt);
    }

    /**
     * Удалить фильтр из стека.
     * Returns: TRUE в случае успеха -- если фильтр был найден и удален;
     * FALSE в случае ошибки, либо если фильтр не был найден в стеке.
     */
    public bool remove( Filter filter)
    {
      bool rval = false;

      filters.foreach(( oflt) => {
        if (oflt.filter == filter)
        {
          filters.remove( oflt);
          rval = true;
        }
      });

      return rval;
    }

    /**
     * Запрос количества фильтров в стеке.
     */
    public int get_amount()
    {
      return filters.length;
    }

    /**
     * Запрос максимального размера окна, установленного для активных фильтров.
     * Если в стеке нет активных фильтров с параметром "окно", возвращается 0.
     * Подразумевается, что метод учитывает свойство фильтра с именем "window" типа uint.
     */
    public int get_win_size()
    {
      uint max = 0;

      filters.foreach(( oflt) => {
        if (oflt.active == true)
        {
          Type tp = oflt.filter.get_type();
          ObjectClass ocl = (ObjectClass)tp.class_ref();
          foreach( ParamSpec spec in ocl.list_properties())
          {
            if (spec.get_name() == "window")
            {
              Value val = Value( typeof(uint));
              (oflt.filter as Object).get_property( spec.get_name(), ref val);
              max = uint.max( max, val.get_uint());
            }
          }
        }
      });

      return (int)max;
    }

    /**
     * Установить флаг активности фильтра filter из стека в значение active.
     * Если фильтр не активен, он не применяется к данным при фильтрации.
     * Returns: TRUE в случае успеха -- если фильтр был найден и его флаг установлен;
     * FALSE в случае ошибки, либо если фильтр не был найден в стеке.
     */
    public bool set_active( Filter filter, bool active)
    {
      bool rval = false;

      filters.foreach(( oflt) => {
        if (oflt.filter == filter)
        {
          oflt.active = active;
          rval = true;
        }
      });

      return rval;
    }


    /**
     * Получает массив исходных данных,
     * выполняет алгоритм фильтрации последовательно по всему установленному набору фильтров и
     * записывает данные в указанный массив.
     * Данный метод переопределяет одноименный из интерфейса GpFilter.
     */
    public void process( float[] input_data, float[] output_data, float discretization_frequency = 0, float transient_time = 0)
    {
      for( int i=0; i<input_data.length; i++) output_data[i] = input_data[i];
      if (filters.length == 0) return;

      var inbuf = new float[ input_data.length ];
      var outbuf = new float[ input_data.length ];
      var tmp = new float[ input_data.length ];

      for( int i=0; i<input_data.length; i++) outbuf[i] = input_data[i];

      filters.foreach(( oflt) => {
        if (oflt.active == true)
        {
          tmp = outbuf;
          outbuf = inbuf;
          inbuf = tmp;

          oflt.filter.process( inbuf, outbuf, discretization_frequency, transient_time);
        }
      });

      for( int i=0; i<input_data.length; i++) output_data[i] = outbuf[i];
    }


    public unowned OneFilter? get_filter( uint index)
    {
      unowned OneFilter? p = this.filters.get( index);
      return p;
    }

  } //class StackFilter

} //namespace Gp
