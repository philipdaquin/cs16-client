/*
filesystem_impl.cpp - xash filesystem_stdio
Copyright (C) 2016-2017 a1batross
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, or
(at your option) any later version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <string>

#include <FileSystem.h>

namespace
{
struct FindState
{
	DIR *dir = nullptr;
	std::string directory;
	std::string pattern;
	std::string current;
};

static void SplitSearchPattern( const char *pattern, std::string &directory, std::string &filePattern )
{
	const char *slash = strrchr( pattern, '/' );
	const char *backslash = strrchr( pattern, '\\' );
	const char *sep = slash;

	if( backslash && (!sep || backslash > sep ))
		sep = backslash;

	if( sep )
	{
		directory.assign( pattern, sep - pattern );
		filePattern.assign( sep + 1 );
	}
	else
	{
		directory = ".";
		filePattern = pattern;
	}

	if( directory.empty() )
		directory = ".";
	if( filePattern.empty() )
		filePattern = "*";
}

static std::string JoinPath( const std::string &dir, const char *name )
{
	if( dir.empty() || dir == "." )
		return std::string( name );

	std::string result = dir;
	if( result.back() != '/' && result.back() != '\\' )
		result.push_back( '/' );
	result.append( name );
	return result;
}

static bool IsDir( const std::string &path )
{
	struct stat st;
	return stat( path.c_str(), &st ) == 0 && S_ISDIR( st.st_mode );
}

class CStdFileSystem : public IFileSystem
{
public:
	void Mount( void ) override {}
	void Unmount( void ) override {}
	void RemoveAllSearchPaths( void ) override {}
	void AddSearchPath( const char *, const char * ) override {}
	bool RemoveSearchPath( const char * ) override { return true; }
	void RemoveFile( const char *pRelativePath, const char * ) override { unlink( pRelativePath ); }
	void CreateDirHierarchy( const char *path, const char * ) override
	{
		if( !path || !*path )
			return;

		std::string current;
		for( const char *p = path; *p; ++p )
		{
			current.push_back( *p );
			if( *p == '/' || *p == '\\' )
				mkdir( current.c_str(), 0755 );
		}
		mkdir( current.c_str(), 0755 );
	}
	bool FileExists( const char *pFileName ) override
	{
		struct stat st;
		return pFileName && stat( pFileName, &st ) == 0;
	}
	bool IsDirectory( const char *pFileName ) override
	{
		return pFileName && IsDir( pFileName );
	}
	FileHandle_t Open( const char *pFileName, const char *pOptions, const char * ) override
	{
		return pFileName && pOptions ? (FileHandle_t)fopen( pFileName, pOptions ) : FILESYSTEM_INVALID_HANDLE;
	}
	void Close( FileHandle_t file ) override { if( file ) fclose( (FILE *)file ); }
	void Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType ) override
	{
		if( !file ) return;
		int origin = seekType == FILESYSTEM_SEEK_HEAD ? SEEK_SET : seekType == FILESYSTEM_SEEK_CURRENT ? SEEK_CUR : SEEK_END;
		fseek( (FILE *)file, pos, origin );
	}
	unsigned Tell( FileHandle_t file ) override { return file ? (unsigned)ftell( (FILE *)file ) : 0; }
	unsigned Size( FileHandle_t file ) override
	{
		if( !file ) return 0;
		long cur = ftell( (FILE *)file );
		fseek( (FILE *)file, 0, SEEK_END );
		long end = ftell( (FILE *)file );
		fseek( (FILE *)file, cur, SEEK_SET );
		return end < 0 ? 0u : (unsigned)end;
	}
	unsigned Size( const char *pFileName ) override
	{
		struct stat st;
		return pFileName && stat( pFileName, &st ) == 0 ? (unsigned)st.st_size : 0u;
	}
	long GetFileTime( const char *pFileName ) override
	{
		struct stat st;
		return pFileName && stat( pFileName, &st ) == 0 ? (long)st.st_mtime : -1;
	}
	void FileTimeToString( char *pStrip, int maxCharsIncludingTerminator, long fileTime ) override
	{
		if( !pStrip || maxCharsIncludingTerminator <= 0 )
			return;
		time_t t = (time_t)fileTime;
		char *s = ctime( &t );
		if( !s )
		{
			pStrip[0] = 0;
			return;
		}
		strncpy( pStrip, s, maxCharsIncludingTerminator );
		pStrip[maxCharsIncludingTerminator - 1] = 0;
	}
	bool IsOk( FileHandle_t file ) override { return file && !ferror( (FILE *)file ); }
	void Flush( FileHandle_t file ) override { if( file ) fflush( (FILE *)file ); }
	bool EndOfFile( FileHandle_t file ) override { return file ? feof( (FILE *)file ) != 0 : true; }
	int Read( void *pOutput, int size, FileHandle_t file ) override { return file ? (int)fread( pOutput, 1, size, (FILE *)file ) : 0; }
	int Write( const void *pInput, int size, FileHandle_t file ) override { return file ? (int)fwrite( pInput, 1, size, (FILE *)file ) : 0; }
	char *ReadLine( char *pOutput, int maxChars, FileHandle_t file ) override { return file && fgets( pOutput, maxChars, (FILE *)file ) ? pOutput : nullptr; }
	int FPrintf( FileHandle_t file, const char *pFormat, ... ) override
	{
		if( !file || !pFormat ) return 0;
		va_list ap;
		va_start( ap, pFormat );
		int ret = vfprintf( (FILE *)file, pFormat, ap );
		va_end( ap );
		return ret;
	}
	char *GetReadBuffer( FileHandle_t, char * ) override { return nullptr; }
	void ReleaseReadBuffer( FileHandle_t, char * ) override {}
	const char *FindFirst( const char *pWildCard, FileFindHandle_t *pHandle, const char * ) override
	{
		if( !pWildCard || !pHandle )
			return nullptr;

		auto *state = new FindState();
		if( !state )
			return nullptr;

		SplitSearchPattern( pWildCard, state->directory, state->pattern );
		state->dir = opendir( state->directory.c_str() );
		if( !state->dir )
		{
			delete state;
			return nullptr;
		}

		struct dirent *ent = nullptr;
		while( ( ent = readdir( state->dir )) != nullptr )
		{
			if( fnmatch( state->pattern.c_str(), ent->d_name, 0 ) == 0 )
			{
				state->current = JoinPath( state->directory, ent->d_name );
				*pHandle = (FileFindHandle_t)state;
				return state->current.c_str();
			}
		}

		FindClose( (FileFindHandle_t)state );
		return nullptr;
	}
	const char *FindNext( FileFindHandle_t handle ) override
	{
		auto *state = (FindState *)handle;
		if( !state || !state->dir )
			return nullptr;

		struct dirent *ent = nullptr;
		while( ( ent = readdir( state->dir )) != nullptr )
		{
			if( fnmatch( state->pattern.c_str(), ent->d_name, 0 ) == 0 )
			{
				state->current = JoinPath( state->directory, ent->d_name );
				return state->current.c_str();
			}
		}
		return nullptr;
	}
	bool FindIsDirectory( FileFindHandle_t handle ) override
	{
		auto *state = (FindState *)handle;
		return state ? IsDir( state->current ) : false;
	}
	void FindClose( FileFindHandle_t handle ) override
	{
		auto *state = (FindState *)handle;
		if( !state )
			return;
		if( state->dir )
			closedir( state->dir );
		delete state;
	}
	void GetLocalCopy( const char * ) override {}
	const char *GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize ) override
	{
		if( !pFileName || !pLocalPath || localPathBufferSize <= 0 )
			return nullptr;
		strncpy( pLocalPath, pFileName, localPathBufferSize );
		pLocalPath[localPathBufferSize - 1] = 0;
		return pLocalPath;
	}
	char *ParseFile( char *data, char *token, bool *wasquoted ) override
	{
		if( !data || !token )
			return nullptr;
		if( wasquoted ) *wasquoted = false;
		while( *data && *data <= ' ' ) ++data;
		if( !*data ) { token[0] = 0; return data; }
		char *out = token;
		if( *data == '"' )
		{
			if( wasquoted ) *wasquoted = true;
			++data;
			while( *data && *data != '"' ) *out++ = *data++;
			if( *data == '"' ) ++data;
		}
		else
		{
			while( *data && *data > ' ' ) *out++ = *data++;
		}
		*out = 0;
		return data;
	}
	bool FullPathToRelativePath( const char *pFullpath, char *pRelative ) override
	{
		if( !pFullpath || !pRelative )
			return false;
		strncpy( pRelative, pFullpath, MAX_PATH );
		pRelative[MAX_PATH - 1] = 0;
		return false;
	}
	bool GetCurrentDirectory( char *pDirectory, int maxlen ) override
	{
		return getcwd( pDirectory, maxlen ) != nullptr;
	}
	void PrintOpenedFiles( void ) override {}
	void SetWarningFunc( void (*)( const char *, ... ) ) override {}
	void SetWarningLevel( FileWarningLevel_t ) override {}
	void LogLevelLoadStarted( const char * ) override {}
	void LogLevelLoadFinished( const char * ) override {}
	int HintResourceNeed( const char *, int ) override { return 0; }
	int PauseResourcePreloading( void ) override { return 0; }
	int ResumeResourcePreloading( void ) override { return 0; }
	int SetVBuf( FileHandle_t stream, char *buffer, int mode, long size ) override
	{
		return stream ? setvbuf( (FILE *)stream, buffer, mode, size ) : -1;
	}
	void GetInterfaceVersion( char *p, int maxlen ) override
	{
		if( p && maxlen > 0 )
			strncpy( p, "stdio-local", maxlen );
			p[maxlen - 1] = 0;
	}
	bool IsFileImmediatelyAvailable( const char *pFileName ) override { return FileExists( pFileName ); }
	WaitForResourcesHandle_t WaitForResources( const char * ) override { return 0; }
	bool GetWaitForResourcesProgress( WaitForResourcesHandle_t, float *progress, bool *complete ) override
	{
		if( progress ) *progress = 1.0f;
		if( complete ) *complete = true;
		return true;
	}
	void CancelWaitForResources( WaitForResourcesHandle_t ) override {}
	bool IsAppReadyForOfflinePlay( ) override { return true; }
	bool AddPackFile( const char *, const char * ) override { return false; }
	FileHandle_t OpenFromCacheForRead( const char *pFileName, const char *pOptions, const char * ) override
	{
		return Open( pFileName, pOptions, nullptr );
	}
	void AddSearchPathNoWrite( const char *, const char * ) {}
};

static CStdFileSystem g_StdioFileSystem;
}

IFileSystem *filesystem( void )
{
	return &g_StdioFileSystem;
}

#include <sys/stat.h>
#include <tier1/strtools.h>

#include "filesystem.h"

/*
Legacy engine-owned bootstrap path, kept here for reference:

#include "library.h"

static CSysModule *g_hFileSystemModule = nullptr;
IFileSystem *g_pFileSystem = nullptr;

bool FileSystem_Init()
{
    if( g_pFileSystem )
        return true;

    g_hFileSystemModule = Sys_LoadModule( STDIO_FILESYSTEM_LIB );
    if( !g_hFileSystemModule )
        return false;

    CreateInterfaceFn filesystemFactoryFn = Sys_GetFactory( g_hFileSystemModule );
    if( !filesystemFactoryFn )
        return false;

    g_pFileSystem = (IFileSystem *)filesystemFactoryFn( FILESYSTEM_INTERFACE_VERSION, nullptr );
    return g_pFileSystem != nullptr;
}

void FileSystem_Shutdown()
{
    if( g_hFileSystemModule )
    {
        Sys_UnloadModule( g_hFileSystemModule );
        g_hFileSystemModule = nullptr;
    }

    g_pFileSystem = nullptr;
}
*/

