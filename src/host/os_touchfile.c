/**
 * \file   os_touchfile.c
 * \brief markes a file as modified without changing its contents.
 * \author Blizzard Entertainment (contact tvandijck@blizzard.com)
 * \author Copyright (c) 2015 Jason Perkins and the Premake project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "premake.h"

#if PLATFORM_WINDOWS
	#include <io.h>
  #include <fcntl.h>
	#include <sys/stat.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

static int truncate_file(const char* fn)
{
#if PLATFORM_WINDOWS
	int fh; //File handle
	if( _sopen_s( &fh, fn, _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE ) == 0 )
	{
		long size = _filelength( fh );
		//Write, then truncate
		char buffer[] = " ";
		if (_lseek( fh, 0L, SEEK_END ) != -1L)
		{
			if (_write(fh, buffer, 1) == 1)
			{
				if(_chsize( fh, size ) == 0)
				{
					_close(fh);
					return TRUE;
				}
			}
		}
		_close(fh);
		return FALSE;
	}
	else
		return FALSE;
#else
	FILE* file = fopen(fn, "rb");
	size_t size;
	file = fopen(fn, "ab");
	if (file == NULL)
	{
		return FALSE;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	// append a dummy space. There are better ways to do
	// a touch, however this is a rather simple
	// multiplatform method
	if (fwrite(" ", 1, 1, file) != 1)
	{
		fclose(file);
		return FALSE;
	}

	fclose(file);

	if (truncate(fn, (off_t)size) != 0)
	{
		return FALSE;
	}
#endif
	return TRUE;
}

int os_touchfile(lua_State* L)
{
	FILE* file;
	const char* dst     = luaL_checkstring(L, 1);

	// if destination exist, mark the file as modified
	if (do_isfile(L, dst))
	{
		if (truncate_file(dst))
		{
			lua_pushinteger(L, 0);
			return 1;
		} else {
			lua_pushinteger(L, -1);
			lua_pushfstring(L, "unable to touch file '%s'", dst);
			return 2;
		}
	}

#if PLATFORM_WINDOWS
	wchar_t wide_path[PATH_MAX];
	if (MultiByteToWideChar(CP_UTF8, 0, dst, -1, wide_path, PATH_MAX) == 0)
	{
		lua_pushinteger(L, -1);
		lua_pushstring(L, "unable to encode path");
		return 2;
	}

	file = _wfopen(wide_path, L"wb");
#else
	file = fopen(dst, "wb");
#endif

	if (file != NULL)
	{
		fclose(file);

		lua_pushinteger(L, 1);
		return 1;
	}

	lua_pushinteger(L, -1);
	lua_pushfstring(L, "unable to open file to '%s'", dst);
	return 2;
}
