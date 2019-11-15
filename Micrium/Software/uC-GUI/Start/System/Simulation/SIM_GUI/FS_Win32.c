/*********************************************************************
File    : FS_Win32.c
Purpose : Implementation of file system on Windows OS
Revision: $Rev: 6176 $
*/

#include <stdio.h>
#include <string.h>
#include "FS_Win32.h"

#ifdef _WIN32
#include <windows.h>

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct {
  unsigned Pos;
  char     c;
} TRUNCATE_INFO;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static char _acBaseDir[256] = "C:\\ftp\\";

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ConvertFileName
*/
static void _ConvertFileName(char* sDest, const char* sSrc, unsigned BufferSize) {
  char c;
  const char* s;
  U32 i;

  //
  // If sSrc starts with a '/' skip it.
  //
  if (sSrc[0] == '/') {
    s = sSrc + 1;
  } else {
    s = sSrc;
  }
  //
  // Convert given filename into Windows filename.
  // e.g.:  "/pub/segger.gif"  -> "C:\ftp\pub\segger.gif"
  //
  strncpy(sDest, _acBaseDir, BufferSize);
  *(sDest + BufferSize - 1) = '\0';
  i = strlen(sDest);
  while(1) {
    c = *s++;
    if (c == '/') {
      c = '\\';
    }
    *(sDest + i++) = c;
    if (c == 0) {
      break;
    }
  }
}

/*********************************************************************
*
*       _FS_WIN32_Open
*/
static void* _FS_WIN32_Open(const char* sFilename) {
  char acFilename[256];
  HANDLE hFile;

  _ConvertFileName(acFilename, sFilename, sizeof(acFilename));
  hFile = CreateFile(acFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
    hFile = 0;
  }
  return hFile;
}

/*********************************************************************
*
*       _FS_WIN32_Close
*/
static int _FS_WIN32_Close(void* hFile) {
  CloseHandle((HANDLE)hFile);
  return 0;
}

/*********************************************************************
*
*       _FS_WIN32_Seek
*/
static long long _FS_WIN32_Seek (void* hFile, long long distance, U32 MoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer (hFile, 
                                li.LowPart, 
                                &li.HighPart, 
                                MoveMethod);

   if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
   {
      li.QuadPart = 0;
   }

   return li.QuadPart;
}

/*********************************************************************
*
*       _FS_WIN32_ReadAt
*/
static int _FS_WIN32_ReadAt(void* hFile, void* pDest, U32 Pos, U32 NumBytes) {
  DWORD NumBytesRead;
  BOOL State;
  _FS_WIN32_Seek((HANDLE)hFile, Pos, FILE_BEGIN);
  State = ReadFile((HANDLE)hFile, pDest, NumBytes, &NumBytesRead, NULL);
  if (State == FALSE && GetLastError() != NO_ERROR)
  {
      return -1;
  }
  return 0;
}

/*********************************************************************
*
*       _FS_WIN32_GetLen
*/
static long _FS_WIN32_GetLen(void* hFile) {
  DWORD SizeHigh;

  return GetFileSize((HANDLE)hFile, &SizeHigh);
}

/*********************************************************************
*
*       _FS_WIN32_ForEachDirEntry
*/
static void _FS_WIN32_ForEachDirEntry(void* pContext, const char* sDir, void (*pf)(void* pContext, void* pFileEntry)) {
  HANDLE h;
  WIN32_FIND_DATA fd;
  U32 i;
  char acFilter[256];

  strncpy(acFilter, _acBaseDir, sizeof(acFilter));
  acFilter[sizeof(acFilter) - 1] = '\0';
  if (sDir[0] == '/') {
    strcat(acFilter, sDir + 1);
  } else {
    strcat(acFilter, sDir);
  }
  strcat(acFilter, "*.*");

  h = FindFirstFile(acFilter, &fd);
  if (h != INVALID_HANDLE_VALUE) {
    for (i = 0; ; i++) {
      pf(pContext, &fd);
      if (FindNextFile(h, &fd) == 0) {
        break;
      }
    }
  }
  FindClose(h);
}

