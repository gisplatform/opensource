/*
 * gopnik is a small program for EG20T GPIO.
 *
 * Copyright 2017 Sergey Volkhin.
 *
 * This file is part of gopnik.
 *
 * gopnik is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * gopnik is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with gopnik. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "config.h"
#include "error.h"
#include <errno.h>
#include <fcntl.h>
#include <gio/gio.h>
#include <stdio.h>
#include <sys/mman.h>

/*
 * Макросы ниже позволяют читать и писать биты в GPIO.
 * Макросы свободны от блокирововок, посему быстры, но во время их использования нельзя
 * использовать другие способы доступа к GPIO (например, sysfs).
 * BGET позволяет получить бит GPIO.
 * BSET позволяет незамедлительно установить бит GPIO.
 * BSET_POSTPONED позволяет заранее задать бит без реального доступа к PCIe,
 * применение всех предыдущих BSET_POSTPONED происходит во время BSET.
 * Перед BSET/BSET_POSTPONED необходимо вызвать BSET_INIT единожды, в той же области видимости.
 */
#define BGET(nr) ((regs->pi >> (nr)) & 1)
#define BSET_POSTPONED(nr, val) G_STMT_START { if(val) _bset_po_cache |= (1 << (nr)); else _bset_po_cache &= ~(1 << (nr)); } G_STMT_END
#define BSET(nr, val) G_STMT_START { BSET_POSTPONED(nr, val); regs->po = _bset_po_cache; } G_STMT_END
#define BSET_INIT() int _bset_po_cache = regs->po;

static gboolean only_find = FALSE;
static gboolean load_test = FALSE;
static gint set_test = -1;
static gint get_test = -1;
static gint bit_get = -1;
static gint bit_set = -1;
static gint bit_unset = -1;
static gchar *firmware = NULL;

static GOptionEntry entries[] =
{
  { "find",      'f', 0, G_OPTION_ARG_NONE, &only_find, "Only find EG20T GPIO and exit", NULL },
  { "bit-get",   'g', 0, G_OPTION_ARG_INT,  &bit_get,   "Get GPIO signal at specified PIN", NULL },
  { "bit-set",   's', 0, G_OPTION_ARG_INT,  &bit_set,   "Set up GPIO signal at specified PIN", NULL },
  { "bit-unset", 'u', 0, G_OPTION_ARG_INT,  &bit_unset, "Unset GPIO signal at specified PIN", NULL },
  { "set-test",  'w', 0, G_OPTION_ARG_INT,  &set_test, "Performance test of one PIN set", NULL },
  { "get-test",  'r', 0, G_OPTION_ARG_INT,  &get_test, "Performance test of one PIN get", NULL },
  { "load-test", 't', 0, G_OPTION_ARG_NONE, &load_test, "Load test", NULL },
  { "load-firmware", 'l', 0, G_OPTION_ARG_STRING, &firmware, "Load firmware", NULL },
  { NULL }
};


// Struct from Linux/drivers/gpio/gpio-pch.c.
// Copyright (C) 2011 LAPIS Semiconductor Co., Ltd.
struct _PchRegs
{
  volatile guint32 ien;
  volatile guint32 istatus;
  volatile guint32 idisp;
  volatile guint32 iclr;
  volatile guint32 imask;
  volatile guint32 imaskclr;
  volatile guint32 po;
  volatile guint32 pi;
  volatile guint32 pm;
  volatile guint32 im0;
  volatile guint32 im1;
  volatile guint32 reserved[3];
  volatile guint32 gpio_use_sel;
  volatile guint32 reset;
};

typedef struct _PchRegs PchRegs;


