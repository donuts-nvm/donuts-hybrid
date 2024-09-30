#ifndef CACHE_SET_H
#define CACHE_SET_H

#include "fixed_types.h"
#include "cache_block_info.h"
#include "cache_base.h"
#include "lock.h"
#include "random.h"
#include "log.h"

// Per-cache object to store replacement-policy related info (e.g. statistics),
// can collect data from all CacheSet* objects which are per set and implement the actual replacement policy
class CacheSetInfo
{
   public:
      virtual ~CacheSetInfo() = default;
};

// Everything related to cache sets
class CacheSet
{
   public:

      CacheSet(CacheBase::cache_t cache_type, UInt32 associativity, UInt32 blocksize);
      virtual ~CacheSet();

      [[nodiscard]] UInt32 getBlockSize() const { return m_blocksize; }
      [[nodiscard]] UInt32 getAssociativity() const { return m_associativity; }
      Lock& getLock() { return m_lock; }

      void read_line(UInt32 line_index, UInt32 offset, Byte *out_buff, UInt32 bytes, bool update_replacement);
      void write_line(UInt32 line_index, UInt32 offset, const Byte *in_buff, UInt32 bytes, bool update_replacement);
      CacheBlockInfo* find(IntPtr tag, UInt32* line_index = nullptr) const;
      [[nodiscard]] bool invalidate(const IntPtr& tag) const;
      void insert(CacheBlockInfo* cache_block_info, const Byte* fill_buff, bool* eviction, CacheBlockInfo* evict_block_info, Byte* evict_buff, CacheCntlr *cntlr = nullptr);

      [[nodiscard]] CacheBlockInfo* peekBlock(const UInt32 way) const { return m_cache_block_info_array[way]; }

      [[nodiscard]] char* getDataPtr(UInt32 line_index, UInt32 offset = 0) const;

      virtual UInt32 getReplacementIndex(CacheCntlr *cntlr) = 0;
      virtual void updateReplacementIndex(UInt32) = 0;

      virtual bool isValidReplacement(UInt32 index); // Modified by Kleber Kruger (now is virtual)

      // Modified by Kleber Kruger (added arg index and cache_set_threshold)
      static CacheSet* createCacheSet(UInt32 index, const String& cfgname, core_id_t core_id, CacheBase::ReplacementPolicy replacement_policy, CacheBase::cache_t cache_type,
                                      UInt32 associativity, UInt32 blocksize, CacheSetInfo* set_info = nullptr,
                                      float cache_set_threshold = 1.0f); // Arg added by Kleber Kruger
      static CacheSetInfo* createCacheSetInfo(const String& name, const String& cfgname, core_id_t core_id, const String& replacement_policy, UInt32 associativity);
      static CacheBase::ReplacementPolicy parsePolicyType(const String& policy);
      static UInt8 getNumQBSAttempts(CacheBase::ReplacementPolicy, const String& cfgname, core_id_t core_id);

   protected:
      CacheBlockInfo** m_cache_block_info_array;
      char* m_blocks;
      UInt32 m_associativity;
      UInt32 m_blocksize;
      Lock m_lock;
};

#endif /* CACHE_SET_H */