// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_RESOURCES_SOUNDBUFFER
#define _HPP_RESOURCES_SOUNDBUFFER

#include "types/resource.hpp"

namespace blunted {

  struct WavData {
    WavData() {
      data = 0;
      size = 0;
    }
    ~WavData() {
      delete [] data;
      size = 0;
    }
    WavData(const WavData &src) {
      data = new unsigned char[src.size];
      memcpy(data, src.data, src.size * sizeof(unsigned char));
      size = src.size;
      channels = src.channels;
      bits = src.bits;
      frequency = src.frequency;
    }
    unsigned char *data;
    int size;
    int channels, bits;
    unsigned int frequency;
  };

  class SoundBuffer {

    public:
      SoundBuffer();
      virtual ~SoundBuffer();
      SoundBuffer(const SoundBuffer &src);

      const WavData *GetData() const;
      void SetData(WavData *data);

    protected:
      WavData *data;

  };

}

#endif
