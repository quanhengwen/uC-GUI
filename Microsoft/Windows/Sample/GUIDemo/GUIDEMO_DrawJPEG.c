/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2019  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.50 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_DrawJPEG.c
Purpose     : Sample for drawing JPEG files.
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )

              Can be used in a MS Windows environment only!
----------------------------------------------------------------------
*/

#ifndef SKIP_TEST

#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#include "GUI.h"
#include "GUIDEMO.H"
#include "FS_Win32.h"
#include "jencode_decode.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NUM_IMAGES_UNTIL_HINT 5
#define TITLE_HEIGHT          40
#define BORDER_SIZE           5
#define YPOS_IMAGE            73

#define JPEG_IMAGE_FILE       "ColorCircle_240x240_24bpp.jpg"
#define BMP_IMAGE_FILE        "ColorCircle_240x240_24bpp.bmp"
#define IMAGE_QUALITY         100

//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 200)

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawJPEGs
*
* Function description
*   Draws the given JPEG image.
*/
static void _DrawJPEGs(const char * sFileName) {
  static int      i;
  const char      acError[] = "There is possibly not enough memory to display this JPEG image.\n\nPlease assign more memory to emWin in GUIConf.c.";
  GUI_JPEG_INFO   Info;
  GUI_RECT        Rect;
  HANDLE          hFile;
  DWORD           NumBytesRead;
  DWORD           FileSize;
  char          * pFile;
  int             xSize,  ySize;
  int             xPos,   yPos;
  int             r;

  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Display file name.
  //
  Rect.x0 = BORDER_SIZE;
  Rect.y0 = TITLE_HEIGHT;
  Rect.x1 = xSize - BORDER_SIZE - 1;
  Rect.y1 = YPOS_IMAGE - 1;
  GUI_ClearRectEx(&Rect);
  GUI_SetTextMode(GUI_TM_NORMAL);
  GUI_SetFont(&GUI_Font8x16);
  GUI_DispStringInRectWrap(sFileName, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER, GUI_WRAPMODE_CHAR);
  //
  // Clear the area in which the JPEG files are displayed.
  //
  Rect.x0 = BORDER_SIZE;
  Rect.y0 = YPOS_IMAGE;
  Rect.x1 = xSize - BORDER_SIZE - 1;
  Rect.y1 = ySize - BORDER_SIZE - 1;
  GUI_ClearRectEx(&Rect);
  //
  // Load image.
  //
  hFile    = CreateFile(sFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  FileSize = GetFileSize(hFile, NULL);
  pFile    = malloc(FileSize);
  (void) ReadFile(hFile, pFile, FileSize, &NumBytesRead, NULL);
  CloseHandle(hFile);
  //
  // Check if the current JPEG image fits on the screen.
  //
  GUI_JPEG_GetInfo(pFile, FileSize, &Info);
  xSize -= BORDER_SIZE * 2 + 1;
  ySize -= YPOS_IMAGE + BORDER_SIZE + 1;
  //
  // Display the image centered.
  //
  xPos = BORDER_SIZE + (xSize - Info.XSize) / 2;
  yPos = YPOS_IMAGE  + (ySize - Info.YSize) / 2;
  GUI_SetClipRect(&Rect);
  r = GUI_JPEG_Draw(pFile, FileSize, xPos, yPos);
  GUI_SetClipRect(NULL);
  if (r) {
    //
    // The image could not be displayed successfully. Show an error message.
    //
    GUI_DispStringInRectWrap(acError, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);
  } else {
    GUIDEMO_Delay(2000);
    if ((Info.XSize > xSize) || (Info.YSize > ySize)) {
      //
      // Inform the user about the possibility of scaling JPEG images.
      //
      if (i == 0) {
        GUI_ClearRectEx(&Rect);
        GUI_DispStringInRectWrap("JPEG images can be scaled as it is shown in 2DGL_JPEG_DrawScaled.c.",  &Rect, GUI_TA_BOTTOM | GUI_TA_HCENTER, GUI_WRAPMODE_WORD);
        GUIDEMO_Delay(3000);
      }
      i++;
      if (i == NUM_IMAGES_UNTIL_HINT) {
        i = 0;
      }
    }
  }
  free(pFile);
}

/*********************************************************************
*
*       _GetFirstBitmapDirectory
*
* Function description
*   Returns the first directory which contains one or more JPG files.
*/
static int _GetFirstBitmapDirectory(char * pDir, char * pBuffer) {
  WIN32_FIND_DATA Context;
  HANDLE          hFind;
  char            acMask[_MAX_PATH];
  char            acPath[_MAX_PATH];

  sprintf(acMask, "%s\\*.jpg", pDir);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    sprintf(pBuffer, "%s\\", pDir);
    return 1;
  }
  sprintf(acMask, "%s\\*.", pDir);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if ((strcmp(Context.cFileName, ".") != 0) && (strcmp(Context.cFileName, "..") != 0)) {
        if (Context.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          sprintf(acPath, "%s\\%s", pDir, Context.cFileName);
          if (_GetFirstBitmapDirectory(acPath, pBuffer)) {
            return 1;
          }
        }
      }
    } while (FindNextFile(hFind, &Context));
  }
  return 0;
}

