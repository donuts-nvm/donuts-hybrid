// #ifndef CACHE_DONUTS_H
// #define CACHE_DONUTS_H
//
// #include <cache.h>
//
// class CacheLLCDonuts : Cache {
// public:
//    CacheLLCDonuts(const String& name,
//                   const String& cfgname,
//                   core_id_t core_id,
//                   UInt32 num_sets,
//                   UInt32 associativity,
//                   UInt32 cache_block_size,
//                   const String& replacement_policy,
//                   cache_t cache_type,
//                   hash_t hash                   = HASH_MASK,
//                   FaultInjector* fault_injector = nullptr,
//                   AddressHomeLookup* ahl        = nullptr);
//
//    ~CacheLLCDonuts() override = default;
//
//    [[nodiscard]] float getCacheThreshold() const { return m_cache_threshold; }
//    [[nodiscard]] float getCacheSetThreshold() const { return m_sets[0]->getThreshold(); }
//
// private:
//    float cache_set_threshold;
//    float cache_threshold;
// };
//
//
//
// #endif //CACHE_DONUTS_H