/*********************************************************************
*
*       _FS_WIN32_GetDirEntryFileName
*/
static void _FS_WIN32_GetDirEntryFileName(void* pFileEntry, char* sFileName, U32 SizeOfBuffer) {
  WIN32_FIND_DATA* pFD;

  pFD = (WIN32_FIND_DATA*)pFileEntry;

  strncpy(sFileName, pFD->cFileName, SizeOfBuffer);
  *(sFileName + SizeOfBuffer - 1) = 0;
}

/*********************************************************************
*
*       _FS_WIN32_GetDirEntryFileSize
*/
static U32 _FS_WIN32_GetDirEntryFileSize (void* pFileEntry, U32* pFileSizeHigh) {
  WIN32_FIND_DATA* pFD;

  pFD = (WIN32_FIND_DATA*)pFileEntry;
  return pFD->nFileSizeLow;
}

/*********************************************************************
*
*       _FS_WIN32_GetDirEntryFileTime
*/
static U32 _FS_WIN32_GetDirEntryFileTime (void* pFileEntry) {
  WIN32_FIND_DATA* pFD;
  U16 Date, Time;

  pFD = (WIN32_FIND_DATA*)pFileEntry;
  FileTimeToDosDateTime(&pFD->ftLastWriteTime, &Date, &Time);
  return (Date << 16) | Time;
}


