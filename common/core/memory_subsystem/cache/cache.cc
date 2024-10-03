#include "cache.h"
#include "config.hpp"// Added by Kleber Kruger
#include "log.h"
#include "simulator.h"

#include <cache_set_donuts.h>

// Cache class
// constructors/destructors
Cache::Cache(
   const String& name,
   const String& cfgname,
   const core_id_t core_id,
   const UInt32 num_sets,
   const UInt32 associativity,
   const UInt32 cache_block_size,
   const String& replacement_policy,
   const cache_t cache_type,
   const hash_t hash,
   FaultInjector *fault_injector,
   AddressHomeLookup *ahl)
:
   CacheBase(name, num_sets, associativity, cache_block_size, hash, ahl),
   m_enabled(false),
   m_num_accesses(0),
   m_num_hits(0),
   m_cache_type(cache_type),
   m_fault_injector(fault_injector),
   m_replacement_policy(CacheSet::parsePolicyType(replacement_policy)), // Added by Kleber Kruger
   m_cache_threshold(getCacheThreshold(cfgname))                        // Added by Kleber Kruger
{
   m_set_info = CacheSet::createCacheSetInfo(name, cfgname, core_id, replacement_policy, m_associativity);
   m_sets = new CacheSet*[m_num_sets];
   const auto cache_set_threshold = CacheSetDonuts::getCacheSetThreshold(cfgname, core_id);
   for (UInt32 i = 0; i < m_num_sets; i++)
   {  // Modified by Kleber Kruger (passing index as parameter also)
      m_sets[i] = CacheSet::createCacheSet(i, cfgname, core_id, m_replacement_policy, m_cache_type, m_associativity, m_blocksize, m_set_info, cache_set_threshold);
   }

   #ifdef ENABLE_SET_USAGE_HIST
   m_set_usage_hist = new UInt64[m_num_sets];
   for (UInt32 i = 0; i < m_num_sets; i++)
      m_set_usage_hist[i] = 0;
   #endif
}

Cache::~Cache()
{
   #ifdef ENABLE_SET_USAGE_HIST
   printf("Cache %s set usage:", m_name.c_str());
   for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
      printf(" %" PRId64, m_set_usage_hist[i]);
   printf("\n");
   delete [] m_set_usage_hist;
   #endif

   delete m_set_info;

   for (SInt32 i = 0; i < static_cast<SInt32>(m_num_sets); i++)
      delete m_sets[i];
   delete [] m_sets;
}

Lock&
Cache::getSetLock(const IntPtr addr) const
{
   IntPtr tag;
   UInt32 set_index;

   splitAddress(addr, tag, set_index);
   assert(set_index < m_num_sets);

   return m_sets[set_index]->getLock();
}

bool
Cache::invalidateSingleLine(const IntPtr addr) const
{
   IntPtr tag;
   UInt32 set_index;

   splitAddress(addr, tag, set_index);
   assert(set_index < m_num_sets);

   return m_sets[set_index]->invalidate(tag);
}

CacheBlockInfo*
Cache::accessSingleLine(const IntPtr addr, const access_t access_type,
      Byte* buff, const UInt32 bytes, const SubsecondTime& now, const bool update_replacement) const
{
   //assert((buff == NULL) == (bytes == 0));

   IntPtr tag;
   UInt32 set_index;
   UInt32 line_index = -1;
   UInt32 block_offset;

   splitAddress(addr, tag, set_index, block_offset);

   CacheSet* set = m_sets[set_index];
   CacheBlockInfo* cache_block_info = set->find(tag, &line_index);

   if (cache_block_info == nullptr)
      return nullptr;

   if (access_type == LOAD)
   {
      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
      if (m_fault_injector)
         m_fault_injector->preRead(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);

      set->read_line(line_index, block_offset, buff, bytes, update_replacement);
   }
   else
   {
      set->write_line(line_index, block_offset, buff, bytes, update_replacement);

      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
      if (m_fault_injector)
         m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);
   }

   return cache_block_info;
}

