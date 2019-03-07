// Sysfs poll listener.
// Based on http://stackoverflow.com/questions/16367623/using-the-linux-sysfs-notify-call
// by Vilhelm Gray (and thanks to http://stackoverflow.com/users/1401351/peter).

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <libgen.h>


int main(int argc, char **argv)
{
  char buf4read[100];
  int trigger_fd, rv;
  struct pollfd ufd;

  if(argc != 2)
  {
    printf("Poll event listener (classic poll).\n");
    printf("Usage: %s <path to file to poll on>\n", argv[0]);
    return -1;
  }

  if((trigger_fd = open(argv[1], O_RDWR)) < 0)
  {
    perror("Unable to open trigger");
    return -2;
  }

  ufd.fd = trigger_fd;
  ufd.events = POLLPRI | POLLERR;

  while(1)
  {
    ufd.revents = 0;

    // Перед poll нужно обязательно вычитать, иначе зациклемся.
    read(trigger_fd, buf4read, sizeof(buf4read));

    if((rv = poll(&ufd, 1, 100000)) < 0)
    {
      perror("Poll error");
    }
    else if (rv == 0)
    {
      printf("Timeout occurred!\n");
    }
    else if (ufd.revents & POLLPRI)
    {
      printf("'%s' triggered!\n", basename(argv[1]));
    }

    printf("revents: %08X\n", ufd.revents);
  }

  close(trigger_fd);

  return 0;
}