/*********************************************************************
*
*       _FS_WIN32_GetDirEntryAttributes
*
*  Return value
*    bit 0   - 0: File, 1:Directory
*/
static int _FS_WIN32_GetDirEntryAttributes(void* pFileEntry) {
  WIN32_FIND_DATA* pFD;

  pFD = (WIN32_FIND_DATA*)pFileEntry;
  return (pFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
}

/*********************************************************************
*
*       _FS_WIN32_Create
*/
static void* _FS_WIN32_Create(const char* sFileName) {
  char acFilename[256];
  HANDLE hFile;

  _ConvertFileName(acFilename, sFileName, sizeof(acFilename));
  hFile = CreateFile(acFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
    hFile = 0;
  }
  return hFile;
}

/*********************************************************************
*
*       _FS_WIN32_WriteAt
*/
static int _FS_WIN32_WriteAt(void* hFile, void* pBuffer, U32 Pos, U32 NumBytes) {
  U32 NumBytesWritten;
  U32 r;

  _FS_WIN32_Seek((HANDLE)hFile, Pos, FILE_BEGIN);
  r = WriteFile((HANDLE)hFile, pBuffer, NumBytes, &NumBytesWritten, NULL);
  if (r) {
    return NumBytesWritten;
  } else {
    return -1;
  }
}

/*********************************************************************
*
*       _FS_WIN32_DeleteFile
*/
static void* _FS_WIN32_DeleteFile(const char* sFilename) {
  char acFilename[256];

  _ConvertFileName(acFilename, sFilename, sizeof(acFilename));
  return (void*)(DeleteFile(acFilename) ? 0 : -1);
}

/*********************************************************************
*
*       _TruncateToDirName()
*
*  Function description
*    Truncates a path to its dirname.
*
*  Parameters
*    sPath         : Path to truncate to dirname.
*    NumBytesToSkip: Number of bytes to skip from the end (not counting
*                    the string termination).
*    pInfo         : Pointer to element of TRUNCATE_INFO that can be
*                    used to restore the original string if needed.
*                    Can be NULL.
*/
static void _TruncateToDirName(char* sPath, unsigned NumBytesToSkip, TRUNCATE_INFO* pInfo) {
  unsigned Len;

  Len = strlen(sPath) - NumBytesToSkip;
  //
  // Search for '\\' from the end of the string.
  //
  do {
    Len--;
  } while (*(sPath + Len) != '\\');
  Len++;
  if (pInfo != NULL) {
    //
    // Save which character was replaced and where.
    //
    pInfo->Pos = Len;
    pInfo->c   = *(sPath + Len);
  }
  *(sPath + Len) = '\0';
}

/*********************************************************************
*
*       _FS_WIN32_RenameFile
*/
static int _FS_WIN32_RenameFile(const char* sOldFilename, const char* sNewFilename) {
  char acOldFilename[256];
  char acNewFilename[256];
  unsigned      Len;
  unsigned      LenNew;
  TRUNCATE_INFO TruncInfo;

  _ConvertFileName(acOldFilename, sOldFilename, sizeof(acOldFilename));
  strncpy(acNewFilename, acOldFilename, sizeof(acNewFilename));
  _TruncateToDirName(acNewFilename, 1, &TruncInfo);  // Get directory by skipping last char (possible '\\' and searching until next '\\').
  Len    = strlen(acNewFilename);
  LenNew = strlen(sNewFilename);
  if ((Len + LenNew) >= sizeof(acNewFilename)) {
    return -1;  // New filename does not fit into buffer.
  }
  strcat(acNewFilename, sNewFilename);
  return rename(acOldFilename, acNewFilename);
}

/*********************************************************************
*
*       _FS_WIN32_MakeDir
*/
static int _FS_WIN32_MakeDir(const char* sDirname) {
  char acDirname[256];

  _ConvertFileName(acDirname, sDirname, sizeof(acDirname));
  return CreateDirectory(acDirname, NULL) ? 0 : -1;
}

/*********************************************************************
*
*       _FS_WIN32_RemoveDir
*/
static int _FS_WIN32_RemoveDir(const char* sDirname) {
  char acDirname[256];

  _ConvertFileName(acDirname, sDirname, sizeof(acDirname));
  return RemoveDirectory(acDirname) ? 0 : -1;
}

/*********************************************************************
*
*       _FS_WIN32_IsFolder
*
* Parameters
*   sPath: Path to be checked if it is a file or a folder
*          e.g. "/sub/" or "/sub".
*
* Return value
*   Path does NOT exist: -1.
*   Path is a file     :  0.
*   Path is a folder   :  1.
*/
static int _FS_WIN32_IsFolder(const char* sDirname) {
  char acDirname[256];
  U32  Attr;
  
  _ConvertFileName(acDirname, sDirname, sizeof(acDirname));
  Attr = GetFileAttributes(acDirname);
  if (((int)Attr) == -1) {  // Old MSVC versions do not know INVALID_FILE_ATTRIBUTES .
    return -1;
  }
  if (Attr & FILE_ATTRIBUTE_DIRECTORY) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _FS_WIN32_MoveFile
*/
static int _FS_WIN32_MoveFile(const char* sOldFilename, const char* sNewFilename) {
  char acOldFilename[256];
  char acNewFilename[256];

  _ConvertFileName(acOldFilename, sOldFilename, sizeof(acOldFilename));
  _ConvertFileName(acNewFilename, sNewFilename, sizeof(acNewFilename));
  return (MoveFile(acOldFilename, acNewFilename) ? 0 : -1);
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

const _FS_API _FS_Win32 = {
  //
  // Read only file operations.
  //
  _FS_WIN32_Open,
  _FS_WIN32_Close,
  _FS_WIN32_Seek,
  _FS_WIN32_ReadAt,
  _FS_WIN32_GetLen,
  //
  // Simple directory operations.
  //
  _FS_WIN32_ForEachDirEntry,
  _FS_WIN32_GetDirEntryFileName,
  _FS_WIN32_GetDirEntryFileSize,
  _FS_WIN32_GetDirEntryFileTime,
  _FS_WIN32_GetDirEntryAttributes,
  //
  // Simple write type file operations.
  //
  _FS_WIN32_Create,
  _FS_WIN32_DeleteFile,
  _FS_WIN32_RenameFile,
  _FS_WIN32_WriteAt,
  //
  // Additional directory operations
  //
  _FS_WIN32_MakeDir,
  _FS_WIN32_RemoveDir,
  //
  // Additional operations
  //
  _FS_WIN32_IsFolder,
  _FS_WIN32_MoveFile,
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       _FS_WIN32_ConfigBaseDir
*/
void _FS_WIN32_ConfigBaseDir(const char* sDir) {
  if (strlen(sDir) < sizeof(_acBaseDir)) {
    strncpy(_acBaseDir, sDir, sizeof(_acBaseDir));
  }
}

#endif  // _WIN32

/*************************** End of file ****************************/
