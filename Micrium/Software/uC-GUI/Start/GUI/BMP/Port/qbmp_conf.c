/**
  ******************************************************************************
  * @file    qbmp_conf.c
  * @author  MCD Application Team
  * @brief   this file contains the libJPEG abstruction defines for the memory
  *          management routines and IO File operations
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "qbmp_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

size_t bread_file(uint8_t *buf, size_t size, uint32_t sizeofbuf, BFILE  *file)
{
  static size_t BytesReadfile ;  
  BOOL State;

  (void)size;
  State = ReadFile((HANDLE) file, buf , sizeofbuf, &BytesReadfile, NULL);
  if (State == FALSE && GetLastError() != NO_ERROR)
  {
      return -1;
  }
  return BytesReadfile; 
}

size_t bwrite_file(uint8_t *buf, size_t size, uint32_t sizeofbuf, BFILE  *file)
{
  static size_t BytesWritefile ;  
  BOOL State;

  (void)size;
  State = WriteFile((HANDLE) file, buf , sizeofbuf, &BytesWritefile, NULL);
  if (State == FALSE && GetLastError() != NO_ERROR)
  {
      return -1;
  }
  return BytesWritefile; 
}

int bclose_file(BFILE *file)
{
  CloseHandle((HANDLE)file);
  return 0;
}

BFILE *bopenr_file(const uint8_t *file, uint8_t *mode)
{
  HANDLE hFile;
  (void)mode;

  hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
    hFile = 0;
  }
  return hFile;
}

BFILE *bopenw_file(const uint8_t *file, uint8_t *mode)
{
  HANDLE hFile;
  (void)mode;

  hFile = CreateFile(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
    hFile = 0;
  }
  return hFile;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
