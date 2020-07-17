// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "wavloader.hpp"

#include "base/log.hpp"

#include <fstream>

namespace blunted {

  WAVLoader::WAVLoader() : Loader<SoundBuffer>() {
  }

  WAVLoader::~WAVLoader() {
  }

  // load file into resource
  void WAVLoader::Load(std::string filename, boost::intrusive_ptr < Resource <SoundBuffer> > resource) {

    int channels, bits;
    unsigned int frequency;
    long BUFFER_SIZE = 1024 * 1024 * 4;

    FILE *f;
    f = fopen(filename.c_str(), "rb");
    if (!f) {
      Log(e_FatalError, "WAVLoader", "Load", "Could not load " + filename + ": file not found");
    }

    unsigned char *buf;
    buf = new unsigned char[BUFFER_SIZE];

    fread(buf, 1, 12, f); // ignore wave header
    fread(buf, 1, 8, f); // 4 identifier, 4 chunk size (ignore)
    if (buf[0] != 'f' || buf[1] != 'm' || buf[2] != 't' || buf[3] != ' ') {
      fclose(f);
      Log(e_FatalError, "WAVLoader", "Load", "Could not load " + filename + ": format information header incorrect");
    }

    fread(buf, 1, 2, f);

    if (buf[0] != 1 || buf[1] != 0) {
      fclose(f);
      Log(e_FatalError, "WAVLoader", "Load", "Could not load " + filename + ": not PCM");
    }

    fread(buf, 1, 2, f);
    channels  = buf[1] << 8;
    channels |= buf[0];

    fread(buf, 1, 4, f);
    frequency  = buf[3] << 24;
    frequency |= buf[2] << 16;
    frequency |= buf[1] << 8;
    frequency |= buf[0];

    fread(buf, 1, 6, f); // blocksize / bps (ignore)

    fread(buf, 1, 2, f);
    bits  = buf[1] << 8;
    bits |= buf[0];

    fread(buf, 1, 8, f); // 4 identifier, 4 chunk size (ignore)
    if (buf[0] != 'd' || buf[1] != 'a' || buf[2] != 't' || buf[3] != 'a') {
      fclose(f);
      Log(e_FatalError, "WAVLoader", "Load", "Could not load " + filename + ": data chunk not found");
    }

    int size = fread(buf, 1, BUFFER_SIZE, f);

    WavData *data = new WavData();
    data->data = new unsigned char[size];
    memcpy(data->data, buf, size * sizeof(unsigned char));
    data->size = size;
    data->channels = channels;
    data->bits = bits;
    data->frequency = frequency;

    //printf("wav data: %i bytes, %i channels, %i bits, %u freq\n", size * sizeof(unsigned char), channels, bits, frequency);

    resource->GetResource()->SetData(data);

    fclose(f);
    delete [] buf;
  }

}
