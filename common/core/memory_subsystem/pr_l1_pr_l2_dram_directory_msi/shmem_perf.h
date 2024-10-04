#ifndef SHMEM_PERF_H
#define SHMEM_PERF_H

#include "subsecond_time.h"

#include <vector>

class NetPacket;

class ShmemPerf
{
   public:
      typedef enum {
         NOC_BASE,
         NOC_QUEUE,
         TD_ACCESS,
         INV_IMBALANCE,
         REMOTE_CACHE_INV,
         REMOTE_CACHE_FWD,
         REMOTE_CACHE_WB,
         PENDING_HIT,
         NUCA_TAGS,
         NUCA_QUEUE,
         NUCA_BUS,
         NUCA_DATA,
         DRAM_CACHE,
         DRAM_CACHE_TAGS,
         DRAM_CACHE_QUEUE,
         DRAM_CACHE_BUS,
         DRAM_CACHE_DATA,
         DRAM,
         DRAM_QUEUE,
         DRAM_BUS,
         DRAM_DEVICE,
         UNKNOWN,
         NUM_SHMEM_TIMES
      } shmem_times_type_t;

      ShmemPerf();
      void disable();
      void reset(SubsecondTime time, core_id_t core_id);
      void updateTime(SubsecondTime time, shmem_times_type_t reason = UNKNOWN);
      void updatePacket(NetPacket& packet);
      void add(ShmemPerf *perf);

      [[nodiscard]] core_id_t getCore() const { return m_core_id; }
      [[nodiscard]] SubsecondTime getInitialTime() const { return m_time_begin; }
      SubsecondTime &getComponent(const shmem_times_type_t reason) { return m_times[reason]; }

   private:
      core_id_t m_core_id;
      SubsecondTime m_time_begin;
      SubsecondTime m_time_last;
      std::vector<SubsecondTime> m_times;
};

const char* ShmemReasonString(ShmemPerf::shmem_times_type_t reason);

#endif // SHMEM_PERF_H
