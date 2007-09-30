/*-
 * Copyright 2007 Jason Harmening
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef __TUNER_CONFIG_H__
#define __TUNER_CONFIG_H__

#ifdef _DIAGNOSTIC
#define DIAGNOSTIC(stmt) stmt;
#else
#define DIAGNOSTIC(stmt)
#endif

#define LIBTUNERERR (cerr << "[libtuner] ")
#define LIBTUNERLOG (clog << "[libtuner] ")

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
using namespace std;

class tuner_config
{
   public:

      tuner_config(void) {}

      tuner_config(const char *filename, int &error)
      {
         if (!error)
         {
            error = load_file(filename);
         }
      }

      virtual ~tuner_config(void) {}

      int load_file(const char *filename);

      template <typename numtype> numtype get_number(const char *key);

      const char *get_string(const char *key);

   private:

      typedef map<string, string> strmap;
      strmap entries;

};

#endif
