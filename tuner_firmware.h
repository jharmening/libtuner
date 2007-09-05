#ifndef __TUNER_FIRMWARE_H__
#define __TUNER_FIRMWARE_H__

#include <stdio.h>
#include <sys/types.h>

class tuner_firmware
{

   public:

      tuner_firmware(const char *filename, int &error);

      virtual ~tuner_firmware(void);

      virtual bool up_to_date(void)
      {
         return m_uptodate;
      }
      
      virtual void update(void);
      
      virtual void *buffer(void)
      {
         return m_buffer;
      }

      virtual size_t length(void)
      {
         return m_length;
      }

   private:

      void *m_buffer;
      size_t m_length;
      FILE *m_stream;
      bool m_uptodate;
      char *m_statfile;
      time_t m_modtime;
};

#endif
