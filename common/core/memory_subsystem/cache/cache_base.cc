#include "cache_base.h"
#include "utils.h"
#include "log.h"
#include "rng.h"
#include "address_home_lookup.h"

CacheBase::CacheBase(
   const String& name, const UInt32 num_sets, const UInt32 associativity, const UInt32 cache_block_size,
   const hash_t hash, AddressHomeLookup *ahl)
:
   m_name(name),
   m_cache_size(static_cast<UInt64>(num_sets) * associativity * cache_block_size),
   m_associativity(associativity),
   m_blocksize(cache_block_size),
   m_hash(hash),
   m_num_sets(num_sets),
   m_ahl(ahl)
{
   m_log_blocksize = floorLog2(m_blocksize);
   m_log_num_sets = floorLog2(m_num_sets);

   LOG_ASSERT_ERROR((m_num_sets == (1UL << floorLog2(m_num_sets))) || (hash != CacheBase::HASH_MASK),
      "Caches of non-power of 2 size need funky hash function");
}

CacheBase::~CacheBase() = default;

// utilities
CacheBase::hash_t
CacheBase::parseAddressHash(const String& hash_name)
{
   if (hash_name == "mask")
      return HASH_MASK;
   if (hash_name == "mod")
      return HASH_MOD;
   if (hash_name == "rng1_mod")
      return HASH_RNG1_MOD;
   if (hash_name == "rng2_mod")
      return HASH_RNG2_MOD;
   if (hash_name == "prime_dis")
      return HASH_PRIME_DIS;
   if (hash_name == "xor_mod")
      return HASH_XOR_MOD;
   if (hash_name == "mersenne_mod")
      return HASH_MER_MOD;

   LOG_PRINT_ERROR("Invalid address hash function %s", hash_name.c_str());
}

void
CacheBase::splitAddress(const IntPtr addr, IntPtr& tag, UInt32& set_index) const
{
   tag = addr >> m_log_blocksize;

   const IntPtr linearAddress = m_ahl ? m_ahl->getLinearAddress(addr) : addr;
   const IntPtr block_num = linearAddress >> m_log_blocksize;

   switch(m_hash)
   {
      case HASH_MASK:
         set_index = block_num & (m_num_sets-1);
         break;
      case HASH_MOD:
         set_index = block_num % m_num_sets;
         break;
      case HASH_RNG1_MOD:
      {
         UInt64 state = rng_seed(block_num);
         set_index = rng_next(state) % m_num_sets;
         break;
      }
      case HASH_RNG2_MOD:
      {
         UInt64 state = rng_seed(block_num);
         rng_next(state);
         set_index = rng_next(state) % m_num_sets;
         break;
      }
      case HASH_PRIME_DIS:
      {
         //Prime Displacement Hashing Function (HPCA04)
         UInt64 si = block_num % m_num_sets;
         UInt64 Ti = block_num >> m_log_num_sets;
         UInt64 rho = 3;
         set_index = (rho * Ti + si) % m_num_sets;
         break;
      }
      case HASH_XOR_MOD:
      {
      	 //Based on related work of "Eliminating Conflict Misses Using Prime Number-Based Cache Indexing" (TC may 2005)
      	 //XOR based hash function
      	 UInt64 si = block_num % m_num_sets;
      	 UInt64 ti = (block_num >> m_log_num_sets) % m_num_sets;
      	 set_index = si ^ ti; // ^ -> bitwise XOR
      	 break;
      }
      case HASH_MER_MOD:
      {
      	 //Based on related work of "Eliminating Conflict Misses Using Prime Number-Based Cache Indexing" (TC may 2005)
      	 //Mersenne based hash function
      	 //Disadvantage of this mod is that we will not use one set in our cache.
      	 set_index = block_num % (m_num_sets - 1);
      	 break;
      }
      default:
         LOG_PRINT_ERROR("Invalid hash function %d", m_hash);
   }
}

void
CacheBase::splitAddress(const IntPtr addr, IntPtr& tag, UInt32& set_index,
                  UInt32& block_offset) const
{
   block_offset = addr & (m_blocksize-1);
   splitAddress(addr, tag, set_index);
}

IntPtr
CacheBase::tagToAddress(const IntPtr tag) const
{
   return tag << m_log_blocksize;
}