static gboolean load_firmware(const gchar *path, PchRegs *regs, GError **err)
{
  gboolean rval = FALSE;
  g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

  // Ресурсы, подлежащие освобождению -->
    GError *internal_error = NULL;
    GFileIOStream *ios = NULL;
    GBufferedInputStream *bis = NULL; //< Buffered.
  // Ресурсы, подлежащие освобождению <--

  { // Открываем файл -->
    GFile *gfile = g_file_new_for_path(path);
      ios = g_file_open_readwrite(gfile, NULL, &internal_error);
    g_object_unref(gfile);

    if(G_UNLIKELY(ios == NULL))
      goto exit;

    bis = G_BUFFERED_INPUT_STREAM(g_buffered_input_stream_new(g_io_stream_get_input_stream(G_IO_STREAM(ios))));
  } // Открываем файл <--

  BSET_INIT();

  BSET(PIN_DCLK, 0);
  BSET(PIN_NCONFIG, 1);
  g_usleep(1);

#if PIN_NSTATUS
  if(G_UNLIKELY(BGET(PIN_NSTATUS) == 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "nSTATUS is low at startup.");
    goto exit;
  }
#endif

//#ifdef PIN_CONF_DONE
//  if(G_UNLIKELY(BGET(PIN_CONF_DONE) == 0))
//  {
//    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "CONF_DONE is low at startup.");
//    goto exit;
//  }
//#endif

  BSET(PIN_NCONFIG, 0);
  g_usleep(1); //< 500 нан по манулу, t CFG = nCONFIG low pulse width.

#if PIN_NSTATUS
  if(G_UNLIKELY(BGET(PIN_NSTATUS) != 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "nSTATUS is high after nCONFIG low.");
    goto exit;
  }
#endif

#ifdef PIN_CONF_DONE
  if(G_UNLIKELY(BGET(PIN_CONF_DONE) != 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "CONF_DONE is high after nCONFIG low.");
    goto exit;
  }
#endif

  BSET(PIN_NCONFIG, 1);
  g_usleep(1); //< 230 нан по манулу, t CF2ST1 = nCONFIG high to nSTATUS high.

#ifdef PIN_CONF_DONE
  if(G_UNLIKELY(BGET(PIN_CONF_DONE) != 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "CONF_DONE is high after nCONFIG high.");
    goto exit;
  }
#endif

#if PIN_NSTATUS
  if(G_UNLIKELY(BGET(PIN_NSTATUS) == 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "nSTATUS is low after nCONFIG high.");
    goto exit;
  }
#endif

  g_usleep(2); //< 2 мкс по манулу, t ST2CK = nSTATUS high to first rising edge of DCLK.

  gint byte, shift;
  while((byte = g_buffered_input_stream_read_byte(bis, NULL, &internal_error)) != -1)
  {
    if(G_UNLIKELY(internal_error != NULL))
      goto exit;

    for(shift = 0; shift < 8; shift++)
    {
      BSET_POSTPONED(PIN_DCLK, 0);
      BSET(PIN_DATA, (byte & (1 << shift)) ? 1 : 0);
      BSET(PIN_DCLK, 1);
    }
  }

  if(G_UNLIKELY(internal_error != NULL))
    goto exit;

  // Из манула: Two DCLK falling edges are required after CONF_DONE
  // goes high to begin the initialization of the device.
  g_usleep(1);
  BSET(PIN_DCLK, 0);
  g_usleep(1);
  BSET(PIN_DCLK, 1);
  g_usleep(1);
  BSET(PIN_DCLK, 0);

#if PIN_NSTATUS
  if(G_UNLIKELY(BGET(PIN_NSTATUS) == 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "nSTATUS is low after last bit transfer.");
    goto exit;
  }
#endif

#ifdef PIN_CONF_DONE
  if(G_UNLIKELY(BGET(PIN_CONF_DONE) == 0))
  {
    g_set_error(err, GOPNIK_ERROR, GOPNIK_ERROR_FPGA, "CONF_DONE is low after last bit transfer.");
    goto exit;
  }
#endif

exit:
  if(internal_error != NULL)  g_propagate_error(err, internal_error);
  g_clear_object(&bis);
  g_clear_object(&ios);

  return rval;
}


/**
 * find_device:
 *
 * Поиск PCI-устройства EG20T GPIO.
 *
 * Returns: Путь к BAR устройства в sysfs.
 */
static gchar* find_device()
{
  // Ресурсы, подлежащие освобождению -->
    GDir *pch_gpio_dir = NULL;
    GRegex *regex = NULL;
    GError *error = NULL;
  // Ресурсы, подлежащие освобождению <--

  const gchar *entry;
  gchar *rval = NULL;
  pch_gpio_dir = g_dir_open(PCH_GPIO_DIR_PATH, 0, &error);

  if(!error)
  {
    regex = g_regex_new("^[0-9]{4}:[0-9]{2}:[0-9]{2}\\.[0-9]$", 0, 0, &error);

    if(!error)
    {
      while(( entry = g_dir_read_name(pch_gpio_dir) ))
        if(g_regex_match(regex, entry, 0, NULL))
        {
          rval = g_build_filename(PCH_GPIO_DIR_PATH, entry, "resource1", NULL);
          goto exit;
        }
    }
    else
    {
      g_print("Failed to create pattern object: %s\n", error->message);
      goto exit;
    }
  }
  else
  {
    g_print("Failed to open dir in sysfs with PCI drivers: %s\n", error->message);
    goto exit;
  }

exit:
  if(pch_gpio_dir != NULL)  g_dir_close(pch_gpio_dir);
  if(regex != NULL)         g_regex_unref(regex);
  if(error != NULL)         g_error_free(error);

  return rval;
}