#if 0

static CSysModule *g_hFileSystemModule = nullptr;
IFileSystem *g_pFileSystem = nullptr;

static const char *GetStdioFilesystemLib()
{
#ifdef _WIN32
    return "filesystem_stdio.dll";
#elif defined(__EMSCRIPTEN__)
    return "filesystem_stdio.wasm";
#elif defined(__ANDROID__)
    return "libfilesystem_stdio.so";
#elif defined(__APPLE__)
    return "filesystem_stdio.dylib";
#else
    return "filesystem_stdio.so";
#endif
}

bool FileSystem_Init()
{
    if( g_pFileSystem )
        return true;

    g_hFileSystemModule = Sys_LoadModule( GetStdioFilesystemLib() );
    if( !g_hFileSystemModule )
        return false;

    CreateInterfaceFn filesystemFactoryFn = Sys_GetFactory( g_hFileSystemModule );
    if( !filesystemFactoryFn )
        return false;

    g_pFileSystem = (IFileSystem *)filesystemFactoryFn( FILESYSTEM_INTERFACE_VERSION, nullptr );
    return g_pFileSystem != nullptr;
}

void FileSystem_Shutdown()
{
    if( g_hFileSystemModule )
    {
        Sys_UnloadModule( g_hFileSystemModule );
        g_hFileSystemModule = nullptr;
    }

    g_pFileSystem = nullptr;
}

