#ifndef __CACHE_ATD_H
#define __CACHE_ATD_H

#include "fixed_types.h"
#include "cache_base.h"
#include "cache_set.h"
#include "core.h"

#include <unordered_map>

class CacheSet;

class ATD
{
   private:
      CacheBase m_cache_base;
      std::unordered_map<UInt32, CacheSet*> m_sets;
      CacheSetInfo *m_set_info;

      UInt64 loads, stores;
      UInt64 load_misses, store_misses;
      UInt64 loads_constructive, stores_constructive;
      UInt64 loads_destructive, stores_destructive;

      [[nodiscard]] bool isSampledSet(UInt32 set_index) const;

   public:
      ATD(const String& name, const String& configName, core_id_t core_id, UInt32 num_sets, UInt32 associativity,
          UInt32 cache_block_size, const String& replacement_policy, CacheBase::hash_t hash_function);
      ~ATD();

      void access(Core::mem_op_t mem_op_type, bool cache_hit, IntPtr address);
};

#endif // __CACHE_ATD_H
