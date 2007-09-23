#include <sys/errno.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include "tuner_config.h"

#define WHITESPACE " \t"
#define DELIMS WHITESPACE"="

int tuner_config::load_file(const char *filename)
{
   ifstream file(filename);
   if (!file.is_open())
   {
      return ENOENT;
   }
   int error = 0;
   try
   {
      string line;
      int lineno = 0;
      while (!file.eof())
      {
         getline(file, line);
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
            cout << filename << '[' << lineno << "]: Warning: skipped identifier without value" << endl;
            continue;
         }
         string ident = line.substr(token_begin, token_end - token_begin);
         transform(ident.begin(), ident.end(), ident.begin(), (int(*)(int))std::tolower);
         token_begin = line.find_first_not_of(DELIMS, token_end);
         if (token_begin == string::npos)
         {
            cout << filename << '[' << lineno << "]: Warning: skipped identifier without value" << endl;
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
   file.close();
   return error;
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