class CXashFileSystem : public IFileSystem
{
public:
    CXashFileSystem();
    CXashFileSystem(bool mBMounted);

    void Mount( void );
    void Unmount( void );
    void RemoveAllSearchPaths( void );
    void AddSearchPath( const char *pPath, const char *pathID );
    bool RemoveSearchPath( const char *pPath );
    void RemoveFile( const char *pRelativePath, const char *pathID );
    void CreateDirHierarchy( const char *path, const char *pathID );
    bool FileExists( const char *pFileName );
    bool IsDirectory( const char *pFileName );
    FileHandle_t Open( const char *pFileName, const char *pOptions, const char *pathIDL );
    void Close( FileHandle_t file );
    void Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType );
    unsigned int Tell( FileHandle_t file );
    unsigned int Size( FileHandle_t file );
    unsigned int Size( const char *pFileName );
    long GetFileTime( const char *pFileName );
    void FileTimeToString( char* pStrip, int maxCharsIncludingTerminator, long fileTime );
    bool IsOk( FileHandle_t file );
    void Flush( FileHandle_t file );
    bool EndOfFile( FileHandle_t file );
    int Read( void* pOutput, int size, FileHandle_t file );
    int Write( void const* pInput, int size, FileHandle_t file );
    char* ReadLine( char *pOutput, int maxChars, FileHandle_t file );
    int FPrintf( FileHandle_t file, const char *pFormat, ... );
    char *GetReadBuffer(FileHandle_t file, char *pBuffer);
    void ReleaseReadBuffer( FileHandle_t file, char *readBuffer );
    const char* FindFirst( const char *pWildCard, FileFindHandle_t *pHandle, const char *pathIDL );
    const char* FindNext( FileFindHandle_t handle );
    bool FindIsDirectory( FileFindHandle_t handle );
    void FindClose( FileFindHandle_t handle );
    void GetLocalCopy( const char *pFileName );
    const char* GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize );
    char* ParseFile( char* pFileBytes, char* pToken, bool* pWasQuoted );
    bool FullPathToRelativePath( const char *pFullpath, char *pRelative );
    bool GetCurrentDirectory( char* pDirectory, int maxlen );
    void PrintOpenedFiles( void );
    void SetWarningFunc( void (*pfnWarning)( const char *fmt, ... ) );
    void SetWarningLevel( FileWarningLevel_t level );
    void LogLevelLoadStarted( const char *name );
    void LogLevelLoadFinished( const char *name );
    int HintResourceNeed( const char *hintlist, int forgetEverything );
    int PauseResourcePreloading( void );
    int ResumeResourcePreloading( void );
    int SetVBuf( FileHandle_t stream, char *buffer, int mode, long size );
    void GetInterfaceVersion( char *p, int maxlen );
    bool IsFileImmediatelyAvailable(const char *pFileName);
    WaitForResourcesHandle_t WaitForResources( const char *resourcelist );
    bool GetWaitForResourcesProgress( WaitForResourcesHandle_t handle, float *progress, bool *complete );
    void CancelWaitForResources( WaitForResourcesHandle_t handle );
    bool IsAppReadyForOfflinePlay( );
    bool AddPackFile( const char *fullpath, const char *pathID );
    FileHandle_t OpenFromCacheForRead( const char *pFileName, const char *pOptions, const char *pathIDL );
    void AddSearchPathNoWrite( const char *pPath, const char *pathID );

