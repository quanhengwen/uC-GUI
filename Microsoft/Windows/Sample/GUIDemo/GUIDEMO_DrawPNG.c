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
File        : GUIDEMO_DrawPNG.c
Purpose     : Example for drawing PNG images
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - (x)
              TrueTypeFonts - ( )

              Can be used in a MS Windows environment only!
----------------------------------------------------------------------
*/

#ifndef SKIP_TEST

#include <windows.h>
#include "GUI.h"
#include "GUIDEMO.H"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 160)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawStripes
*/
static void _DrawStripes(void) {
  int i;

  GUI_COLOR _aColor[] = {
    GUI_BLUE   ,
    GUI_GREEN  ,
    GUI_RED    ,
    GUI_CYAN   ,
    GUI_MAGENTA,
    GUI_YELLOW ,
  };
  for (i = 0; i < 480; i += 10) {
    GUI_SetColor(_aColor[(i / 10) % GUI_COUNTOF(_aColor)]);
    GUI_FillRect(0, i, 640, i + 9);
  }
}

/*********************************************************************
*
*       _GetData
*/
static int _GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) {
  HANDLE * phFile;
  DWORD    NumBytesRead;
  U8     * pData;

  pData  = (U8 *)*ppData;
  phFile = (HANDLE *)p;
  //
  // Set file pointer to the required position
  //
  SetFilePointer(*phFile, Off, 0, FILE_BEGIN);
  //
  // Read data into buffer
  //
  ReadFile(*phFile, pData, NumBytesReq, &NumBytesRead, NULL);
  //
  // Return number of available bytes
  //
  return NumBytesRead;
}

/*********************************************************************
*
*       _DrawPNG_file
*
* Function description
*   Draws a PNG file from a file system
*/
static void _DrawPNG_file(const char * sFilename) {
  HANDLE hFile;
  int    xSize;
  int    ySize;
  int    w;
  int    h;
  int    xPos;
  int    yPos;

  hFile = CreateFile(sFilename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  w     = GUI_PNG_GetXSizeEx(_GetData, &hFile);
  h     = GUI_PNG_GetYSizeEx(_GetData, &hFile);
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  xPos  = (xSize - w) / 2 + 10;
  yPos  = (ySize - h) / 2 + 10;
  GUI_PNG_DrawEx(_GetData, &hFile, xPos, yPos);
  CloseHandle(hFile);
}

/*********************************************************************
*
*       _DrawPNG_mem
*
* Function description
*   Draws a PNG file from memory
*/
static void _DrawPNG_mem(const void * pFileData, int FileSize) {
  int xSize;
  int ySize;
  int w;
  int h;
  int xPos;
  int yPos;

  w     = GUI_PNG_GetXSize(pFileData, FileSize);
  h     = GUI_PNG_GetYSize(pFileData, FileSize);
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  xPos  = (xSize - w) / 2 - 10;
  yPos  = (ySize - h) / 2 - 10;
  GUI_PNG_Draw(pFileData, FileSize, xPos, yPos);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_PNGs
*/
void GUIDEMO_PNGs(void) {
  GUIDEMO_ShowIntro("PNGs",   "" );
  //
  // Check if recommended memory for the sample is available
  //
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  _DrawStripes();
  _DrawPNG_file("..\\..\\Media\\Toucan_162x150.png"); /* Please change the link to a valid PNG file */
  GUIDEMO_Delay(1000);
  _DrawStripes();
  _DrawPNG_file("..\\..\\Media\\Printer_200x200_ARGB.png"); /* Please change the link to a valid PNG file */
  GUIDEMO_Delay(1000);
  GUIDEMO_Wait();
}

#endif

/*************************** End of file ****************************/
