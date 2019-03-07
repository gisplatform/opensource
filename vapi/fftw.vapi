/*
 * fftw.vapi is a file with libfftw bindings for vala
 *
 * Copyright (C) 2012 Andrey Vodilov, Sergey Volkhin.
 *
 * fftw.vapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fftw.vapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fftw.vapi. If not, see <http://www.gnu.org/licenses/>.
 *
*/

[CCode (cheader_filename = "fftw3.h")]
namespace Fftw
{
  [CCode(cprefix = "FFTW_")]
  public enum Sign
  {
    FORWARD = -1,
    BACKWARD = 1
  }

  [CCode(cprefix = "FFTW_")]
  public enum Flag
  {
    MEASURE         = (0U),
    DESTROY_INPUT   = (1U << 0),
    UNALIGNED       = (1U << 1),
    CONSERVE_MEMORY = (1U << 2),
    EXHAUSTIVE      = (1U << 3),
    PRESERVE_INPUT  = (1U << 4),
    PATIENT         = (1U << 5),
    ESTIMATE        = (1U << 6)
  }

  [CCode(cname = "fftwf_plan", destroy_function = "fftwf_destroy_plan")]
  public struct Plan : int // fake int
  {
    [CCode(cname = "fftwf_plan_dft_1d")]
    public static Plan dft_1d(int n, Array input, Array output, Sign sign, Flag flags);

    [CCode(cname = "fftwf_plan_dft_r2c_1d")]
    public static Plan dft_r2c_1d(int n, Array input, Array output, Flag flags);

    [CCode(cname = "fftwf_plan_dft_c2r_1d")]
    public static Plan dft_c2r_1d(int n, Array input, Array output, Flag flags);

    [CCode(cname = "fftwf_execute")]
    public void execute();
  }

  // Пример обращения к массиву:
  // var input = new Fftw.Array(2 * sizeof(float) * N);
  //  for(int i = 0; i < 2 * N; i++)
  //    ((float[])input)[i] = 3.14f;
  [CCode(cname = "float", free_function = "fftwf_free", has_type_id = false)]
  [Compact]
  public class Array
  {
    [CCode(cname = "fftwf_malloc")]
    public Array(size_t n);
  }
}

