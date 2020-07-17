// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "scheduler.hpp"

#include "managers/resourcemanagerpool.hpp"
#include "managers/environmentmanager.hpp"
#include "managers/taskmanager.hpp"
#include "base/log.hpp"

namespace blunted {

  Scheduler::Scheduler(TaskManager *taskManager) : taskManager(taskManager) {
    cleanUpTimeOffset = 0;
    previousTime_ms = 0;
  }

  Scheduler::~Scheduler() {
  }

  void Scheduler::Exit() {

    if (GetSequenceCount() != 0) { // shouldn't happen
      sequences.Lock();
      for (unsigned int i = 0; i < sequences.data.size(); i++) {
        printf("sequence '%s' is stuck on entry #%i!\n", sequences.data.at(i)->taskSequence->GetName().c_str(), sequences.data.at(i)->programCounter);
      }
      sequences.Unlock();
    }
    assert(GetSequenceCount() == 0);
  }

  int Scheduler::GetSequenceCount() {
    sequences.Lock();
    int size = sequences.data.size();
    sequences.Unlock();
    return size;
  }

  void Scheduler::RegisterTaskSequence(boost::shared_ptr<TaskSequence> sequence) {
    if (sequence->GetEntryCount() == 0) Log(e_FatalError, "Scheduler", "RegisterTaskSequence", "Trying to add a sequence without entries");
    sequence->AddTerminator();
    boost::shared_ptr<TaskSequenceProgram> program(new TaskSequenceProgram());
    unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
    program->taskSequence = sequence;
    program->programCounter = 0;
    program->previousProgramCounter = -1;
    program->sequenceStartTime = time_ms;
    program->lastSequenceTime = 0;
    program->startTime = time_ms;
    program->timesRan = 0;
    program->paused = false;
    program->dueQuit = false;
    program->readyToQuit = false;
    sequences.Lock();
    sequences.data.push_back(program);
    sequences.Unlock();
  }

  void Scheduler::UnregisterTaskSequence(boost::shared_ptr<TaskSequence> sequence) {
    // todo
  }

