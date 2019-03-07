/*
 * GpRumorista is an async sound effect library.
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * GpRumorista is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpRumorista is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpRumorista. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using PulseAudio;

namespace Gp
{
/*
* Объект для воспроизведения звуковых эффектов в программе.
* Rumorista -- так по-итальянски называют специалиста по записи шумовых эффектов для кино.
* Каждому эффекту назначен приоритет, приоритет является одновременно идентификатором эффекта.
* Эффекты можно проигрывать методом play.
* Если в данный момент уже играет какой-то эффект, то новый будет положен в очередь.
* Если после воспроизведения эффекта в очереди есть еще один или более,
* будет воспроизведен только один с наивысшим приоритетом.
* Т.е. несколько вызовов play подряд без пауз приведут к воспроизведению только одного эффекта,
* при этом эффект с наивысшим приоритетом будет гарантированно воспроизведен.
* Объект воспроизводит эффекты в отдельном потоке, т.е. метод play -- неблокирующий.
* В данной версии доступны следующие эффекты:
* Priority.LOW, Priority.DEFAULT и Priority.HIGH.
*/
public class Rumorista : Object
{
  Simple? simple;
  Thread<int> thread;
  AsyncQueue<int?> to_play = new AsyncQueue<int?>();
  HashTable<int, ByteArray> effects = new HashTable<int, ByteArray>(direct_hash, direct_equal);

  construct
  {
    int pulse_error;
    string rumorista_uniq_string = "Rumorista [%p]".printf(this);

    /* The Sample format to use */
    var spec = SampleSpec();
    spec.format = SampleFormat.S16LE;
    spec.rate = 44100;
    spec.channels = 1;

    /* Create a new playback stream */
    simple = new Simple(null, rumorista_uniq_string, Stream.Direction.PLAYBACK,
      null, "Rumorista", spec, null, null, out pulse_error);

    try
    {
      if(simple == null)
        throw new IOError.FAILED("new Simple failed: " + GLib.strerror(pulse_error));

      load_effect(Priority.LOW,     "/ru/gis-platform/libgpcore/dist/lebaston-click.wav");
      load_effect(Priority.DEFAULT, "/ru/gis-platform/libgpcore/dist/wine-glass.wav");
      load_effect(Priority.HIGH,    "/ru/gis-platform/libgpcore/dist/steady-heart.wav");

      thread = new Thread<int>.try(rumorista_uniq_string, run);
    }
    catch(GLib.Error e)
    {
      critical("Failed to start Rumorista: %s", e.message);
    }
  }

  private int run()
  {
    int pulse_error;

    while(true)
    {
      // Ждем из очереди эффектов.
      int id = to_play.pop();

      // Может в очереди есть более приоритетный эффект? Тогда воспроизведем именно его!
      int? new_id;
      while((new_id = to_play.try_pop()) != null)
        if(new_id < id)
          id = new_id;

      int unboxed_id = id;
      ByteArray? wav = effects.lookup(unboxed_id);
      return_val_if_fail(wav != null, -1);

      if(simple.write(wav.data, wav.len, out pulse_error) < 0)
      {
        critical("simple write failed: %s", GLib.strerror(pulse_error));
        return -2;
      }

      if(simple.drain(out pulse_error) < 0)
      {
        critical("simple drain failed: %s", GLib.strerror(pulse_error));
        return -3;
      }
    }
  }

  void load_effect(int id, string resource_path) throws GLib.Error
  {
    var wav_stream = resources_open_stream(resource_path, ResourceLookupFlags.NONE);
    var wav = new ByteArray();

    ssize_t r;
    uint8 portion[1024];
    while((r = wav_stream.read(portion)) > 0)
      wav.append(portion[0 : r]);

    effects.insert(id, wav);
  }


  /**
  * Воспроизведение эффекта.
  * @param id Идентификатор = приоритет эффекта.
  */
  public void play(int id)
  {
    to_play.push(id);
  }
} //< public class Rumorista
} //< namespace Gp

