/*
	compat: Some compatibility functions (basic memory & string stuff in separate file)

	The mpg123 code is determined to keep it's legacy. A legacy of old, old UNIX.
	So anything possibly somewhat advanced should be considered to be put here, with proper #ifdef;-)

	copyright 2007-2020 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis, Windows Unicode stuff by JonY.
*/

#include "config.h"
/* This source file does need _POSIX_SOURCE to get some sigaction. */
#define _POSIX_SOURCE
#include "compat.h"

#ifdef _MSC_VER
#include <io.h>

#if(defined(WINAPI_FAMILY) && (WINAPI_FAMILY==WINAPI_FAMILY_APP))
#define WINDOWS_UWP
#endif

#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#endif

/* Win32 is only supported with unicode now. These headers also cover
   module stuff. The WANT_WIN32_UNICODE macro is synonymous with
   "want windows-specific API, and only the unicode variants of which". */
#ifdef WANT_WIN32_UNICODE
#include <wchar.h>
#include <windows.h>
#include <winnls.h>
#include <shlwapi.h>
#endif

#include "debug.h"

#ifndef WINDOWS_UWP

char *compat_getenv(const char* name)
{
	char *ret = NULL;
#ifdef WANT_WIN32_UNICODE
	wchar_t *env;
	wchar_t *wname = NULL;
	if(win32_utf8_wide(name, &wname, NULL) > 0)
	{
		env = _wgetenv(wname);
		free(wname);
		if(env)
			win32_wide_utf8(env, &ret, NULL);
	}
#else
	ret = getenv(name);
	if(ret)
		ret = compat_strdup(ret);
#endif
	return ret;
}

#endif

#include "wpathconv.h"

