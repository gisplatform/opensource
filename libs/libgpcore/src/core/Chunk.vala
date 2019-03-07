/*
 * GpChunk is a data storage with built-in header and optional compression.
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * GpChunk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpChunk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpChunk. If not, see <http://www.gnu.org/licenses/>.
 *
*/



namespace Gp
{
/**
* Объект хранит пачку данных, предназначенную для приема/отправки/другой обработки.
* Пачка данных предваряется 4-байтным заголовком, представляющем собой big-endian 32-битное число.
* Модуль числа означает размер данных.
* Если число отрицательное, то данные сжаты алгоритмом ZLib,
* если число положительное, то данные не сжаты.
*/
[Compact]
public class Chunk
{
  /**
  * Размер заголовка данных (заголовок -- число int32).
  */
  public const uint HEADER_SIZE = 4;

  /**
  * Пачка данных: заголовок + пользовательские данные единым массивом.
  */
  public uint8[] data;

  /**
  * Вспомогательное поле. Предназначено для хранения числа уже обработанных байт из папки
  * (например, переданных по сети или записанных в файл или устройство).
  */
  public size_t bytes_processed = 0;

  private static ZlibCompressor? compressor = null;
  private static ZlibDecompressor? decompressor = null;

  /**
  * Создает объект, заголовок не заполняет.
  * @param user_size Размер пользовательских данных, не включая заголовок.
  */
  public Chunk.sized(uint user_size)
    requires(user_size < (int32.MAX - HEADER_SIZE))
  {
    data = new uint8[user_size + HEADER_SIZE];
  }

  /**
  * Создает объект, заполняет пользовательскими данными и корректно выставляет заголовок.
  * @param user_buffer Непожатое сообщение, чисто полезные данные, без заголовка.
  */
  public Chunk.with_data(uint8[] user_buffer)
    requires(user_buffer.length < (int32.MAX - HEADER_SIZE))
  {
    data = new uint8[user_buffer.length + HEADER_SIZE];

    int32 header_in_int = user_buffer.length;
    header_in_int = header_in_int.to_big_endian();
    Memory.copy(data, &header_in_int, HEADER_SIZE);

    Memory.copy(((uint8*)this.data) + HEADER_SIZE, user_buffer, user_buffer.length);
  }

  /**
  * Создает объект, заполняет пользовательскими данными и корректно выставляет заголовок.
  * @param str Непожатое сообщение в виде null-terminated строки, чисто полезные данные, без заголовка.
  */
  public Chunk.with_string(string str)
    requires((str.length + 1) < (int32.MAX - HEADER_SIZE))
  {
    var user_data_size = str.length + 1;
    data = new uint8[user_data_size + HEADER_SIZE];

    int32 header_in_int = user_data_size;
    header_in_int = header_in_int.to_big_endian();
    Memory.copy(data, &header_in_int, HEADER_SIZE);

    Memory.copy(((uint8*)this.data) + HEADER_SIZE, str, user_data_size);
  }

  /**
  * Метод позволяет получить MD5-сумму пользовательских данных.
  */
  public string get_user_data_md5sum()
  {
    return Checksum.compute_for_data(ChecksumType.MD5, data[HEADER_SIZE:data.length]);
  }

  /**
  * Метод позволяет получить размер данных, зашитый в заголовке.
  */
  public uint get_user_data_size_from_header()
  {
    if(unlikely(data.length < HEADER_SIZE))
      return 0;

    int32 size = 0;
    Memory.copy(&size, data, HEADER_SIZE);
    size = int32.from_big_endian(size);

    return (size < 0) ? -size : size;
  }

  /**
  * Метод позволяет получить флаг сжатия данных по заголовку.
  */
  public bool get_compress_flag_from_header()
  {
    if(unlikely(data.length < HEADER_SIZE))
      return false;

    int32 header_in_int = 0;
    Memory.copy(&header_in_int, data, HEADER_SIZE);
    header_in_int = int32.from_big_endian(header_in_int);

    return (header_in_int < 0) ? true : false;
  }

  /**
  * Сжимает или распаковывает данные, корректно заполняет заголовок.
  * Также предполагает, что в заголовке изначально корректно заполнен флаг сжатия.
  * @param compress Сжать данные, если true; распаковать, если false.
  * @return true в случае успеха, либо если делать нечего, на это две возможные причины:
  * в массиве нет пользовательских данных, либо они уже сжаты/распакованы.
  */
  public bool deflate(bool compress) throws Error
  {
    if(get_compress_flag_from_header() == compress || data.length <= HEADER_SIZE)
      return true;

    Converter converter;

    if(compress)
      converter = compressor   ?? (  compressor = new   ZlibCompressor(ZlibCompressorFormat.RAW));
    else
      converter = decompressor ?? (decompressor = new ZlibDecompressor(ZlibCompressorFormat.RAW));

    size_t bytes_read, bytes_written = 0;
    size_t total_bytes_read = 0, total_bytes_written = 0;
    ConverterResult conv_rval = ConverterResult.ERROR;

    var new_data = new uint8[data.length];

    converter.reset();

    while(conv_rval != ConverterResult.FINISHED)
      try
      {
        if(HEADER_SIZE + total_bytes_written == new_data.length)
          new_data.resize(2 * new_data.length); //< Не хватило места -- увеличим буфер в два раза.

        conv_rval = converter.convert(
              data[HEADER_SIZE + total_bytes_read:        data.length],
          new_data[HEADER_SIZE + total_bytes_written: new_data.length],
          ConverterFlags.INPUT_AT_END, out bytes_read, out bytes_written);
        total_bytes_read += bytes_read;
        total_bytes_written += bytes_written;
      }
      catch(Error e)
      {
        if(e is IOError.NO_SPACE)
          new_data.resize(2 * new_data.length); //< Не хватило места -- увеличим буфер в два раза.
        else
          throw e;
      }

    new_data.resize((int)(HEADER_SIZE + total_bytes_written));

    int32 header_in_int = (int)total_bytes_written;
    if(compress) header_in_int = -header_in_int;
    header_in_int = header_in_int.to_big_endian();
    Memory.copy(new_data, &header_in_int, HEADER_SIZE);

    data = new_data;

    return true;
  }

  /**
  * Печатает на экран данные из Chunk в hex-виде.
  * @param columns Количество колонок.
  */
  public void hexdump(uint columns)
  {
    uint i;
    print("HEADER: ");

    for(i = 0; i < uint.min(data.length, HEADER_SIZE); i++)
      print("[%2x] ", data[i]);

    if(data.length < HEADER_SIZE)
    {
      warning("<-- HEADER IS NOT FULL (only %u bytes)\n", data.length);
      return;
    }

    print("\n");

    uint user_data_size = data.length - HEADER_SIZE;
    uint user_data_size_from_header = get_user_data_size_from_header();

    print("COMPRESS FLAG FROM HEADER: %s\n", get_compress_flag_from_header().to_string());
    print("USER DATA SIZE FROM HEADER: %u\n", user_data_size_from_header);
    print("USER DATA SIZE FROM DATA AR: %u\n", user_data_size);

    if(user_data_size == user_data_size_from_header)
      print("HEADER LOOKS VALID!\n");
    else
      warning("<-- HEADER IS INVALID");

    print("USER DATA MD5: %s\n", get_user_data_md5sum());

    print("USER DATA:\n");

    for(i = 0; i < user_data_size; i++)
    {
      print("[%2x] ", data[HEADER_SIZE + i]);

      if((i + 1) % columns == 0)
        print("\n");
    }

    if(i  % columns != 0)
      print("\n");
  }
} //< public class Chunk
} //< namespace Gp

