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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string>
using namespace std;

#include "tuner_firmware.h"

tuner_firmware::tuner_firmware(tuner_config &conf, const char *filename, int &error)
   : m_buffer(NULL),
     m_length(0),
     m_stream(NULL),
     m_uptodate(false),
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
   const char *filepart = strrchr(filename, '/');
   if (filepart == NULL)
   {
      filepart = filename;
   }
   else
   {
      ++filepart;
   }
   try
   {
      string statfile = filepart;
      statfile += ".stat";
      m_statfile = conf.get_file(statfile.c_str());
   }
   catch (...)
   {
      LIBTUNERERR << "Exception when generating firmware stat file for " << filename << endl;
      error = ENOMEM;
      return;
   }
   struct stat fwstat;
   if ((error = fstat(fileno(m_stream), &fwstat)))
   {
      return;
   }
   m_modtime = fwstat.st_mtime;
   time_t last_update;
   FILE *statstream = fopen(m_statfile.c_str(), "r");
   if (statstream != NULL)
   {
      flock(fileno(statstream), LOCK_EX);
      fscanf(statstream, "%ld", &last_update);
      flock(fileno(statstream), LOCK_UN);
      fclose(statstream);
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
}

void tuner_firmware::update(void)
{
   if (!m_uptodate)
   {
      m_uptodate = true;
      FILE *statstream = fopen(m_statfile.c_str(), "w+");
      if (statstream != NULL)
      {
         flock(fileno(statstream), LOCK_EX);
         fprintf(statstream, "%ld", m_modtime);
         fflush(statstream);
         flock(fileno(statstream), LOCK_UN);
         fclose(statstream); 
      }
   }
}

