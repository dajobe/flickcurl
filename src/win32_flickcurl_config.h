/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * win32_flickcurl_config.h - Flickcurl WIN32 hard-coded config
 *
 * Copyright (C) 2008-2012, David Beckett http://www.dajobe.org/
 * 
 * It is licensed under the following three licenses as alternatives:
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
 * 
 */


#ifndef WIN32_CONFIG_H
#define WIN32_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <io.h>
#include <memory.h>


/* MS names for these functions */
/* This check is not strictly correct, we want to check against SDK used */
#if _MSC_VER >= 1900
#	define HAVE_C99_VSNPRINTF 1
#	define HAVE_STRUCT_TIMESPEC 1
#else
#	define vsnprintf _vsnprintf
#endif

#define access _access

/* define missing flag for access() - the only one used here */
#ifndef R_OK
#define R_OK 4
#endif


/* 
 * All defines from config.h should be added here with appropriate values
 */

#undef HAVE_NANOSLEEP

/* not quite true - but it's called something else and the define above
 * handles it
 */
#define HAVE_VSNPRINTF 1

/* FIXME - rest of win32 config defines should go here */
#define HAVE_STDINT_H 1
#undef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval* tp, void* tzp);

/* Version number of package */
#define VERSION "1.27"

#ifdef __cplusplus
}
#endif

#endif
