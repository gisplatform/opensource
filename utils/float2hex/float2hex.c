/*
 * float2hex is a small program that converts float value to hex representation and back.
 *
 * Copyright 2016 Sergey Volkhin.
 *
 * This file is part of float2hex.
 *
 * float2hex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * float2hex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with float2hex. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("Float to hex and back converter.\n");
    printf("Usage: %s <hex (starting with '0x') or float number>\n", argv[0]);
    return -1;
  }

  if(strlen(argv[1]) > 2 && !strncmp(argv[1], "0x", 2))
  {
    unsigned int in;

    if(sscanf(argv[1], "%x", &in) == 1)
    {
      printf("%f\n", *(float*)&in);
      return 0;
    }
    else
    {
      perror("Failed to parse input hex value");
      return -1;
    }
  }
  else
  {
    float in;

    if(sscanf(argv[1], "%f", &in) == 1)
    {
      printf("0x%x\n", *(unsigned int*)&in);
      return 0;
    }
    else
    {
      perror("Failed to parse input float value");
      return -1;
    }
  }
}

