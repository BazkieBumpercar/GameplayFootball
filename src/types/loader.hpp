// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_LOADER
#define _HPP_LOADER

namespace blunted {

  // loads specific kind of resource

  template < typename T >
  class Resource;

  template < typename T >
  class Loader {

    public:
      virtual void Load(std::string filename, boost::intrusive_ptr < Resource<T> > resource) = 0;

    protected:

  };

}

#endif