  void Scheduler::UnregisterTaskSequence(const std::string &name) {
    sequences.Lock();
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {
        program->dueQuit = true;
        break;
      }
    }
    sequences.Unlock();
  }

  void Scheduler::PauseTaskSequence(const std::string &name) {
    sequences.Lock();
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {
        program->paused = true;
        break;
      }
    }
    sequences.Unlock();
  }

  void Scheduler::UnpauseTaskSequence(const std::string &name) {
    sequences.Lock();
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {
        program->paused = false;
        break;
      }
    }
    sequences.Unlock();
  }

  void Scheduler::ResetTaskSequenceTime(const std::string &name) {
    sequences.Lock();
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {
        program->startTime = EnvironmentManager::GetInstance().GetTime_ms();// - startTime_ms;
        program->timesRan = 0;
        break;
      }
    }
    sequences.Unlock();
  }

  unsigned long Scheduler::GetTaskSequenceTime_ms(const std::string &name) {
    unsigned int resultTime_ms = 0;
    sequences.Lock();
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {

        if (program->taskSequence->GetSkippable()) {
          resultTime_ms = program->sequenceStartTime + program->taskSequence->GetSequenceTime();
        } else {
          resultTime_ms = program->startTime + program->taskSequence->GetSequenceTime() * program->timesRan;
        }

        break;
      }
    }
    sequences.Unlock();
    return resultTime_ms;
  }

  TaskSequenceInfo Scheduler::GetTaskSequenceInfo(const std::string &name) {
    TaskSequenceInfo info;
    sequences.Lock(); // todo: cache this to overcome threading traffic slowdowns?
    for (unsigned int i = 0; i < sequences.data.size(); i++) {
      boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(i);
      if (program->taskSequence->GetName() == name) {

        info.sequenceStartTime_ms = program->sequenceStartTime;
        info.lastSequenceTime_ms = program->lastSequenceTime;
        info.startTime_ms = program->startTime;
        info.sequenceTime_ms = program->taskSequence->GetSequenceTime();
        info.timesRan = program->timesRan;

        break;
      }
    }
    sequences.Unlock();
    return info;
  }

  bool Scheduler::Run() {

    bool verbose = false;

    // sequenced version, the best yet!
    // thought up by Jurian Broertjes & Bastiaan Konings Schuiling

    boost::mutex::scoped_lock lock(somethingIsDoneMutex);

    unsigned int firstSequence = 0;
    int quiterations = 0;
    bool sequencesQuitMessageDone = false;

    while (EnvironmentManager::GetInstance().GetQuit() == false || GetSequenceCount() > 0) {

      if (EnvironmentManager::GetInstance().GetQuit()) {

        // let sequences finish
        if (!sequencesQuitMessageDone) {
          sequences.Lock();
          for (unsigned int i = 0; i < sequences.data.size(); i++) {
            sequences.data.at(i)->dueQuit = true;
          }
          sequences.Unlock();
          sequencesQuitMessageDone = true;
        }

        // check if we're stuck
        if (sequencesQuitMessageDone && GetSequenceCount() > 0) {
          quiterations++;
          if (quiterations > 1000) {
            // something won't shut up!
            sequences.Lock();
            for (unsigned int i = 0; i < sequences.data.size(); i++) {
              printf("sequence '%s' is stuck on entry #%i!\n", sequences.data.at(i)->taskSequence->GetName().c_str(), sequences.data.at(i)->programCounter);
            }
            sequences.Unlock();
            quiterations = 0;
          }
        }
      }

      unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();
      unsigned long timeDiff_ms = time_ms - previousTime_ms;
      previousTime_ms = time_ms;

      cleanUpTimeOffset += timeDiff_ms;


      // find first sequence entry that needs to be started

      TaskSequenceQueueEntry dueEntry;

      sequences.Lock();

      bool someSequenceNeedsDeleting = false;

      for (unsigned int i = 0; i < sequences.data.size(); i++) {
        int programIndex = (i + firstSequence) % sequences.data.size();
        boost::shared_ptr<TaskSequenceProgram> program = sequences.data.at(programIndex);

        if (verbose) printf("sequence %i, previous program counter %i, program counter %i\n", i, program->previousProgramCounter, program->programCounter);

        // check if previous entry is ready
        bool previousEntryIsReady = true;
        if (program->previousProgramCounter != -1) {

          previousEntryIsReady = program->taskSequence->GetEntry(program->previousProgramCounter)->IsReady();

          if (previousEntryIsReady && program->programCounter == program->taskSequence->GetEntryCount()) {
            program->programCounter = 0;
            program->timesRan++;
            program->lastSequenceTime = time_ms - program->sequenceStartTime;
          }

        }

        if (verbose) {
          if (previousEntryIsReady) {
            printf("sequence %i counter %i is ready\n", programIndex, program->previousProgramCounter);
          } else {
            printf("sequence %i counter %i is not ready\n", programIndex, program->previousProgramCounter);
          }
        }

        if (previousEntryIsReady) {

          long timeUntilDueEntry_ms = 0; // if programCounter != 0, we just want to start the next entry ASAP

          if (program->programCounter == 0) { // else, (re)starting sequence; find out when it's due

            if (program->dueQuit == true) {

              program->readyToQuit = true;
              someSequenceNeedsDeleting = true;

            } else if (program->paused == true) {

              if (!program->taskSequence->GetSkippable()) program->startTime += timeDiff_ms;

            } else { // not quitting or paused

              if (program->taskSequence->GetSkippable()) {
                // use relative time: don't mind if last frame lasted too long
                timeUntilDueEntry_ms = (program->sequenceStartTime + program->taskSequence->GetSequenceTime()) - time_ms;
                //printf("wiieee: %i .. %li .. %li\n", program->taskSequence->GetFrameTime(), time_ms, startTimeRel_ms);
              } else {
                // use absolute time: if not enough iterations have been done to get to frametime * timesran, start immediately
                timeUntilDueEntry_ms = (program->startTime + program->taskSequence->GetSequenceTime() * program->timesRan) - time_ms;
                //printf("wiieee: %i .. %li .. %li\n", program->taskSequence->GetSequenceTime(), time_ms, timeUntilDueEntry_ms);
              }

            }

          }

          if (!program->readyToQuit && !program->paused && (timeUntilDueEntry_ms < dueEntry.timeUntilDueEntry_ms || dueEntry.program == boost::shared_ptr<TaskSequenceProgram>())) {
            dueEntry.program = program;
            dueEntry.timeUntilDueEntry_ms = timeUntilDueEntry_ms;
          }

        }

      } // checked all sequences and (hopefully) found an entry that is due next


      // delete sequences that are ready to pussy out

      if (someSequenceNeedsDeleting) {

        std::vector < boost::shared_ptr<TaskSequenceProgram> >::iterator quiterator = sequences.data.begin();
        while (quiterator != sequences.data.end()) {
          boost::shared_ptr<TaskSequenceProgram> program = *quiterator;
          if (program->readyToQuit == true) {
            quiterator = sequences.data.erase(quiterator);
          } else {
            quiterator++;
          }
        }

        // sequences.Unlock();
        // continue;

      } else { // (no deletes)

        // switch first sequence to handle next time (so they all get a turn)
        // todo: this way, longer sequences get relatively less time in total. is this desirable?
        firstSequence++;
        if (firstSequence >= sequences.data.size()) firstSequence = 0;

        long timeout_ms = 0;
        if (dueEntry.program != boost::shared_ptr<TaskSequenceProgram>()) {
          // wait until time for due sequence entry (even if that is <= 0, because we need to unlock the condition and sequences locks anyway)
          timeout_ms = dueEntry.timeUntilDueEntry_ms;
          timeout_ms = std::max(timeout_ms, (long)0);
        } else {
          timeout_ms = 100; // wait for wakeup signal
        }

        sequences.Unlock();
        boost::system_time tAbsoluteTime = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
        bool isMessage = somethingIsDone.timed_wait(lock, tAbsoluteTime);
        sequences.Lock();

        if (dueEntry.program != boost::shared_ptr<TaskSequenceProgram>()) {

          if (verbose) printf("time until due entry: %li\n", dueEntry.timeUntilDueEntry_ms);

          if (dueEntry.timeUntilDueEntry_ms <= 0) { // the first and/or other entries are due

            // run!
            if (dueEntry.program->programCounter == 0) {
              dueEntry.program->sequenceStartTime = time_ms;
            }

            if (verbose) printf("executing program counter %i\n", dueEntry.program->programCounter);
            dueEntry.program->taskSequence->GetEntry(dueEntry.program->programCounter)->Reset();
            dueEntry.program->taskSequence->GetEntry(dueEntry.program->programCounter)->Execute();

            dueEntry.program->previousProgramCounter = dueEntry.program->programCounter;
            dueEntry.program->programCounter++;

          }

        }

      }

      sequences.Unlock();


      // cleanup unused resources

      if (cleanUpTimeOffset > 5000) { // every 5 seconds
        //printf("cleanup..\n");
        ResourceManagerPool::GetInstance().CleanUp();
        cleanUpTimeOffset = 0;
      }

    }

    return true;
  }


}
