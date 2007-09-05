#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "tuner_firmware.h"

tuner_firmware::tuner_firmware(const char *filename, int &error)
   : m_buffer(NULL),
     m_length(0),
     m_stream(NULL),
     m_uptodate(false),
     m_statfile(NULL),
     m_modtime(0)
{
   if (error)
   {
      return;
   }
   if ((m_stream = fopen(filename, "r")) == NULL)
   {
      error = ENOENT;
      return;
   }
   error = fseek(m_stream, 0, SEEK_END);
   m_length = ftell(m_stream);
   m_buffer = mmap(NULL, m_length, PROT_READ, MAP_PRIVATE, fileno(m_stream), 0);
   if (m_buffer == MAP_FAILED)
   {
      m_buffer = NULL;
      error = ENOMEM;
      return;
   }
   size_t len = strlen(filename);
   m_statfile = new char[len + 6];
   if (m_statfile == NULL)
   {
      error = ENOMEM;
      return;
   }
   strncpy(m_statfile, filename, len);
   strncpy(m_statfile + len, ".stat", 5);
   m_statfile[len + 5] = '\0';
   struct stat fwstat;
   if ((error = fstat(fileno(m_stream), &fwstat)))
   {
      return;
   }
   m_modtime = fwstat.st_mtime;
   time_t last_update;
   ifstream statstream(m_statfile);
   if (statstream.is_open())
   {
      statstream >> last_update;
      statstream.close();
      if (m_modtime <= last_update)
      {
         m_uptodate = true;
      }
   }
}

tuner_firmware::~tuner_firmware(void)
{
   if (m_buffer != NULL)
   {
      munmap(m_buffer, m_length);
      m_buffer = NULL;
   }
   if (m_stream != NULL)
   {
      fclose(m_stream);
      m_stream = NULL;
   }
   if (m_statfile != NULL)
   {
      delete[] m_statfile;
      m_statfile = NULL;
   }
}

void tuner_firmware::update(void)
{
   m_uptodate = true;
   if (m_statfile != NULL)
   {
      ofstream statstream(m_statfile);
      if (statstream.is_open())
      {
         statstream << m_modtime; 
         statstream.close(); 
      }
   }
}