int main(int argc, char *argv[])
{
  GTimeVal tv[2];
  guint i, rval = -1;

  g_type_init();

  // Ресурсы, подлежащие освобождению -->
    GError *error = NULL;
    GOptionContext *context = NULL;
    gchar *pci_bar_path = NULL;
    int fd = -1;
    PchRegs* regs = NULL;
  // Ресурсы, подлежащие освобождению <--

  context = g_option_context_new("- EG20T GPIO manipulator");
  g_option_context_add_main_entries(context, entries, NULL);
  g_option_context_set_help_enabled(context, TRUE);

  if(!g_option_context_parse(context, &argc, &argv, &error))
  {
    g_print("Options parsing failed: %s\n", error->message);
    goto exit;
  }

  if(( pci_bar_path = find_device() ))
  {
    g_print("Found PCI device by driver pch_gpio, BAR: %s\n", pci_bar_path);
  }
  else
  {
    g_print("Failed to find PCI device, so exiting... :(\n");
    goto exit;
  }

  if(only_find)
    goto exit;

  if((fd = open(pci_bar_path, O_RDWR | O_SYNC)) >= 0)
  {
    g_print("PCI device BAR file succesfully opened\n");
  }
  else
  {
    perror("Failed to open PCI device BAR file");
    goto exit;
  }

  if(( regs = mmap(0, MMAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) ))
  {
    g_print("PCI device BAR succesfully mapped\n");
  }
  else
  {
    perror("Failed to map PCI BAR, so exiting... :(\n");
    goto exit;
  }

  if(bit_get != -1)
    printf("BIT GET: %u\n", BGET(bit_get));

  if(bit_set != -1)
  {
    BSET_INIT();
    BSET(bit_set, 1);
  }

  if(bit_unset != -1)
  {
    BSET_INIT();
    BSET(bit_unset, 0);
  }

  if(get_test != -1)
  {
    volatile int get_val;

    g_get_current_time(&tv[0]);

    for(i = 0; i < DOWNLOAD_TEST_BITS; i++)
      get_val = BGET(get_test);

    g_get_current_time(&tv[1]);
    double time = (double)(tv[1].tv_usec - tv[0].tv_usec) / G_USEC_PER_SEC + tv[1].tv_sec - tv[0].tv_sec;
    g_print("Get bit %u test done: %u times in %.3lf seconds, freq = %.3lf KHz, last val = %u\n",
      get_test, DOWNLOAD_TEST_BITS, time, (double)DOWNLOAD_TEST_BITS / time / 1000, get_val);
  }

  if(set_test != -1)
  {
    g_get_current_time(&tv[0]);

    BSET_INIT();
    for(i = 0; i < LOAD_TEST_BITS; i++)
      BSET(set_test, i & 1);

    g_get_current_time(&tv[1]);
    double time = (double)(tv[1].tv_usec - tv[0].tv_usec) / G_USEC_PER_SEC + tv[1].tv_sec - tv[0].tv_sec;
    g_print("Set bit %u test done: %u times in %.3lf seconds, freq = %.3lf KHz\n", set_test,
      LOAD_TEST_BITS, time, (double)LOAD_TEST_BITS / time / 1000);
  }

  if(firmware && load_test)
  {
    g_print("Error: firmware and test load options are not compatible.\n");
    goto exit;
  }

  if(firmware)
  {
    g_print("Loading firware. Please, wait...\n");

    g_get_current_time(&tv[0]);
    load_firmware(firmware, regs, &error);
    g_get_current_time(&tv[1]);

    if(error != NULL)
    {
      g_print("Failed to load firmware: %s\n", error->message);
      goto exit;
    }
    else
      g_print("Firware has been successfully loaded in %.3lf seconds\n", 
        (double)(tv[1].tv_usec - tv[0].tv_usec) / G_USEC_PER_SEC + tv[1].tv_sec - tv[0].tv_sec);
  }

  if(load_test)
  {
    g_get_current_time(&tv[0]);

    BSET_INIT();
    for(i = 0; i < LOAD_TEST_BITS; i++)
    {
      BSET_POSTPONED(PIN_DCLK, 0);
      BSET(PIN_DATA, i & 1);
      BSET(PIN_DCLK, 1);
    }

    g_get_current_time(&tv[1]);
    double time = (double)(tv[1].tv_usec - tv[0].tv_usec) / G_USEC_PER_SEC + tv[1].tv_sec - tv[0].tv_sec;
    g_print("Load test done: %u bits in %.3lf seconds, speed = %.3lf Kbits per sec\n",
      LOAD_TEST_BITS, time, (double)LOAD_TEST_BITS / time / 1000);
  }

  rval = 0;
exit:
  if(error != NULL)         g_error_free(error);
  if(context != NULL)       g_option_context_free(context);
  g_free(pci_bar_path);
  if(fd >= 0)               close(fd);
  if(regs != NULL)          munmap(regs, MMAP_LENGTH);
  g_free(firmware);

  return rval;
}