/* Always add a default permission mask in case of flags|O_CREAT. */
int compat_open(const char *filename, int flags)
{
	int ret;
#if defined (WANT_WIN32_UNICODE)
	wchar_t *frag = NULL;

	frag = u2wlongpath(filename);
	/* Fallback to plain open when ucs-2 conversion fails */
	if(!frag)
		goto open_fallback;

	/*Try _wopen */
	ret = _wopen(frag, flags|_O_BINARY, _S_IREAD | _S_IWRITE);
	if(ret != -1 )
		goto open_ok; /* msdn says -1 means failure */

open_fallback:
#endif

#if (defined(WIN32) && !defined (__CYGWIN__))
	/* MSDN says POSIX function is deprecated beginning in Visual C++ 2005 */
	/* Try plain old _open(), if it fails, do nothing */
	ret = _open(filename, flags|_O_BINARY, _S_IREAD | _S_IWRITE);
#else
	ret = open(filename, flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#endif

#if defined (WANT_WIN32_UNICODE)
open_ok:
	free(frag);
#endif

	return ret;
}

/* Moved over from wav.c, logic with fallbacks added from the
   example of compat_open(). */
FILE* compat_fopen(const char *filename, const char *mode)
{
	FILE* stream = NULL;
#ifdef WANT_WIN32_UNICODE
	int cnt = 0;
	wchar_t *wname = NULL;
	wchar_t *wmode = NULL;

	wname = u2wlongpath(filename);
	if(!wname)
		goto fopen_fallback;
	cnt = win32_utf8_wide(mode, &wmode, NULL);
	if( (wmode == NULL) || (cnt == 0))
		goto fopen_fallback;

	stream = _wfopen(wname, wmode);
	if(stream) goto fopen_ok;

fopen_fallback:
#endif
	stream = fopen(filename, mode);
#ifdef WANT_WIN32_UNICODE

fopen_ok:
	free(wmode);
	free(wname);
#endif
	return stream;
}

FILE* compat_fdopen(int fd, const char *mode)
{
	return fdopen(fd, mode);
}

int compat_close(int infd)
{
#if (defined(WIN32) && !defined (__CYGWIN__)) /* MSDN says POSIX function is deprecated beginning in Visual C++ 2005 */
	return _close(infd);
#else
	return close(infd);
#endif
}

int compat_fclose(FILE *stream)
{
	return fclose(stream);
}

#ifndef WINDOWS_UWP

/*
	The Windows file and path stuff is an extract of jon_y's win32 loader
	prototype from the loader_rework branch. It's been divided in to
	reusable functons by ThOr in the hope to work out some generic-looking
	loader code for both POSIX and Windows. The routines might be
	helpful for consistent path work in other parts of mpg123, too.

	This all is about getting some working code on a wide range of
	systems while staying somewhat sane. If it does ridiculously inefficient
	things with extraneous copies and grabbing of functions that made
	it late to some official APIs, that's still fine with us.
*/

char* compat_catpath(const char *prefix, const char* path)
{
	char *ret = NULL;
#ifdef WANT_WIN32_UNICODE
	wchar_t *wprefix = NULL; /* Wide windows versions of */
	wchar_t *wpath   = NULL; /* input arguments. */
	wchar_t *locwret = NULL; /* Tmp return value from LocalAlloc */
	/*
		This variation of combinepath can work with long and UNC paths, but
		is not officially exposed in any DLLs, It also allocates all its buffers
		internally via LocalAlloc, avoiding buffer overflow problems.
		ThOr: I presume this hack is for supporting pre-8 Windows, as
		from Windows 8 on, this is documented in the API.
	*/
	HRESULT (__stdcall *mypac)( const wchar_t *in, const wchar_t* more
	,	unsigned long flags, wchar_t **out ) = NULL;
	HMODULE pathcch = NULL;

	if(!prefix && !path)
		goto catpath_end;
	wprefix = u2wpath(prefix);
	wpath   = u2wpath(path);
	if((prefix && !wprefix) || (path && !wpath))
		goto catpath_end;

	/* Again: I presume this whole fun is to get at PathAllocCombine
	   even when pathcch.h is not available (like in MinGW32). */
	if( (pathcch = GetModuleHandleA("kernelbase")) )
		mypac = (void *)GetProcAddress(pathcch, "PathAllocCombine");
	if(mypac) /* PATHCCH_ALLOW_LONG_PATH = 1 per API docs */
	{
		debug("Actually calling PathAllocCombine!");
		mypac(wprefix, wpath, 1, &locwret);
	}
	else
	{
		/* Playing safe, if we'd care much about performance, this would be on
		   the stack. */
		locwret = LocalAlloc(LPTR, sizeof(wchar_t)*MAX_PATH);
		if(locwret)
			PathCombineW(locwret, wprefix, wpath);
	}
	ret = w2upath(locwret);

catpath_end:
	LocalFree(locwret);
	free(wprefix);
	free(wpath);
#else
	size_t len, prelen, patlen;

	if(path && path[0] == '/')
		prefix = NULL; /* Absolute path stays as it is. */
	prelen = prefix ? strlen(prefix) : 0;
	patlen = path   ? strlen(path)   : 0;
	/* Concatenate the two, put a / in between if both present. */
	len = ((prefix && path) ? 1 : 0) + prelen + patlen;
	ret = malloc(len+1);
	if(ret)
	{
		size_t off=0;
		memcpy(ret, prefix, prelen);
		if(prefix && path)
			ret[prelen+(off++)] = '/';
		memcpy(ret+prelen+off, path, patlen);
		ret[len] = 0;
	}
#endif
	return ret;
}

int compat_isdir(const char *path)
{
	int ret = 0;
#ifdef WANT_WIN32_UNICODE
	wchar_t *wpath;
	wpath = u2wlongpath(path);
	if(wpath)
	{
		DWORD attr = GetFileAttributesW(wpath);
		if(attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY)
			ret=1;
		free(wpath);
	}
#else
	struct stat sb;
	if(path && !stat(path, &sb))
	{
		if(S_ISDIR(sb.st_mode))
			ret=1;
	}
#endif
	return ret;
}

struct compat_dir
{
	char *path;
#ifdef WANT_WIN32_UNICODE
	int gotone; /* Got a result stored from FindFirstFileW. */
	WIN32_FIND_DATAW d;
	HANDLE ffn;
#else
	DIR* dir;
#endif
};

struct compat_dir* compat_diropen(char *path)
{
	struct compat_dir *cd;
	if(!path)
		return NULL;
	cd = malloc(sizeof(*cd));
	if(!cd)
		return NULL;
#ifdef WANT_WIN32_UNICODE
	cd->gotone = 0;
	{
		char *pattern;
		wchar_t *wpattern;
		pattern = compat_catpath(path, "*");
		wpattern = u2wlongpath(pattern);
		if(wpattern)
		{
			cd->ffn = FindFirstFileW(wpattern, &(cd->d));
			if(cd->ffn == INVALID_HANDLE_VALUE)
			{
				/* FindClose() only needed after successful first find, right? */
				free(cd);
				cd = NULL;
			}
			else
				cd->gotone = 1;
		}
		free(wpattern);
		free(pattern);
	}
#else
	cd->dir = opendir(path);
	if(!cd->dir)
	{
		free(cd);
		cd = NULL;
	}
#endif
	if(cd)
	{
		cd->path = compat_strdup(path);
		if(!cd->path)
		{
			compat_dirclose(cd);
			cd = NULL;
		}
	}
	return cd;
}

void compat_dirclose(struct compat_dir *cd)
{
	if(cd)
	{
		free(cd->path);
#ifdef WANT_WIN32_UNICODE
		FindClose(cd->ffn);
#else
		closedir(cd->dir);
#endif
		free(cd);
	}
}

char* compat_nextfile(struct compat_dir *cd)
{
	if(!cd)
		return NULL;
#ifdef WANT_WIN32_UNICODE
	while(cd->gotone || FindNextFileW(cd->ffn, &(cd->d)))
	{
		cd->gotone = 0;
		if(!(cd->d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char *ret;
			win32_wide_utf8(cd->d.cFileName, &ret, NULL);
			return ret;
		}
	}
#else
	{
		struct dirent *dp;
		while((dp = readdir(cd->dir)))
		{
			struct stat fst;
			char *fullpath = compat_catpath(cd->path, dp->d_name);
			if(fullpath && !stat(fullpath, &fst) && S_ISREG(fst.st_mode))
			{
				free(fullpath);
				return compat_strdup(dp->d_name);
			}
			free(fullpath);
		}
	}
#endif
	return NULL;
}

char* compat_nextdir(struct compat_dir *cd)
{
	if(!cd)
		return NULL;
#ifdef WANT_WIN32_UNICODE
	while(cd->gotone || FindNextFileW(cd->ffn, &(cd->d)))
	{
		cd->gotone = 0;
		if(cd->d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char *ret;
			win32_wide_utf8(cd->d.cFileName, &ret, NULL);
			return ret;
		}
	}
#else
	{
		struct dirent *dp;
		while((dp = readdir(cd->dir)))
		{
			struct stat fst;
			char *fullpath = compat_catpath(cd->path, dp->d_name);
			if(fullpath && !stat(fullpath, &fst) && S_ISDIR(fst.st_mode))
			{
				free(fullpath);
				return compat_strdup(dp->d_name);
			}
			free(fullpath);
		}
	}
#endif
	return NULL;
}

#endif

// Revisit logic of write():
// Return -1 if interrupted before any data was written,
// set errno to EINTR. Any other error value is serious
// for blocking I/O, which we assume here. EAGAIN should be
// handed through.
// Reaction to zero-sized write attempts could also be funky, so avoid that.
// May return short count for various reasons. I assume that
// any serious condition will show itself as return value -1
// eventually.

// These uninterruptible write/read functions shall persist as long as
// possible to finish the desired operation. A short byte count is short
// because of a serious reason (maybe EOF, maybe out of disk space). You
// can inspect errno.

size_t unintr_write(int fd, void const *buffer, size_t bytes)
{
	size_t written = 0;
	errno = 0;
	while(bytes)
	{
		errno = 0;
		ssize_t part = write(fd, (char*)buffer+written, bytes);
		// Just on short writes, we do not abort. Only when
		// there was no successful operation (even zero write) at all.
		// Any other error than EINTR ends things here.
		if(part >= 0)
		{
			bytes   -= part;
			written += part;
		} else if(errno != EINTR)
			break;
	}
	return written;
}

/* Same for reading the data. */
size_t unintr_read(int fd, void *buffer, size_t bytes)
{
	size_t got = 0;
	errno = 0;
	while(bytes)
	{
		errno = 0;
		ssize_t part = read(fd, (char*)buffer+got, bytes);
		if(part >= 0)
		{
			bytes -= part;
			got   += part;
		} else if(errno != EINTR)
			break;
	}
	return got;
}

// and again for streams
size_t unintr_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = 0;
	errno = 0;
	while(size && nmemb)
	{
		errno = 0;
		size_t part = fwrite((char*)ptr+written*size, size, nmemb, stream);
		if(part > 0)
		{
			nmemb   -= part;
			written += part;
		} else if(errno != EINTR)
			break;
	}
	return written;
}

#ifndef NO_CATCHSIGNAL
#if (!defined(WIN32) || defined (__CYGWIN__)) && defined(HAVE_SIGNAL_H)
void (*catchsignal(int signum, void(*handler)()))()
{
	struct sigaction new_sa;
	struct sigaction old_sa;

#ifdef DONT_CATCH_SIGNALS
	fprintf (stderr, "Not catching any signals.\n");
	return ((void (*)()) -1);
#endif

	new_sa.sa_handler = handler;
	sigemptyset(&new_sa.sa_mask);
	new_sa.sa_flags = 0;
	if(sigaction(signum, &new_sa, &old_sa) == -1)
		return ((void (*)()) -1);
	return (old_sa.sa_handler);
}
#endif
#endif
