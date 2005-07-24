/** @file Run python interpreter on a python script

  We assume that the python script name is like the executable name with .py
  extension.

  We set PYTHONPATH environment variable to point to <script-dir>/pylib and
  run "system" on local python with the script as argument.
*/

/*======================================================#
# Copyright (c) Miki Tebeka <miki.tebeka@gmail.com>     #
# This file is under the GNU Public License (GPL), see  #
# http://www.gnu.org/copyleft/gpl.html for more details #
# =====================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define ON_MSWIN defined(WIN32) || defined(_MSC_VER)

#if ON_MSWIN
#define WIN32_LEAN_AND_MEAN /* Exclude rearly used headers */
#include <windows.h>
#define S_IFREG _S_IFREG
#define execv _execv
#define DIRSEP '\\' /* Directory seperator */
#define PATHSEP ';' /* Path seperator */

#else /* POSIX */
#include <unistd.h>

#define MAX_PATH 1024
#define DIRSEP '/' /* Directory seperator */
#define PATHSEP ':' /* Path seperator */
#endif /* WIN32 */

#define PYLIBDIR "pylib" /* Name of python library */

/* Forward declaration */

/** Print an error message
  @param format printf like format
  @param ... Rest of arguments
*/
static void error(char *format, ...);
/** Print system error */
static void sys_error();
/** Get executable file name 
  @param buf Output buffer
  @param buf_size Size of output buffer
  @return 1 on success, 0 otherwise
*/
static int get_exe_name(char *buf, size_t buf_size);
/** Check if file exists
  @param filename Name of file to check
  @return 1 if file exists, 0 otherwise
*/
static int is_file(const char *filename);

static void
error(char *format, ...)
{
    va_list args;

    fprintf(stderr, "error: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

int
is_file(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) != 0) {
		return 0;
	}

	return (st.st_mode & S_IFREG) != 0;
}


#if ON_MSWIN
static void 
sys_error()
{
    LPVOID buf;
    DWORD errnum;

    errnum = GetLastError();

    if (!FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errnum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&buf,
            0,
            NULL)) {
        error("unknown error - %d", errnum);
    }
    else {
        error("%s", (char *)buf);
    }
}

static int
get_exe_name(char *buf, size_t buf_size)
{

	if(!GetModuleFileName(NULL, buf, buf_size)) {
        return 0;
    }

    return 1;
}

#else /* POSIX */

static void 
sys_error()
{
    perror("error");
}

/* FIXME: We read /proc/<pid>/exe which works on linux but not on Solaris.
   Find a more portable way to do this (see Python's sysmodule.c)
*/
static int
get_exe_name(char *buf, size_t buf_size)
{
    char link[MAX_PATH];

    sprintf(link, "/proc/%d/exe", getpid());

    if (readlink(link, buf, buf_size) < 0) {
        return 0;
    }

    return 1;
}

#endif /* WIN32 */

int
main(int argc, char *argv[])
{
    char scriptdir[MAX_PATH]; /* Script directory */
    char scriptname[MAX_PATH]; /* Script file name */
    char *cp; /* General use char * */
    char pyexe[MAX_PATH]; /* Name of python executable */
    char pypath[MAX_PATH]; /* PYTHONPATH value */
    char *oldpath; /* Original PYTHONPATH value */
    char cmd[MAX_PATH * 4]; /* Command buffer */
    int i;

    if (!get_exe_name(scriptname, MAX_PATH)) {
        sys_error();
        return 1;
    }

    /* Find executable directory */
    strcpy(scriptdir, scriptname);
    cp = strrchr(scriptdir, DIRSEP);
    if (NULL == cp) {
        strcpy(scriptdir, ".");
    } 
    else {
        *cp = 0;
    }

    /* Find python executable full path */
    sprintf(pyexe, "%s%cpython", scriptdir, DIRSEP);
#if ON_MSWIN
    strcat(pyexe, ".exe");
#endif

    if (!is_file(pyexe)) {
        error("can't find %s", pyexe);
        return 1;
    }

    /* Find script to run (full path) */
    cp = strrchr(scriptname, '.');
    if (NULL == cp) {
        cp = scriptname + strlen(scriptname);
    }
    strcpy(cp, ".py");
    if (!is_file(scriptname)) {
        error("can't find %s", scriptname);
        return 1;
    } 

    /* Set PYTHONPATH */
    sprintf(pypath, "PYTHONPATH=");
    oldpath = getenv("PYTHONPATH");
    if (oldpath) {
        sprintf(pypath + strlen(pypath), "%s%c", oldpath, PATHSEP);
    }
    else {
        sprintf(pypath, "PYTHONPATH=");
    }
    sprintf(pypath + strlen(pypath), "%s%c%s", scriptdir, DIRSEP, PYLIBDIR);
    /* We don't add the script directory since Python does it */
    putenv(pypath);


    sprintf(cmd, "%s %s", pyexe, scriptname);
    /* FIXME: This is inefficent as hell */
    for (i = 1; i < argc; ++i) {
        strcat(cmd, " \"");
        strcat(cmd, argv[i]);
        strcat(cmd, "\"");
    }

    return system(cmd);
}
