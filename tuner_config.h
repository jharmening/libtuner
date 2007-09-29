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
