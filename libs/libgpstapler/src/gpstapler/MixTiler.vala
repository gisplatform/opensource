/*
 * MixTiler.vala
 *
 *  Created on: 01.08.17
 *  Author: Vodilov Andrey
 */

using Gtk;

namespace Gp
{
  public class MixTiler : Tiler
  {
    PaletteBox palette_pref = null;
    int[] provides_types = new int[0];

    struct Area
    {
      double from_x;
      double from_y;
      double to_x;
      double to_y;
    }

    private class Trunk<MemTile> trunk = new Trunk<MemTile>(() => { return new MemTile(); });

    public TreeModel main_stapler { get; construct; }
    Area area;

    public MixTiler(SmartCache cache_to_set, TreeModel stapler, int tile_types_num_to_set)
    {
      Object(cache : cache_to_set, tile_types_num : tile_types_num_to_set, main_stapler : stapler, name : "Tiles mixer");
      area = Area();
      area.from_x = 1000000000000000;
      area.from_y = 1000000000000000;
      area.to_x = 0;
      area.to_y = 0;

      TreeIter iter;
      bool iter_valid = false;

      if ((iter_valid = stapler.get_iter_first( out iter)) == true)
      {
        while(iter_valid)
        {
          Value val;
          stapler.get_value( iter, TilerTreeModelCols.TILER, out val);
          (val as Tiler).data_updated.connect(() =>
          {
            this.data_updated();
          });
          iter_valid = stapler.iter_next( ref iter);
        }
      }
    }

    public override bool is_graphical(int type)
        requires(type >= 0)
        requires(type < this.tile_types_num)
    {
        return true;
    }

    public override bool provides_tile_type(int type)
      requires(type >= 0)
      requires(type < this.tile_types_num)
    {
      for(int i = 0; i < provides_types.length; i++)
        if(type == provides_types[i])
          return true;

      return false;
    }
    public void set_palette(PaletteBox plt_pref)
    {
      this.palette_pref = plt_pref;
    }

    public void add_provides_type(int type)
      requires(type >= 0)
      requires(type < this.tile_types_num)
    {
      provides_types += type;
    }

    public bool determ_area()
    {
      TreeIter iter;
      bool iter_valid = false;
      double x_min, y_min, x_max, y_max;
      if (main_stapler.get_iter_first( out iter) == true)
      {
        iter_valid = main_stapler.iter_next( ref iter);
        while(iter_valid)
        {
          Value val;
          main_stapler.get_value( iter, TilerTreeModelCols.TILER, out val);
          (val as Tiler).get_area(6, out x_min, out x_max, out y_min, out y_max);
          print("CC AREA: %f\t%f\t%f\t%f\n", x_min, x_max, y_min, y_max);
          area.from_x = double.min(area.from_x, x_min);
          area.from_y = double.min(area.from_y, y_min);
          area.to_x = double.max(area.to_x, x_max);
          area.to_y = double.max(area.to_y, y_max);

          iter_valid = main_stapler.iter_next( ref iter);
        }
      }
      print("AREA: %f\t%f\t%f\t%f\n", area.from_x, area.to_x, area.from_y, area.to_y);

      return true;
    }

    public override bool get_area(int type, out double from_x, out double to_x, out double from_y, out double to_y)
    {
      if(type == 6)
      {
        from_x = area.from_x;
        from_y = area.from_y;
        to_x = area.to_x;
        to_y = area.to_y;
        print("GET AREA: %f\t%f\t%f\t%f\n", area.from_x, area.to_x, area.from_y, area.to_y);
        return true;
      }
      else
      {
        from_x = 0;
        from_y = 0;
        to_x = 1;
        to_y = 1;
        return false;
      }
    }