private:
    IFileSystem *Backend();
    bool IsGameDir( const char *pathID );

    bool m_bMounted;
};

static CXashFileSystem fs;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CXashFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION, fs )

CXashFileSystem *XashFileSystem( void )
{
    return &fs;
}

static IFileSystem *GetBackendFileSystem()
{
    if( !g_pFileSystem )
    {
        if( !FileSystem_Init() )
            return nullptr;
    }

    return g_pFileSystem;
}

IFileSystem *filesystem( void )
{
    return GetBackendFileSystem();
}

IFileSystem *CXashFileSystem::Backend()
{
    return GetBackendFileSystem();
}

CXashFileSystem::CXashFileSystem()
{
    m_bMounted = false;
}

CXashFileSystem::CXashFileSystem(bool mBMounted)
{
    m_bMounted = mBMounted;
}

void CXashFileSystem::Mount( void )
{
    if( auto *fs = Backend() )
        fs->Mount();
    m_bMounted = true;
}

void CXashFileSystem::Unmount( void )
{
    if( auto *fs = Backend() )
        fs->Unmount();
    m_bMounted = false;
}

void CXashFileSystem::RemoveAllSearchPaths( void )
{
    if( auto *fs = Backend() )
        fs->RemoveAllSearchPaths();
}

void CXashFileSystem::AddSearchPath(const char *pPath, const char *pathID)
{
    if( auto *fs = Backend() )
        fs->AddSearchPath( pPath, pathID );
}

