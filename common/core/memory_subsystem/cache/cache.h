#ifndef CACHE_H
#define CACHE_H

#include "cache_base.h"
#include "cache_set.h"
#include "cache_block_info.h"
#include "cache_perf_model.h"
#include "core.h"
#include "fault_injection.h"

#include <optional>

// Define to enable the set usage histogram
//#define ENABLE_SET_USAGE_HIST

class Cache : public CacheBase
{
   public:
      // constructors/destructors
      Cache(const String& name,
            const String& cfgname,
            core_id_t core_id,
            UInt32 num_sets,
            UInt32 associativity,
            UInt32 cache_block_size,
            const String& replacement_policy,
            cache_t cache_type,
            hash_t hash = HASH_MASK,
            FaultInjector *fault_injector = nullptr,
            AddressHomeLookup *ahl = nullptr);
      ~Cache() override;

      [[nodiscard]] Lock& getSetLock(IntPtr addr) const;

      bool invalidateSingleLine(IntPtr addr) const;
      CacheBlockInfo* accessSingleLine(IntPtr addr,
            access_t access_type, Byte* buff, UInt32 bytes, const SubsecondTime& now, bool update_replacement) const;
      void insertSingleLine(IntPtr addr, const Byte* fill_buff,
            bool* eviction, IntPtr* evict_addr,
            CacheBlockInfo* evict_block_info, Byte* evict_buff, const SubsecondTime& now, CacheCntlr *cntlr = nullptr) const;
      [[nodiscard]] CacheBlockInfo* peekSingleLine(IntPtr addr) const;

      [[nodiscard]] CacheBlockInfo* peekBlock(const UInt32 set_index, const UInt32 way) const { return m_sets[set_index]->peekBlock(way); }

      // Update Cache Counters
      void updateCounters(bool cache_hit);
      void updateHits(Core::mem_op_t mem_op_type, UInt64 hits);

      [[nodiscard]] ReplacementPolicy getReplacementPolicy() const { return m_replacement_policy; } // Added by Kleber Kruger
      [[nodiscard]] float getCapacityUsed() const;                                                  // Added by Kleber Kruger
      [[nodiscard]] float getSetCapacityUsed(UInt32 index) const;                                   // Added by Kleber Kruger

      [[nodiscard]] static bool isDonutsAndLLC(const String& cfgname);                              // Added by Kleber Kruger
      [[nodiscard]] static std::optional<float> getCacheThreshold(const String& cfgname);           // Added by Kleber Kruger

      void enable() { m_enabled = true; }
      void disable() { m_enabled = false; }

protected:
      static constexpr float DEFAULT_CACHE_THRESHOLD = 1.0; // Added by Kleber Kruger

      bool m_enabled;

      // Cache counters
      UInt64 m_num_accesses;
      UInt64 m_num_hits;

      // Generic Cache Info
      cache_t m_cache_type;
      CacheSet** m_sets;
      CacheSetInfo* m_set_info;

      FaultInjector *m_fault_injector;

      ReplacementPolicy m_replacement_policy; // Added by Kleber Kruger
      std::optional<float> m_cache_threshold; // Added by Kleber Kruger

#ifdef ENABLE_SET_USAGE_HIST
      UInt64* m_set_usage_hist;
#endif
};

template <class T>
UInt32 moduloHashFn(T key, UInt32 hash_fn_param, UInt32 num_buckets)
{
   return (key >> hash_fn_param) % num_buckets;
}

#endif /* CACHE_H */