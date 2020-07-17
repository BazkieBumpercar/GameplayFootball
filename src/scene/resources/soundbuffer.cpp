// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "soundbuffer.hpp"

namespace blunted {

  SoundBuffer::SoundBuffer() : data(0) {
  }

  SoundBuffer::~SoundBuffer() {
    if (data) {
      delete data;
    }
    data = 0;
  }

  SoundBuffer::SoundBuffer(const SoundBuffer &src) {
    this->data = new WavData(*src.GetData());
  }

  const WavData *SoundBuffer::GetData() const {
    return data;
  }

  void SoundBuffer::SetData(WavData *data) {
    if (this->data) delete this->data;
    this->data = data;
  }

}