bool CXashFileSystem::RemoveSearchPath(const char *pPath)
{
    if( auto *fs = Backend() )
        return fs->RemoveSearchPath( pPath );
    return false;
}

void CXashFileSystem::RemoveFile(const char *pRelativePath, const char *pathID)
{
    if( auto *fs = Backend() )
        fs->RemoveFile( pRelativePath, pathID );
}

void CXashFileSystem::CreateDirHierarchy(const char *path, const char *pathID)
{
    if( auto *fs = Backend() )
        fs->CreateDirHierarchy( path, pathID );
}

bool CXashFileSystem::FileExists(const char *pFileName)
{
    return Backend() ? Backend()->FileExists( pFileName ) : false;
}

bool CXashFileSystem::IsDirectory(const char *pFileName)
{
    return Backend() ? Backend()->IsDirectory( pFileName ) : false;
}

FileHandle_t CXashFileSystem::Open(const char *pFileName, const char *pOptions, const char *pathID)
{
    return Backend() ? Backend()->Open( pFileName, pOptions, pathID ) : FILESYSTEM_INVALID_HANDLE;
}

void CXashFileSystem::Close( FileHandle_t file )
{
    if( auto *fs = Backend() )
        fs->Close( file );
}

void CXashFileSystem::Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType )
{
    if( auto *fs = Backend() )
        fs->Seek( file, pos, seekType );
}

unsigned int CXashFileSystem::Tell(FileHandle_t file)
{
    return Backend() ? Backend()->Tell( file ) : 0;
}

unsigned int CXashFileSystem::Size(FileHandle_t file)
{
    return Backend() ? Backend()->Size( file ) : 0;
}

unsigned int CXashFileSystem::Size(const char *pFileName)
{
    return Backend() ? Backend()->Size( pFileName ) : 0;
}

long CXashFileSystem::GetFileTime(const char *pFileName)
{
    return Backend() ? Backend()->GetFileTime( pFileName ) : -1;
}

