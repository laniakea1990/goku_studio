#include <sys/types.h>		/* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>	/* for winsize */
#include <sys/ioctl.h>
#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
void
set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
    int  val;

    if ( (val = fcntl(fd, F_GETFL, 0)) < 0)
        {
            printf("fcntl F_GETFL error");
            exit(1);
        }

    val |= flags; 

    if (fcntl(fd, F_SETFL, val) < 0)
        {
            printf("fcntl F_SETFL error");
            exit(1);
        }
}

void
clr_fl(int fd, int flags)
{
    int val;

    if ((val = fcntl(fd, F_GETFL, 0)) == -1)
    {
        syslog(LOG_ERR, __FILE__, __LINE__,"fcntl() error : %s", strerror(errno));
        exit(1);
    }
    val &= ~flags; /* turn flags off */

    if (fcntl(fd, F_SETFL, val) == -1)
    {
        syslog(LOG_ERR, __FILE__, __LINE__,"fcntl() error : %s", strerror(errno));
        exit(1);
    }
    return;
}
