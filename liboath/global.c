/*
 * global.c - implementation of library global functions
 * Copyright (C) 2009, 2010 Simon Josefsson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <config.h>

#include "oath.h"

#include <stdio.h>		/* For snprintf, getline. */
#include <string.h>		/* For strverscmp. */

#include "gc.h"

/**
 * oath_init - initialize the OATH library
 *
 * This function initializes the OATH library.  Every user of this
 * library needs to call this function before using other functions.
 * You should call oath_done() when use of the OATH library is no
 * longer needed.
 *
 * Note that this function may also initialize Libgcrypt, if the OATH
 * library is built with libgcrypt support and libgcrypt has not been
 * initialized before.  Thus if you want to manually initialize
 * libgcrypt you must do it before calling this function.  This is
 * useful in cases you want to disable libgcrypt's internal lockings
 * etc.
 *
 * Returns: On success, %OATH_OK (zero) is returned, otherwise an
 *   error code is returned.
 **/
int
oath_init (void)
{
  if (gc_init () != GC_OK)
    return OATH_CRYPTO_ERROR;

  return OATH_OK;
}

/**
 * oath_done - deinitialize the OATH library
 *
 * This function deinitializes the OATH library, which were
 * initialized using oath_init().  After calling this function, no
 * other OATH library function may be called except for to
 * re-initialize the library using oath_init().
 *
 * Returns: On success, %OATH_OK (zero) is returned, otherwise an
 *   error code is returned.
 **/
int
oath_done (void)
{
  gc_done ();

  return OATH_OK;
}

/**
 * oath_check_version:
 * @req_version: version string to compare with, or %NULL.
 *
 * Check OATH library version.
 *
 * See %OATH_VERSION for a suitable @req_version string.
 *
 * This function is one of few in the library that can be used without
 * a successful call to oath_init().
 *
 * Return value: Check that the version of the library is at
 *   minimum the one given as a string in @req_version and return the
 *   actual version string of the library; return %NULL if the
 *   condition is not met.  If %NULL is passed to this function no
 *   check is done and only the version string is returned.
 **/
const char *
oath_check_version (const char *req_version)
{
  if (!req_version || strverscmp (req_version, OATH_VERSION) <= 0)
    return OATH_VERSION;

  return NULL;
}

#define maybe_lowercase(c)			\
  ((c) == 'A' ? 'a' :				\
   ((c) == 'B' ? 'b' :				\
    ((c) == 'C' ? 'c' :				\
     ((c) == 'D' ? 'd' :			\
      ((c) == 'E' ? 'e' :			\
       ((c) == 'F' ? 'f' :			\
	c))))))

/**
 * oath_hex2bin:
 * @hexstr: input string with hex data
 * @binstr: output string that holds binary data, or %NULL
 * @binlen: output variable holding needed length of @binstr
 *
 * Convert string with hex data to binary data.
 *
 * Non-hexadecimal data are not ignored but instead will lead to an
 * %OATH_INVALID_HEX error.
 *
 * If @binstr is %NULL, then @binlen will be populated with the
 * necessary length.  If the @binstr buffer is too small,
 * %OATH_TOO_SMALL_BUFFER is returned and @binlen will contain the
 * necessary length.
 *
 * Returns: On success, %OATH_OK (zero) is returned, otherwise an
 *   error code is returned.
 **/
int
oath_hex2bin (char *hexstr, char *binstr, size_t * binlen)
{
  const char *hexalphabet = "0123456789abcdef";
  bool highbits = true;
  size_t save_binlen = *binlen;
  bool too_small = false;

  *binlen = 0;

  while (*hexstr)
    {
      char *p;

      p = strchr (hexalphabet, maybe_lowercase (*hexstr));
      if (!p)
	return OATH_INVALID_HEX;

      if (binstr && save_binlen > 0)
	{
	  if (highbits)
	    *binstr = (*binstr & 0x0F) | ((p - hexalphabet) << 4);
	  else
	    *binstr = (*binstr & 0xF0) | (p - hexalphabet);
	}

      hexstr++;
      if (!highbits)
	{
	  binstr++, (*binlen)++;
	  if (save_binlen > 0)
	    save_binlen--;
	  else
	    too_small = true;
	}
      highbits = !highbits;
    }

  if (!highbits)
    return OATH_INVALID_HEX;

  if (too_small)
    return OATH_TOO_SMALL_BUFFER;

  return OATH_OK;
}
