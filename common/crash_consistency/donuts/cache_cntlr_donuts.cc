#include "cache_cntlr_donuts.h"
#include "memory_manager.h"
#include "hooks_manager.h"
#include "simulator.h"
#include "config.hpp"

#include <ranges>

namespace ParametricDramDirectoryMSI
{

CacheCntlrDonuts::CacheCntlrDonuts(MemComponent::component_t mem_component,
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
                                   EpochCntlr* epoch_cntlr) :
      CacheCntlr(mem_component, name, core_id, memory_manager, tag_directory_home_lookup, user_thread_sem, network_thread_sem, cache_block_size, cache_params, shmem_perf_model, is_last_level_cache),
m_epoch_cntlr(epoch_cntlr),
    m_persistence_policy(getPersistencePolicy())
{
//   printf("Cache %s (%p) | CacheCntlr (%p) | Core %u/%u\n", getCache()->getName().c_str(), getCache(), this, m_core_id, m_core_id_master);

   if (is_last_level_cache)
   {
      LOG_ASSERT_ERROR(!m_cache_writethrough, "DONUTS does not allow LLC write-through");

      // TODO: Implement for non-unified LLC... Use a Master Controller??
      LOG_ASSERT_ERROR(m_core_id_master == 0, "DONUTS does not allow non-unified LLC yet");

//      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT, _checkpoint_timeout, (UInt64) this);
//      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS, _checkpoint_instr, (UInt64) this);
   }
}

CacheCntlrDonuts::~CacheCntlrDonuts() = default;

void
CacheCntlrDonuts::addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const
{
   for (UInt32 way = 0; way < m_master->m_cache->getAssociativity(); way++)
   {
      CacheBlockInfo* block_info = m_master->m_cache->peekBlock(set_index, way);
      if (block_info->isDirty())
         dirty_blocks.push(block_info);
   }
}

/**
 * Select the dirty blocks from the cache according to persistence policy.
 * TODO: When it is a non-unified cache, you must get all the dirty cache blocks that will be on other controllers.
 *
 * @param evicted_set_index
 * @return a sorted queue containing all the dirty blocks
 */
std::queue<CacheBlockInfo*>
CacheCntlrDonuts::selectDirtyBlocks(UInt32 evicted_set_index) const
{
   LOG_ASSERT_ERROR(m_persistence_policy != PersistencePolicy::BALANCED, "Persistence policy not yet implemented");

   std::queue<CacheBlockInfo*> dirty_blocks;
   std::vector<std::pair<UInt32, double> > other_sets;

   addAllDirtyBlocks(dirty_blocks, evicted_set_index);

   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
   {
      if (i == evicted_set_index) continue;
      const auto used = m_persistence_policy == PersistencePolicy::FULLEST_FIRST ? m_master->m_cache->getSetCapacityUsed(i) : 1;
      if (used > 0) other_sets.emplace_back(i, used);
   }

   if (m_persistence_policy == PersistencePolicy::FULLEST_FIRST)
      std::sort(other_sets.begin(), other_sets.end(), [](const auto& a, const auto& b) { return (a.second > b.second); });

   for (auto set: other_sets)
      addAllDirtyBlocks(dirty_blocks, set.first);

   return dirty_blocks;
}

void
CacheCntlrDonuts::checkpoint(CheckpointReason event_type, UInt32 evicted_set_index)
{
   LOG_ASSERT_ERROR(isLastLevel(), "Only the LLC controller can perform a checkpoint");

   auto dirty_blocks = selectDirtyBlocks(evicted_set_index);
   if (!dirty_blocks.empty())
   {
      printf("BEFORE checkpoint | Sending in %lu...\n", getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD).getNS());
      // auto *nvm_cntlr = dynamic_cast<PrL1PrL2DramDirectoryMSI::NvmCntlrDonuts*>(getMemoryManager()->getDramCntlr());
      // nvm_cntlr->checkpoint(event_type, dirty_blocks.size(), m_master->m_cache->getCapacityUsed());

      while (!dirty_blocks.empty())
      {
         auto *cache_block = dirty_blocks.front();
         IntPtr address = m_master->m_cache->tagToAddress(cache_block->getTag());
         Byte data_buf[getCacheBlockSize()];
         processCommit(address, data_buf);
         processPersist(address, data_buf);
         dirty_blocks.pop();
      }
      printf("AFTER checkpoint | Sending in %lu...\n", getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD).getNS());

      //         UInt64 eid = m_epoch_cntlr->getCurrentEID();
      //         m_epoch_cntlr->commit();
      //         m_epoch_cntlr->registerPersistedEID(eid);
   }
}

void
CacheCntlrDonuts::processCommit(IntPtr address, Byte* data_buf)
{
//   if (m_writebuffer_enabled)
//   {
//      auto cache_block = getCacheBlockInfo(address);
//      auto latency = m_writebuffer_cntlr->insert(address, 0, nullptr, getCacheBlockSize(), ShmemPerfModel::_USER_THREAD, cache_block->getEpochID());
//      getMemoryManager()->incrElapsedTime(latency, ShmemPerfModel::_USER_THREAD);
//   }
   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT, MemComponent::TAG_DIR, address, nullptr);
   updateCacheBlock(address, CacheState::SHARED, Transition::COHERENCY, data_buf, ShmemPerfModel::_USER_THREAD);
}
void
CacheCntlrDonuts::processPersist(IntPtr address, Byte* data_buf)
{
   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::PERSIST, MemComponent::TAG_DIR, address, data_buf);
}

void
CacheCntlrDonuts::sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t receiver_mem_component,
                            IntPtr address, Byte* data_buf)
{
   getMemoryManager()->sendMsg(msg_type,MemComponent::LAST_LEVEL_CACHE, receiver_mem_component,
                               m_core_id_master, getHome(address), /* requester and receiver */
                               address, data_buf, data_buf != nullptr ? getCacheBlockSize() : 0,
                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
}

CacheCntlrDonuts::PersistencePolicy
CacheCntlrDonuts::getPersistencePolicy()
{
   const String param = "donuts/persistence_policy";
   const String value = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "sequential";

   if (value == "sequential") return PersistencePolicy::SEQUENTIAL;
   if (value == "fullest_first") return PersistencePolicy::FULLEST_FIRST;
   if (value == "balanced") return PersistencePolicy::BALANCED;

   LOG_ASSERT_ERROR(false, "Persistence policy not found: %", value.c_str());
}

/********************************************************************************
 * ONLY FOR DEBUG!
 ********************************************************************************/
void CacheCntlrDonuts::printCache() const
{
   printf("============================================================\n");
   printf("Cache %s (%.2f%%) (%p)\n", getCache()->getName().c_str(), getCache()->getCapacityUsed() * 100, getCache());
   printf("------------------------------------------------------------\n");
   for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
   {
      printf("%s%4d  ", j == 0 ? "    " : "", j);
   }
   printf("\n------------------------------------------------------------\n");
   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
   {
      printf("%4d ", i);
      for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
      {
         auto cache_block = m_master->m_cache->peekBlock(i, j);
         auto state       = cache_block->getCState() != CacheState::INVALID ? cache_block->getCStateString() : ' ';
         printf("[%c %lu] ", state, cache_block->getEpochID());
      }
      printf("= %.1f\n", m_master->m_cache->getSetCapacityUsed(i) * 100);
   }
   printf("============================================================\n");
}

}