void
Cache::insertSingleLine(const IntPtr addr, const Byte* fill_buff,
      bool* eviction, IntPtr* evict_addr,
      CacheBlockInfo* evict_block_info, Byte* evict_buff,
      const SubsecondTime& now, CacheCntlr *cntlr) const
{
   IntPtr tag;
   UInt32 set_index;
   splitAddress(addr, tag, set_index);

   CacheBlockInfo* cache_block_info = CacheBlockInfo::create(m_cache_type);
   cache_block_info->setTag(tag);

   m_sets[set_index]->insert(cache_block_info, fill_buff,
         eviction, evict_block_info, evict_buff, cntlr);
   *evict_addr = tagToAddress(evict_block_info->getTag());

   // NVM Checkpoint Support (Added by Kleber Kruger)
   if (Sim()->getProjectType() == ProjectType::DONUTS && getCapacityUsed() >= m_cache_threshold.value())
      cntlr->checkpoint(CheckpointReason::CACHE_THRESHOLD, set_index);

   if (m_fault_injector) {
      // NOTE: no callback is generated for read of evicted data
      UInt32 line_index = -1;
      __attribute__((unused)) const CacheBlockInfo* res = m_sets[set_index]->find(tag, &line_index);
      LOG_ASSERT_ERROR(res != nullptr, "Inserted line no longer there?");

      m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, m_sets[set_index]->getBlockSize(), (Byte*)m_sets[set_index]->getDataPtr(line_index, 0), now);
   }

   #ifdef ENABLE_SET_USAGE_HIST
   ++m_set_usage_hist[set_index];
   #endif

   delete cache_block_info;
}


// Single line cache access at addr
CacheBlockInfo*
Cache::peekSingleLine(const IntPtr addr) const
{
   IntPtr tag;
   UInt32 set_index;
   splitAddress(addr, tag, set_index);

   return m_sets[set_index]->find(tag);
}

void
Cache::updateCounters(const bool cache_hit)
{
   if (m_enabled)
   {
      m_num_accesses ++;
      if (cache_hit)
         m_num_hits ++;
   }
}

void
Cache::updateHits(const Core::mem_op_t mem_op_type, const UInt64 hits)
{
   if (m_enabled)
   {
      m_num_accesses += hits;
      m_num_hits += hits;
   }
}

/**
 * Get percentage (0..1) of modified blocks in cache.
 * Added by Kleber Kruger
 */
float
Cache::getCapacityUsed() const
{
   UInt32 count = 0;
   for (UInt32 i = 0; i < m_num_sets; i++)
   {
      for (UInt32 j = 0; j < m_associativity; j++)
      {
         if (peekBlock(i, j)->isDirty())
            count++;
      }
   }
   return static_cast<float>(count) / static_cast<float>(m_num_sets * m_associativity);
}

/**
 * Get percentage (0..1) of modified blocks in cache.
 * Added by Kleber Kruger
 */
float
Cache::getSetCapacityUsed(const UInt32 index) const
{
   UInt32 count = 0;
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_sets[index]->peekBlock(i)->isDirty())
         count++;
   }
   return static_cast<float>(count) / static_cast<float>(m_associativity);
}

bool
Cache::isDonutsAndLLC(const String& cfgname)
{
   if (Sim()->getProjectType() == ProjectType::DONUTS)
   {
      const UInt32 levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
      const String last   = levels == 1 ? "perf_model/l1_dcache" : "perf_model/l" + String(std::to_string(levels).c_str()) + "_cache";
      return cfgname == last;
   }
   return false;
}

std::optional<float>
Cache::getCacheThreshold(const String& cfgname)
{
   if (!isDonutsAndLLC(cfgname))
      return std::nullopt;

   const String key = cfgname + "/cache_threshold";
   return Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloat(key))
                                          : DEFAULT_CACHE_THRESHOLD;
}
