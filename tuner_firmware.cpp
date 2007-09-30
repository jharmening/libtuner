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
