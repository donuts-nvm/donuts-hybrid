#ifndef CHECKPOINT_EVENT_H
#define CHECKPOINT_EVENT_H

#include "subsecond_time.h"

class CheckpointEvent
{
public:
    enum class Reason
    {
        PERIODIC_TIME,
        PERIODIC_INSTRUCTIONS,
        CACHE_SET_THRESHOLD,
        CACHE_THRESHOLD,
        NUM_EVENT_TYPES
     };

    CheckpointEvent(const Reason reason, const SubsecondTime& time, const IntPtr pc,
                    const UInt64 numLogs, const UInt64 checkpointSize) :
        reason(reason),
        time(time),
        pc(pc),
        num_logs(numLogs),
        checkpoint_size(checkpointSize) {}

    [[nodiscard]] Reason getReason() const { return reason; }
    [[nodiscard]] const SubsecondTime& getTime() const { return time; }
    [[nodiscard]] IntPtr getPc() const { return pc; }
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

#endif //CHECKPOINT_EVENT_H
