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

#include <sys/errno.h>
#include <fstream>
#include <sstream>

using namespace std;

#include "tuner_config.h"

#define WHITESPACE " \t"
#define DELIMS WHITESPACE"="

int tuner_config::load(istream &stream)
{
   int error = 0;
   try
   {
      string line;
      int lineno = 0;
      while (!stream.eof())
      {
         getline(stream, line);
         ++lineno;
         string::size_type token_begin, token_end;
         token_begin = line.find_first_not_of(WHITESPACE);
         if (token_begin == string::npos)
         {
            continue;
         }
         if (line[token_begin] == '#')
         {
            continue;
         }
         token_end = line.find_first_of(DELIMS, token_begin);
         if (token_end == string::npos)
         {
            LIBTUNERERR << "line " << lineno << ": Warning: skipped identifier without value" << endl;
            continue;
         }
         string ident = line.substr(token_begin, token_end - token_begin);
         transform(ident.begin(), ident.end(), ident.begin(), (int(*)(int))std::tolower);
         token_begin = line.find_first_not_of(DELIMS, token_end);
         if (token_begin == string::npos)
         {
            LIBTUNERERR << "line " << lineno << ": Warning: skipped identifier without value" << endl;
            continue;
         }
         token_end = line.find_last_not_of(WHITESPACE) + 1;
         string value = line.substr(token_begin, token_end);
         entries.insert(pair<string, string> (ident, value));
      }
   }
   catch(...)
   {
      error = ENOMEM;
   }
   return error;
}
               
int tuner_config::load_file(const char *filename)
{
   ifstream file(filename);
   if (!file.is_open())
   {
      return ENOENT;
   }
   int error = load(file);
   file.close();
   return error;
}

int tuner_config::load_string(const char *str)
{
   string s(str);
   istringstream strstream(s);
   return load(strstream);
}

const char *tuner_config::get_string(const char *key)
{
   try
   {
      string strkey(key);
      transform(strkey.begin(), strkey.end(), strkey.begin(), (int(*)(int))std::tolower);
      strmap::iterator it = entries.find(strkey);
      if (it == entries.end())
      {
         return NULL;
      }
      return it->second.c_str();
   }
   catch(...)
   {
      return NULL;
   }
}

template <typename numtype> 
numtype tuner_config::get_number(const char *key)
{
   try
   {
      string strkey(key);
      transform(strkey.begin(), strkey.end(), strkey.begin(), (int(*)(int))std::tolower);
      strmap::iterator it = entries.find(strkey);
      if (it == entries.end())
      {
         return ((numtype)0);
      }
      string value = it->second;
      stringstream stream(value);
      numtype val;
      stream >> val;
      return val;
   }
   catch(...)
   {
      return ((numtype)0);
   }
}
