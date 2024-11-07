#pragma once

#include "cache_cntlr.h"
#include "epoch_cntlr.h"

namespace ParametricDramDirectoryMSI
{

class CacheCntlrDonuts final : public CacheCntlr
{
public:

   enum class PersistencePolicy {
      SEQUENTIAL,
      FULLEST_FIRST,
      BALANCED
   };

   CacheCntlrDonuts(MemComponent::component_t mem_component,
                    String name,
                    core_id_t core_id,
                    MemoryManager* memory_manager,
                    AddressHomeLookup* tag_directory_home_lookup,
                    Semaphore* user_thread_sem,
                    Semaphore* network_thread_sem,
                    UInt32 cache_block_size,
                    CacheParameters& cache_params,
                    ShmemPerfModel* shmem_perf_model,
                    bool is_last_level_cache,
                    EpochCntlr* epoch_cntlr);

   ~CacheCntlrDonuts() override;

   void checkpoint(CheckpointReason checkpoint_reason, UInt32 evicted_set_index) override;

private:

   EpochCntlr *m_epoch_cntlr;
   PersistencePolicy m_persistence_policy;

   void addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const;
   std::queue<CacheBlockInfo*> selectDirtyBlocks(UInt32 evicted_set_index) const;

   void processCommit(IntPtr address, Byte* data_buf);
   void processPersist(IntPtr address, Byte* data_buf);

   void sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t receiver_mem_component, IntPtr address, Byte* data_buf);

   // Used to periodic checkpoints
   static SInt64 _checkpoint_timeout(const UInt64 arg, const UInt64 val) {
      reinterpret_cast<CacheCntlrDonuts *>(arg)->checkpoint(CheckpointReason::PERIODIC_TIME, 0);
      return 0;
   }

   // Used to periodic checkpoints
   static SInt64 _checkpoint_instr(const UInt64 arg, const UInt64 val) {
      reinterpret_cast<CacheCntlrDonuts *>(arg)->checkpoint(CheckpointReason::PERIODIC_INSTRUCTIONS, 0);
      return 0;
   }

   static PersistencePolicy getPersistencePolicy();

   void printCache() const; // ONLY FOR DEBUG! //
};

} // namespace ParametricDramDirectoryMSI