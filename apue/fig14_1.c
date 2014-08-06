#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include "apueerror.h"
#include <sys/syslog.h>
char	buf[500000];

int
main(void)
{
	int		ntowrite, nwrite;
	char	*ptr;

	ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "read %d bytes\n", ntowrite);

	set_fl(STDOUT_FILENO, O_NONBLOCK);	/* set nonblocking */

	ptr = buf;
	while (ntowrite > 0) {
		errno = 0;
		nwrite = write(STDOUT_FILENO, ptr, ntowrite);
		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);

		if (nwrite > 0) {
			ptr += nwrite;
			ntowrite -= nwrite;
		}
	}

	clr_fl(STDOUT_FILENO, O_NONBLOCK);	/* clear nonblocking */

	exit(0);
}


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
