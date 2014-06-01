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
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include "tuner_config.h"

using namespace std;

#define WHITESPACE " \t"
#define DELIMS WHITESPACE"="

namespace libtuner_config
{
   static ostream& default_log(std::ostream *stream)
   {
      return ((stream == NULL) ? clog : *stream) << "[libtuner] ";
   }

   static ostream& default_err(std::ostream *stream)
   {
      return ((stream == NULL) ? cerr : *stream) << "[libtuner] ";
   }

   ostream *logstream = NULL;
   ostream *errstream = NULL;
   outfunc logfunc = default_log;
   outfunc errfunc = default_err;

   void configure_log(ostream *stream, outfunc func)
   {
      logstream = stream;
      logfunc = func;
   }

   void configure_err(ostream *stream, outfunc func)
   {
      errstream = stream;
      errfunc = func;
   }
}

int tuner_config::load(istream &stream, char line_delim)
{
   if (m_next != NULL)
   {
      return m_next->load(stream, line_delim);  
   }
   int error = 0;
   try
   {
      string line;
      int lineno = 0;
      while (!stream.eof())
      {
         getline(stream, line, line_delim);
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
         token_begin = line.find_first_not_of(DELIMS, token_end);
         if (token_begin == string::npos)
         {
            LIBTUNERERR << "line " << lineno << ": Warning: skipped identifier without value" << endl;
            continue;
         }
         token_end = line.find_last_not_of(WHITESPACE) + 1;
         string value = line.substr(token_begin, token_end);
         set_string(ident, value);
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
   return load_string(str, '\n');
}

int tuner_config::load_string(const char *str, char line_delim)
{
   string s(str);
   istringstream strstream(s);
   return load(strstream, line_delim);
}

int tuner_config::set_string(const char *key, const char *value)
{
   int error = 0;
   try
   {
      string keystr(key), valstr(value);
      set_string(keystr, valstr);
   }
   catch(...)
   {
      error = ENOMEM;
   }
   return error;
}

void tuner_config::set_string(string &key, string &value)
{
   transform(key.begin(), key.end(), key.begin(), (int(*)(int))std::tolower);
   m_map.erase(key);
   m_map.insert(pair<string, string> (key, value));
}

const char *tuner_config::get_string(const char *key)
{
   const char *str = getenv(key);
   if (str == NULL)
   {  
      str = get_config_string(key);
   }
   return str;
}

const char *tuner_config::get_config_string(const char *key)
{
   const char *str = NULL;
   if (m_next != NULL)
   {
      str = m_next->get_string(key);
   }
   if (str == NULL)
   {
      try
      {
         string strkey(key);
         transform(strkey.begin(), strkey.end(), strkey.begin(), (int(*)(int))std::tolower);
         strmap::iterator it = m_map.find(strkey);
         if (it != m_map.end())
         {
            return it->second.c_str();
         }
         return NULL;
      }
      catch(...)
      {
         return NULL;
      }
   }
   return str;
}

int tuner_config::add_config(tuner_config &config)
{
   if (&config == this)
   {
      return EINVAL;  
   }
   if (m_next != NULL)
   {
      return m_next->add_config(config);
   }
   m_next = &config;
   return 0;
}

void tuner_config::remove_config(tuner_config &config)     
{
   if (m_next != NULL)
   {
      if (m_next == &config)
      {
         m_next = m_next->m_next;    
      }
      else
      {
         m_next->remove_config(config);  
      }
   }
}

string tuner_config::get_store_path(void)
{
   string path;
   const char *dir = get_string(LIBTUNER_STORE_PATH_KEY);
   if (dir == NULL)
   {
      const char *homedir = getenv("HOME");
      if (homedir != NULL)
      {
         path = homedir;
      }
      path += "/.libtuner";
   }
   else
   {
      path = dir;
   }
   const char *domain = get_string(LIBTUNER_DOMAIN_KEY);
   if (domain != NULL)
   {
      path += "_";
      path += domain;
   }
   return path;  
}

string tuner_config::get_file(const char *filename)
{
   string path;
   try
   {
      path = get_store_path();
      int error = mkdir(path.c_str(), 0770);
      if (error && (errno != EEXIST))
      {
         LIBTUNERERR << "Unable to create data store at " << path.c_str() << ": " << strerror(errno) << endl;
      }
      path += "/";
      path += filename;
   }
   catch (...)
   {
      LIBTUNERERR << "Exception when generating data store path for " << filename << endl;
      return path;
   }
   return path;
}

void tuner_config::put_file(const char *filename)
{
   try
   {
      string path = get_store_path();
      string full_path = path + "/" + filename;
      remove(full_path.c_str());
      rmdir(path.c_str());
   }
   catch (...)
   {
      LIBTUNERERR << "Exception when generating data store path for " << filename << endl;
   }
}
