/* Copyright (c) 2000, 2002, 2003, 2007 MySQL AB
   Use is subject to license terms

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335  USA */

#include "mysys_priv.h"
#include <stdarg.h>

#ifndef DBUG_OFF
/* Put a protected barrier after every element when using my_multi_malloc() */
#define ALLOC_BARRIER
#endif

/*
  Malloc many pointers at the same time
  Only ptr1 can be free'd, and doing this will free all
  the memory allocated. ptr2, etc all point inside big allocated
  memory area.

  SYNOPSIS
    my_multi_malloc()
      myFlags              Flags
	ptr1, length1      Multiple arguments terminated by null ptr
	ptr2, length2      ...
        ...
	NULL
*/

void* my_multi_malloc(PSI_memory_key key, myf myFlags, ...)
{
  va_list args;
  char **ptr,*start,*res;
  size_t tot_length,length;
  DBUG_ENTER("my_multi_malloc");

  va_start(args,myFlags);
  tot_length=0;
  while ((ptr=va_arg(args, char **)))
  {
    length=va_arg(args,uint);
    tot_length+=ALIGN_SIZE(length);
#ifdef ALLOC_BARRIER
    tot_length+= ALIGN_SIZE(1);
#endif
  }
  va_end(args);

  if (!(start=(char *) my_malloc(key, tot_length,myFlags)))
    DBUG_RETURN(0); /* purecov: inspected */

  va_start(args,myFlags);
  res=start;
  while ((ptr=va_arg(args, char **)))
  {
    *ptr=res;
    length=va_arg(args,uint);
    res+=ALIGN_SIZE(length);
#ifdef ALLOC_BARRIER
    TRASH_FREE(res, ALIGN_SIZE(1));
    res+= ALIGN_SIZE(1);
#endif
  }
  va_end(args);
  DBUG_RETURN((void*) start);
}


/*
  Same as my_multi_malloc, but each entry can be over 4G

  SYNOPSIS
    my_multi_malloc()
      myFlags              Flags
	ptr1, length1      Multiple arguments terminated by null ptr
	ptr2, length2      ...
        ...
	NULL
*/

void *my_multi_malloc_large(PSI_memory_key key, myf myFlags, ...)
{
  va_list args;
  char **ptr,*start,*res;
  ulonglong tot_length,length;
  DBUG_ENTER("my_multi_malloc");

  va_start(args,myFlags);
  tot_length=0;
  while ((ptr=va_arg(args, char **)))
  {
    length=va_arg(args,ulonglong);
    tot_length+=ALIGN_SIZE(length);
#ifdef ALLOC_BARRIER
    tot_length+= ALIGN_SIZE(1);
#endif
  }
  va_end(args);

  if (!(start=(char *) my_malloc(key, (size_t) tot_length, myFlags)))
    DBUG_RETURN(0); /* purecov: inspected */

  va_start(args,myFlags);
  res=start;
  while ((ptr=va_arg(args, char **)))
  {
    *ptr=res;
    length=va_arg(args,ulonglong);
    res+=ALIGN_SIZE(length);
#ifdef ALLOC_BARRIER
    TRASH_FREE(res, ALIGN_SIZE(1));
    res+= ALIGN_SIZE(1);
#endif
  }
  va_end(args);
  DBUG_RETURN((void*) start);
}
