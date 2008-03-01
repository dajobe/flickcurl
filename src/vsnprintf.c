/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * vsnprintf.c - vsnprintf implementation
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
 * 
 * This file is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 */


#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif

/* 
 * Thanks to the patch in this Debian bug for the solution
 * to the crash inside vsnprintf on some architectures.
 *
 * "reuse of args inside the while(1) loop is in violation of the
 * specs and only happens to work by accident on other systems."
 *
 * http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=104325 
 */

#ifndef va_copy
#ifdef __va_copy
#define va_copy(dest,src) __va_copy(dest,src)
#else
#define va_copy(dest,src) (dest) = (src)
#endif
#endif

char* my_vsnprintf(const char *message, va_list arguments); 


char*
my_vsnprintf(const char *message, va_list arguments) 
{
#ifdef HAVE_C99_VSNPRINTF
  char empty_buffer[1];
#endif
  int len;
  char *buffer=NULL;
  va_list args_copy;

#ifdef HAVE_C99_VSNPRINTF
  /* copy for re-use */
  va_copy(args_copy, arguments);
  len=vsnprintf(empty_buffer, 1, message, args_copy)+1;
  va_end(args_copy);

  if(len<=0)
    return NULL;
  
  buffer=(char*)malloc(len);
  if(buffer) {
    /* copy for re-use */
    va_copy(args_copy, arguments);
    vsnprintf(buffer, len, message, args_copy);
    va_end(args_copy);
  }
#else
  /* This vsnprintf doesn't return number of bytes required */
  int size=2;
      
  while(1) {
    buffer=(char*)malloc(size+1);
    if(!buffer)
      break;
    
    /* copy for re-use */
    va_copy(args_copy, arguments);
    len=vsnprintf(buffer, size, message, args_copy);
    va_end(args_copy);

    if(len>=0)
      break;
    free(buffer);
    size+=4;
  }
#endif

  return buffer;
}