void CXashFileSystem::FileTimeToString(char *pStrip, int maxCharsIncludingTerminator, long fileTime)
{
    if( auto *fs = Backend() )
        fs->FileTimeToString( pStrip, maxCharsIncludingTerminator, fileTime );
}

bool CXashFileSystem::IsOk(FileHandle_t file)
{
    return Backend() ? Backend()->IsOk( file ) : false;
}

void CXashFileSystem::Flush(FileHandle_t file)
{
    if( auto *fs = Backend() )
        fs->Flush( file );
}

bool CXashFileSystem::EndOfFile(FileHandle_t file)
{
    return Backend() ? Backend()->EndOfFile( file ) : true;
}

int CXashFileSystem::Read( void *pOutput, int size, FileHandle_t file )
{
    return Backend() ? Backend()->Read( pOutput, size, file ) : 0;
}

int CXashFileSystem::Write(const void *pInput, int size, FileHandle_t file)
{
    return Backend() ? Backend()->Write( pInput, size, file ) : 0;
}

char *CXashFileSystem::ReadLine(char *pOutput, int maxChars, FileHandle_t file)
{
    return Backend() ? Backend()->ReadLine( pOutput, maxChars, file ) : nullptr;
}

int CXashFileSystem::FPrintf(FileHandle_t file, const char *pFormat, ...)
{
    va_list args;
    va_start( args, pFormat );
    int result = Backend() ? Backend()->FPrintf( file, pFormat, args ) : 0;
    va_end( args );
    return result;
}

char *CXashFileSystem::GetReadBuffer(FileHandle_t file, char *pBuffer)
{
    return Backend() ? Backend()->GetReadBuffer( file, pBuffer ) : nullptr;
}

void CXashFileSystem::ReleaseReadBuffer(FileHandle_t file, char *readBuffer)
{
    if( auto *fs = Backend() )
        fs->ReleaseReadBuffer( file, readBuffer );
}

const char *CXashFileSystem::FindFirst(const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID)
{
    return Backend() ? Backend()->FindFirst( pWildCard, pHandle, pathID ) : nullptr;
}

const char *CXashFileSystem::FindNext(FileFindHandle_t handle)
{
    return Backend() ? Backend()->FindNext( handle ) : nullptr;
}

bool CXashFileSystem::FindIsDirectory(FileFindHandle_t handle)
{
    return Backend() ? Backend()->FindIsDirectory( handle ) : false;
}

void CXashFileSystem::FindClose(FileFindHandle_t handle)
{
    if( auto *fs = Backend() )
        fs->FindClose( handle );
}

void CXashFileSystem::GetLocalCopy(const char *pFileName)
{
    if( auto *fs = Backend() )
        fs->GetLocalCopy( pFileName );
}

const char* CXashFileSystem::GetLocalPath(const char *pFileName, char *pLocalPath, int localPathBufferSize)
{
    return Backend() ? Backend()->GetLocalPath( pFileName, pLocalPath, localPathBufferSize ) : nullptr;
}

char *CXashFileSystem::ParseFile(char *pFileBytes, char *pToken, bool *pWasQuoted)
{
    if( !pFileBytes || !pToken )
        return nullptr;

    if( pWasQuoted )
        *pWasQuoted = false;

    char *p = pFileBytes;
    while( *p && *p <= ' ' )
        ++p;

    if( !*p )
    {
        pToken[0] = 0;
        return p;
    }

    char *out = pToken;
    if( *p == '"' )
    {
        if( pWasQuoted )
            *pWasQuoted = true;
        ++p;
        while( *p && *p != '"' )
            *out++ = *p++;
        if( *p == '"' )
            ++p;
    }
    else
    {
        while( *p > ' ' )
            *out++ = *p++;
    }

    *out = 0;
    return p;
}

bool CXashFileSystem::FullPathToRelativePath(const char *pFullpath, char *pRelative)
{
    if( !pFullpath || !pRelative )
        return false;

    if( Backend() && Backend()->FullPathToRelativePath( pFullpath, pRelative ) )
        return true;

	    strncpy( pRelative, pFullpath, MAX_PATH );
	    pRelative[MAX_PATH - 1] = 0;
    return false;
}

