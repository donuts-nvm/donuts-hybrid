#ifndef CACHE_SET_DONUTS_H
#define CACHE_SET_DONUTS_H

#include "cache_set.h"

class CacheSetDonuts : public virtual CacheSet {
public:
   CacheSetDonuts(CacheBase::cache_t cache_type,
                  UInt32 index, UInt32 associativity, UInt32 blocksize,
                  float cache_set_threshold = DEFAULT_CACHE_SET_THRESHOLD);

   ~CacheSetDonuts() override;

   static std::optional<float> getCacheSetThreshold(const String& cfgname, core_id_t core_id);

protected:
   static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;

   UInt32 m_index;
   float m_cache_set_threshold;

   bool isValidReplacement(UInt32 index) override;
};



#endif //CACHE_SET_DONUTS_H
