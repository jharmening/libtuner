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

#define LIBTUNERERR libtuner_config::errfunc(libtuner_config::errstream)
#define LIBTUNERLOG libtuner_config::logfunc(libtuner_config::logstream)

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <list>
#include <sstream>

#define LIBTUNER_STORE_PATH_KEY "LIBTUNER_DATA_STORE"
#define LIBTUNER_DOMAIN_KEY "LIBTUNER_DOMAIN"

namespace libtuner_config
{
   typedef std::ostream& (*outfunc)(std::ostream*);

   extern std::ostream *logstream;
   extern std::ostream *errstream;
   extern outfunc logfunc;
   extern outfunc errfunc;

   void configure_log(std::ostream *stream, outfunc func);
   void configure_err(std::ostream *stream, outfunc func);

}

class tuner_config
{
   public:

      tuner_config(void) : m_next (NULL) {}

      virtual ~tuner_config(void) {}

      int load_file(const char *filename);

      int load_string(const char *str);

      int load_string(const char *str, char line_delim);
      
      int set_string(const char *key, const char *value);

      const char *get_string(const char *key);
            
      template <typename numtype> 
      numtype get_number(const char *key, numtype default_val)
      {
         const char *str = get_string(key);
         if (str != NULL)
         {
            std::string value(str);
            std::stringstream stream(value);
            numtype val;
            stream >> val;
            return val;
         }
         else
         {
            return default_val;
         }
      }
      
      template <typename numtype> 
      numtype get_number(const char *key)
      {
         return get_number<numtype>(key, (numtype)0);
      }
      
      std::string get_file(const char *filename);
      
      void put_file(const char *filename);
      
      int add_config(tuner_config &config);
      
      void remove_config(tuner_config &config);

   private:
      
      int load(std::istream &stream, char line_delim = '\n');
      
      const char *get_config_string(const char *key);

      void set_string(std::string &key, std::string &value);
        
      typedef std::map<std::string, std::string> strmap;
      
      std::string get_store_path(void);
      
      strmap m_map;
      
      tuner_config *m_next;

};

#endif
