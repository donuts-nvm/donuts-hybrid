#ifndef CACHE_SET_LRU_H
#define CACHE_SET_LRU_H

#include "cache_set.h"

class CacheSetInfoLRU final : public CacheSetInfo
{
   public:
      CacheSetInfoLRU(const String& name, const String& cfgname, core_id_t core_id, UInt32 associativity, UInt8 num_attempts);
      ~CacheSetInfoLRU() override;
      void increment(const UInt32 index) const
      {
         LOG_ASSERT_ERROR(index < m_associativity, "Index(%d) >= Associativity(%d)", index, m_associativity);
         ++m_access[index];
      }
      void incrementAttempt(const UInt8 attempt) const
      {
         if (m_attempts)
            ++m_attempts[attempt];
         else
            LOG_ASSERT_ERROR(attempt == 0, "No place to store attempt# histogram but attempt != 0");
      }
   private:
      const UInt32 m_associativity;
      UInt64* m_access;
      UInt64* m_attempts;
};

class CacheSetLRU : public virtual CacheSet // Modified by Kleber Kruger (added virtual keyword)
{
   public:
      CacheSetLRU(CacheBase::cache_t cache_type,
            UInt32 associativity, UInt32 blocksize, CacheSetInfoLRU* set_info, UInt8 num_attempts);
      ~CacheSetLRU() override;

      UInt32 getReplacementIndex(CacheCntlr *cntlr) override;
      void updateReplacementIndex(UInt32 accessed_index) override;

   protected:
      const UInt8 m_num_attempts;
      UInt8* m_lru_bits;
      CacheSetInfoLRU* m_set_info;
      void moveToMRU(UInt32 accessed_index) const;
};

#endif /* CACHE_SET_LRU_H */