/*******************************************************************
*
*       _JPEGsDecodeEncode
*
*/
static void _JPEGsDecodeEncode(void) {
    //
    // File handle
    //
    HANDLE MyFile, MyFile1;
    //
    // File path
    //
    char acBuffer[_MAX_PATH];
    //
    // File system info
    //
    static const _FS_API* _pFS_API;
    //
    // Assign file system
    //
    _pFS_API = &_FS_Win32;     // Use Windows filesystem.
    //
    // Config Base Dir
    //
    sprintf(acBuffer, "%s", "..\\..\\Media\\");
    _FS_WIN32_ConfigBaseDir(acBuffer);
    //
    // Open the JPG image with read access
    //
    MyFile1 = _pFS_API->pfCreate(JPEG_IMAGE_FILE);
    //
    // Open the BMP image with read access
    //
    MyFile  = _pFS_API->pfOpenFile(BMP_IMAGE_FILE);
    //
    // encode JPG
    //
    jpeg_encode(MyFile, MyFile1, IMAGE_QUALITY);
    //
    // close file
    //
    _pFS_API->pfCloseFile(MyFile1);
    _pFS_API->pfCloseFile(MyFile);
    //
    // Open the JPG image with read access
    //
    MyFile1 = _pFS_API->pfOpenFile(JPEG_IMAGE_FILE);
    //
    // decode JPG
    //
    /* Jpeg Decoding for display to LCD */
    jpeg_decode(MyFile1);
    //
    // close file
    //
    _pFS_API->pfCloseFile(MyFile1);
}

/*******************************************************************
*
*       _DrawJPEGsFromWindowsDir
*
* Function description
*   Iterates over all JPEG files in a Windows sub folder.
*/
static void _DrawJPEGsFromWindowsDir(void) {
  WIN32_FIND_DATA Context;
  HANDLE          hFind;
  char            acPath[_MAX_PATH];
  char            acMask[_MAX_PATH];
  char            acFile[_MAX_PATH];
  char            acBuffer[_MAX_PATH];
  int             xSize;

  xSize = LCD_GetXSize();
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("Draw JPEG - Sample", xSize / 2, 5);
  sprintf(acBuffer, "%s", "..\\..\\Media\\");
  _GetFirstBitmapDirectory(acBuffer, acPath);
  sprintf(acMask, "%s*.jp*", acPath);
  hFind = FindFirstFile(acMask, &Context);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      sprintf(acFile, "%s%s", acPath, Context.cFileName);
      _DrawJPEGs(acFile);
    } while (FindNextFile(hFind, &Context));
  } else {
    GUI_DispStringHCenterAt("No JPEG files found!", 160, 60);
    GUIDEMO_Delay(2000);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_JPEGs
*/
void GUIDEMO_JPEGs(void) {
  GUIDEMO_ShowIntro("JPEGs",   "" );
  //
  // Check if recommended memory for the sample is available
  //
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  _JPEGsDecodeEncode();
  _DrawJPEGsFromWindowsDir();
  GUIDEMO_Wait();
}

#endif

/*************************** End of file ****************************/
