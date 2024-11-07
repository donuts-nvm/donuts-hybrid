#pragma once

#include "subsecond_time.h"

class CheckpointEvent
{
public:
   enum class Reason
   {
      PERIODIC_TIME,
      PERIODIC_INSTRUCTIONS,
      CACHE_SET_THRESHOLD,
      CACHE_THRESHOLD
   };

   CheckpointEvent(const Reason reason, const SubsecondTime& time, const IntPtr pc,
                   const UInt64 num_logs, const UInt64 checkpoint_size) :
                   reason(reason),
                   time(time),
                   pc(pc),
                   num_logs(num_logs),
                   checkpoint_size(checkpoint_size) { }

   [[nodiscard]] Reason getReason() const { return reason; }
   [[nodiscard]] const SubsecondTime& getTime() const { return time; }
   [[nodiscard]] IntPtr getPC() const { return pc; }
   [[nodiscard]] UInt64 getNumLogs() const { return num_logs; }
   [[nodiscard]] UInt64 getCheckpointSize() const { return checkpoint_size; }

private:
   const Reason reason;
   const SubsecondTime time;
   const IntPtr pc;
   const UInt64 num_logs;
   const UInt64 checkpoint_size;
   // TODO: Total time to persist this checkpoint?
};

using CheckpointReason = CheckpointEvent::Reason;