    /**
    * Метод получения плитки из очереди this.done_tiles.
    * Может вернуть и плитку, отличную от required_tile.
    *
    * @param required_tile требуемая плитка.
    * @param required_status Требуется плитка со статусом отрисовки более указанного.
    *
    * @return Данные плитки (актуальные или нет), либо null, если данных для плитки пока еще нет.
    */
    protected override Gp.MemTile? get_tile_from_source(Gp.Tile required_tile, Gp.TileStatus required_status)
    {
      int visible_tilers = 0;
      TileStatus res_status = 0;
      var guards = new Gp.Trunk.Guard<MemTile>[0]; //< Отрисованные хоть как-то плитки.

      TreeIter iter;
      if(main_stapler.get_iter_first(out iter) == true)
      {
        do
        {
          Value val;
          main_stapler.get_value( iter, TilerTreeModelCols.VISIBLE, out val);

          if((bool)val == true)
          {
            main_stapler.get_value( iter, TilerTreeModelCols.TILER, out val);

            if((val as Tiler).is_graphical(required_tile.type) == false)
            {
              Trunk.Guard? cur_tiler_guard = null;
              MemTile cur_mem_tile = trunk.access(ref cur_tiler_guard);

              // FIXME: По идее не совсем верно передавать сюда required_status,
              // корректней было б передать статус плитки от конкретно этого Tiler'а,
              // полученной на предыдущей итерации. Но хранить все эти статусы сложно.
              cur_mem_tile.status = (val as Tiler).get_tile(cur_mem_tile.get_buf(), required_tile, required_status);

              if(cur_mem_tile.status > TileStatus.NOT_INIT)
              {
                guards += cur_tiler_guard;
                res_status +=  cur_mem_tile.status;
              }

              visible_tilers++;
            }
          }
        }
        while(main_stapler.iter_next(ref iter));
      }

      // Вернем пустую готовую плитку, если просто не из чего миксовать.
      if(visible_tilers == 0)
          return new MemTile.with_tile(required_tile, TileStatus.ACTUAL);

      // Если в теории есть из чего миксовать, но пока нет ни одной хоть как-то готовой плитки.
      if(guards.length == 0)
        return null;

      res_status = res_status / visible_tilers;

      if(res_status <= required_status)
        return null;

      MemTile res_tile = new MemTile();
      res_tile.status = res_status;
      res_tile.tile = required_tile;
      unowned uint8[] res_buf = res_tile.get_buf();
      uint16 val = 0;
      float buf_val = 0;
      uint64 buf_int[Gp.TILE_DATA_SIZE / 4];
      uint64 median_buf_int[Gp.TILE_DATA_SIZE / 4];
      int64 count_data[Gp.TILE_DATA_SIZE / 4];
      int n = 0;

      for(int i = 0; i < Gp.TILE_DATA_SIZE / 4; i++)
      {
        buf_int[i] = 0;
        count_data[i] = 0;
      }

      // Миксование.

      foreach(var g in guards)
      {
        unowned uint8[] cur_buf = trunk.access(ref g).get_buf();
        n = 0;

        for(int i = 0; i < Gp.TILE_DATA_SIZE - 4; i += 4)
        {
          switch(this.palette_pref.type)
          {
            case PaletteType.SIMPLE:
            {
              if(cur_buf[i + 0] > res_buf[i + 0])
              {
                res_buf[i + 0] = cur_buf[i + 0];
                res_buf[i + 1] = cur_buf[i + 1];
                res_buf[i + 2] = cur_buf[i + 2];
                res_buf[i + 3] = 0;
              }
            }
            break;
            case PaletteType.ADVANCED:
            {
              if(cur_buf[i + 0] > 0)
              {
                *((uint8*)(&val) + 0) = cur_buf[i + 1];
                *((uint8*)(&val) + 1) = cur_buf[i + 2];

                if(val / 10 > this.palette_pref.get_palette().d1 && val / 10 < this.palette_pref.get_palette().d2)
                {
                  switch(cur_buf[i + 0])
                  {
                    case 3:
                    {
                      if(count_data[n] == 0)
                      {
                        buf_int[n] = val;
                        count_data[n] = 100000;
                        median_buf_int[n] = 0;
                      }
                      else
                      {
                        if(count_data[n] == 100000)
                        {
                          if(median_buf_int[n] == 0)
                          {
                            median_buf_int[n] = val;
                          }
                          else
                          {
                            buf_int[n] = (buf_int[n] <= median_buf_int[n] && median_buf_int[n] <= val) ? median_buf_int[n] : ((buf_int[n] <= val && val <= median_buf_int[n]) ? val : buf_int[n]);
                            median_buf_int[n] = 0;
                          }
                        }
                      }
                    }
                    break;
                    case 2:
                    {
                      if(count_data[n] == 0 || count_data[n] == 100000)
                      {
                        buf_int[n] = val;
                        count_data[n] = 1;
                        median_buf_int[n] = 0;
                      }
                      else
                      {
                        if(count_data[n] > 0)
                        {
                          if(median_buf_int[n] == 0)
                          {
                            median_buf_int[n] = val;
                          }
                          else
                          {
                            buf_int[n] = (buf_int[n] <= median_buf_int[n] && median_buf_int[n] <= val) ? median_buf_int[n] : ((buf_int[n] <= val && val <= median_buf_int[n]) ? val : buf_int[n]);
                            median_buf_int[n] = 0;
                          }
                        }
                      }
//~                       if(count_data[n] >= 0)
//~                       {
//~                         buf_int[n] += val;
//~                         count_data[n] += 1;
//~                       }
//~                       else
//~                         if(count_data[n] == 100000)
//~                         {
//~                           buf_int[n] = val;
//~                           count_data[n] = 1;
//~                         }
                    }
                    break;
                    case 1:
                    {
                      if(count_data[n] >= 0)
                      {
                        buf_int[n] = val;
                        count_data[n] = -1;
                        median_buf_int[n] = 0;
                      }
                      else
                      {
                        if(median_buf_int[n] == 0)
                        {
                          median_buf_int[n] = val;
                        }
                        else
                        {
                          buf_int[n] = (buf_int[n] <= median_buf_int[n] && median_buf_int[n] <= val) ? median_buf_int[n] : ((buf_int[n] <= val && val <= median_buf_int[n]) ? val : buf_int[n]);
                          median_buf_int[n] = 0;
                        }
                      }
                      /*
                      if(count_data[n] >= 0)
                      {
                        buf_int[n] = val;
                        count_data[n] = -1;
                      }
                      else
                      {
                        buf_int[n] += val;
                        count_data[n] += -1;
                      }*/
                    }
                    break;
                  }
                }
              }
              n++;
            }
            break;
            default:
            break;
          }
        }
      }

      if(this.palette_pref.type == PaletteType.ADVANCED)
      {
        n = 0;
        for(int i = 0; i < Gp.TILE_DATA_SIZE - 4; i += 4)
        {
          if(count_data[n] != 0)
          {
            if(median_buf_int[n] == 0)
              val = (uint16)(buf_int[n]);// Math.fabs(count_data[n]));
            else
              val = (uint16)((buf_int[n] + median_buf_int[n]) / 2);
          }
          else
            val = 0;

          n++;
          res_buf[i + 1] = *((uint8*)(&val) + 0);
          res_buf[i + 2] = *((uint8*)(&val) + 1);
        }
      }
      // Превращение замиксованных данных в цветные пиксели.
      uint32 color = 0;

      for(int i = 0; i < Gp.TILE_DATA_SIZE - 4; i += 4)
      {
        val = 0;
        if(this.palette_pref != null)
        {
          Palette cur_plt = this.palette_pref.get_palette();

          *((uint8*)(&val) + 0) = res_buf[i + 1];
          *((uint8*)(&val) + 1) = res_buf[i + 2];

          if(val > 0)
          {
            switch(this.palette_pref.type)
            {
              case PaletteType.SIMPLE:
              {
                cur_plt.d1 = 0;
                cur_plt.d2 = 255;
                uint max = (cur_plt.n == 0) ? 255 : cur_plt.n;
                buf_val = (max - val / 256) * 255 / max;
              }
              break;
              case PaletteType.ADVANCED:
              {
                buf_val = (float)val / 10;
              }
              break;
              default:
              break;
            }
            color = cur_plt.get_pixel(buf_val, this.palette_pref.type);
          }
        }

        if(val > 0 && color != 0)
        {
          res_buf[i + 0] = *((uint8*)(&color) + 0);
          res_buf[i + 1] = *((uint8*)(&color) + 1);
          res_buf[i + 2] = *((uint8*)(&color) + 2);
          res_buf[i + 3] = uint8.MAX;
        }
        else
        {
          res_buf[i + 0] = 0;
          res_buf[i + 1] = 0;
          res_buf[i + 2] = 0;
          res_buf[i + 3] = 0;
        }
      }

      return res_tile;
    }
  }
}