bool CXashFileSystem::GetCurrentDirectory(char *pDirectory, int maxlen)
{
    return Backend() ? Backend()->GetCurrentDirectory( pDirectory, maxlen ) : false;
}

void CXashFileSystem::PrintOpenedFiles()
{
    if( auto *fs = Backend() )
        fs->PrintOpenedFiles();
}

void CXashFileSystem::SetWarningFunc(void (*pfnWarning)(const char *, ...))
{
    if( auto *fs = Backend() )
        fs->SetWarningFunc( pfnWarning );
}

void CXashFileSystem::SetWarningLevel(FileWarningLevel_t level)
{
    if( auto *fs = Backend() )
        fs->SetWarningLevel( level );
}

void CXashFileSystem::LogLevelLoadStarted(const char *name)
{
    if( auto *fs = Backend() )
        fs->LogLevelLoadStarted( name );
}

void CXashFileSystem::LogLevelLoadFinished(const char *name)
{
    if( auto *fs = Backend() )
        fs->LogLevelLoadFinished( name );
}

int CXashFileSystem::HintResourceNeed(const char *hintlist, int forgetEverything)
{
    return Backend() ? Backend()->HintResourceNeed( hintlist, forgetEverything ) : 0;
}

int CXashFileSystem::PauseResourcePreloading()
{
    return Backend() ? Backend()->PauseResourcePreloading() : 0;
}

int CXashFileSystem::ResumeResourcePreloading()
{
    return Backend() ? Backend()->ResumeResourcePreloading() : 0;
}

int CXashFileSystem::SetVBuf(FileHandle_t stream, char *buffer, int mode, long size)
{
    return Backend() ? Backend()->SetVBuf( stream, buffer, mode, size ) : 0;
}

void CXashFileSystem::GetInterfaceVersion(char *p, int maxlen)
{
    if( auto *fs = Backend() )
        fs->GetInterfaceVersion( p, maxlen );
    else if( p && maxlen > 0 )
        p[0] = 0;
}

bool CXashFileSystem::IsFileImmediatelyAvailable(const char *pFileName)
{
    return Backend() ? Backend()->IsFileImmediatelyAvailable( pFileName ) : false;
}

WaitForResourcesHandle_t CXashFileSystem::WaitForResources(const char *resourcelist)
{
    return Backend() ? Backend()->WaitForResources( resourcelist ) : 0;
}

bool CXashFileSystem::GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float *progress, bool *complete)
{
    return Backend() ? Backend()->GetWaitForResourcesProgress( handle, progress, complete ) : false;
}

void CXashFileSystem::CancelWaitForResources(WaitForResourcesHandle_t handle)
{
    if( auto *fs = Backend() )
        fs->CancelWaitForResources( handle );
}

bool CXashFileSystem::IsAppReadyForOfflinePlay()
{
    return Backend() ? Backend()->IsAppReadyForOfflinePlay() : true;
}

bool CXashFileSystem::AddPackFile(const char *fullpath, const char *pathID)
{
    return Backend() ? Backend()->AddPackFile( fullpath, pathID ) : false;
}

FileHandle_t CXashFileSystem::OpenFromCacheForRead(const char *pFileName, const char *pOptions, const char *pathID)
{
    return Backend() ? Backend()->OpenFromCacheForRead( pFileName, pOptions, pathID ) : FILESYSTEM_INVALID_HANDLE;
}

void CXashFileSystem::AddSearchPathNoWrite(const char *pPath, const char *pathID)
{
    if( auto *fs = Backend() )
        fs->AddSearchPath( pPath, pathID );
}

bool CXashFileSystem::IsGameDir(const char *pathID)
{
    return pathID && (strstr( pathID, "GAME" ) || strstr( pathID, "CONFIG" ) || strstr( pathID, "GAMECONFIG" ));
}
#endif

#ifndef ALLOCA_H
#if defined(_WIN32)
#define ALLOCA_H <malloc.h>
#else
#define ALLOCA_H <alloca.h>
#endif
#endif
