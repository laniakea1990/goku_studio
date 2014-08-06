#include "apue.h"
#include <sys/wait.h>
#include "apueerror.h"

int
main(void)
{
	pid_t	pid;

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid == 0) {	/* specify pathname, specify environment */
		if (execl("/var/workspace/c_programming/7/cat", "./cat", "test1", "test2", (char *)0) < 0)
			err_sys("execl error");
	}

	if (waitpid(pid, NULL, 0) < 0)
		err_sys("wait error");


	exit(0);
}
