#include "cache_set_donuts.h"
#include "cache.h"
#include "simulator.h"
#include "config.hpp"

CacheSetDonuts::CacheSetDonuts(const CacheBase::cache_t cache_type,
                               const UInt32 index,
                               const UInt32 associativity,
                               const UInt32 blocksize,
                               const float cache_set_threshold) :
      CacheSet(cache_type, associativity, blocksize),
      m_index(index),
      m_cache_set_threshold(cache_set_threshold)
{ }

CacheSetDonuts::~CacheSetDonuts() = default;

bool CacheSetDonuts::isValidReplacement(const UInt32 index)
{
   const auto state = m_cache_block_info_array[index]->getCState();
   return state != CacheState::SHARED_UPGRADING && state != CacheState::MODIFIED;
}

std::optional<float>
CacheSetDonuts::getCacheSetThreshold(const String& cfgname, const core_id_t core_id)
{
   if (!Cache::isDonutsAndLLC(cfgname))
      return std::nullopt;

   const String key = cfgname + "/cache_set_threshold";
   return Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloatArray(key, core_id))
                                          : DEFAULT_CACHE_SET_THRESHOLD;
}
