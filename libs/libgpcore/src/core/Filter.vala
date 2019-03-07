using GLib;

namespace Gp
{

  /**
   * Интерфейс фильтра данных.
   * Интерфейс GpFilter предназначен для создания фильтров как одномерных, так и двумерных (?) данных.
   *
   * set_param - устанавливает параметр для фильтра.
   * get_param - читает параметр фильтра.
   * process - выполняет алгоритм фильтрации и возвращает отфильтрованные данные..
   */
  public interface Filter : Object
  {
    /**
     * Получает массив исходных данных, фильтрует и записывает данные в указанный массив.
     *
     * input_data - массив исходных данных
     * output_data - массив отфильтрованных данных
     * discretization_frequency - частота дискретизации
     * transient_time - время переходного процесса
     */
    public virtual void process( float[] input_data, float[] output_data, float discretization_frequency = 0, float transient_time = 0)
    {
      warning("Unrealized virtual method 'process' in class '%s'", this.get_type().name());
    }

    /**
     * Запрос количества фильтров.
     */
    public virtual int get_amount()
    {
      return 1;
    }

    /**
     * Запрос названия фильтра.
     */
    public virtual unowned string get_name()
    {
      return _("<no_name>");
    }

    /**
     * Запрос подсказки.
     */
    public virtual string get_hint()
    {
      return _("<no_hint>");
    }

    /**
     * Установка параметра для фильтра.
     * Параметр будет установлен, только если в объекте он единственный,
     * иначе выдается предупреждение.
     */
    public virtual void set_param( float p0)
    {
      Type tp = this.get_type();
      ObjectClass ocl = (ObjectClass)tp.class_ref();

      if (ocl.list_properties().length == 1)
      {
        foreach( ParamSpec spec in ocl.list_properties())
        {
          (this as Object).set_property( spec.get_name(), p0);
        }
      }
      else
      {
        warning("set_param(): class '%s' have %d parameters", tp.name(), ocl.list_properties().length);
      }

    }

    /**
     * Запрос параметра для фильтра.
     * Параметр будет считан, только если в объекте он единственный,
     * иначе выдается предупреждение и возвращается 0.
     */
    public virtual float get_param()
    {
      float rval = 0;
      Type tp = this.get_type();
      ObjectClass ocl = (ObjectClass)tp.class_ref();

      if (ocl.list_properties().length == 1)
      {
        var ps = ocl.list_properties();
        Value val = Value( typeof(float));
        (this as Object).get_property( ps[0].get_name(), ref val);
        rval = val.get_float();
      }
      else
      {
        warning("get_param(): class '%s' have %d parameters", tp.name(), ocl.list_properties().length);
      }

      return rval;
    }

  } //interface Filter

} //namespace Gp

