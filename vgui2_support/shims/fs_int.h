#ifndef CS16_VGUI2_FS_INT_SHIM_H
#define CS16_VGUI2_FS_INT_SHIM_H

#include <stdarg.h>
#include <stddef.h>
#include "xash3d_types.h"

#ifndef FS_GAMEDIR_PATH
#define FS_GAMEDIR_PATH (1u << 2)
#endif

#ifndef FS_CUSTOM_PATH
#define FS_CUSTOM_PATH (1u << 3)
#endif

#ifndef FS_NOWRITE_PATH
#define FS_NOWRITE_PATH (1u << 1)
#endif

typedef struct file_s file_t;
typedef struct searchpath_s searchpath_t;

typedef struct
{
	int numfilenames;
	char **filenames;
	char *filenamesbuffer;
} search_t;

typedef struct fs_api_t
{
	void (*FS_AddGameDirectory)( const char *dir, unsigned int flags );
	file_t *(*FS_Open)( const char *filepath, const char *mode, qboolean gamedironly );
	fs_offset_t (*FS_Write)( file_t *file, const void *data, size_t datasize );
	fs_offset_t (*FS_Read)( file_t *file, void *buffer, size_t buffersize );
	int (*FS_Seek)( file_t *file, fs_offset_t offset, int whence );
	fs_offset_t (*FS_Tell)( file_t *file );
	qboolean (*FS_Eof)( file_t *file );
	int (*FS_Close)( file_t *file );
	int (*FS_Getc)( file_t *file );
	int (*FS_VPrintf)( file_t *file, const char *format, va_list ap );
	search_t *(*FS_Search)( const char *pattern, int caseinsensitive, int gamedironly );
	searchpath_t *(*FS_FindFile)( const char *name, int *index, qboolean gamedironly );
	fs_offset_t (*FS_FileSize)( const char *filename, qboolean gamedironly );
	int (*FS_FileTime)( const char *filename, qboolean gamedironly );
	const char *(*FS_GetDiskPath)( const char *name, qboolean gamedironly );
	searchpath_t *(*FS_GetSearchPaths)( void );
	byte *(*FS_LoadFile)( const char *path, fs_offset_t *filesizeptr, qboolean gamedironly );
	void (*FS_CreatePath)( char *path );
	void (*FS_CloseFile)( byte *buffer );
	void (*Msg)( const char *fmt, ... );
	void (*_Mem_Free)( void *data, const char *filename, int fileline );
} fs_api_t;

int FS_GetAPI( fs_api_t *g_api );

#endif
