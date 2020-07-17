// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MESSAGEQUEUE
#define _HPP_MESSAGEQUEUE

#include "defines.hpp"

#include "base/properties.hpp"

#include "types/command.hpp"

#include "managers/environmentmanager.hpp"

namespace blunted {

  template <typename T = boost::intrusive_ptr<Command> >
  class MessageQueue {

    public:
      MessageQueue() {}
      virtual ~MessageQueue() {}

      inline void PushMessage(T message, bool notify = true) {
        boost::mutex::scoped_lock lock(queue.mutex);
        queue.data.push_back(message);
        lock.unlock();
        if (notify) NotifyWaiting(e_NotificationSubject_One);
      }

      inline void NotifyWaiting(e_NotificationSubject notificationSubject = e_NotificationSubject_All) {
        boost::mutex::scoped_lock lock(queue.mutex);
        if (notificationSubject == e_NotificationSubject_One) messageNotification.notify_one();
        if (notificationSubject == e_NotificationSubject_All) messageNotification.notify_all();
      }

      inline T GetMessage(bool &MsgAvail) {
        T message;
        boost::mutex::scoped_lock lock(queue.mutex);
        if (queue.data.size() > 0) {
          message = *queue.data.begin();
          queue.data.pop_front();
          MsgAvail = true;
        } else {
          MsgAvail = false;
        }
        return message;
      }

      inline void Purge() {
        boost::mutex::scoped_lock lock(queue.mutex);
        queue.data.clear();
      }

      inline T WaitForMessage(int timeout_ms = -1) {
        bool dud;
        return WaitForMessage(dud, timeout_ms);
      }

      inline T WaitForMessage(bool &isMessage, int timeout_ms = -1) {

        isMessage = false;
        T message;

        boost::mutex::scoped_lock lock(queue.mutex);
        while (!isMessage) {
          // cannot use queue.WaitForNotification, because its scoped lock unlocks on return
          if (queue.data.size() == 0) {

            if (timeout_ms == -1) {
              messageNotification.wait(lock);
            } else {
              boost::system_time tAbsoluteTime = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
              bool received = messageNotification.timed_wait(lock, tAbsoluteTime);
              if (!received) return message; // timeout
            }

          }
          if (queue.data.size() > 0) {
            message = *queue.data.begin();
            queue.data.pop_front();
            isMessage = true;
          }
        }
        return message;
      }

      inline int GetPending() {
        queue.Lock();
        int size = queue.data.size();
        queue.Unlock();
        return size;
      }

    protected:
      Lockable < std::list < T > > queue;
      boost::condition messageNotification;

  };

}

#endif
