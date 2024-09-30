#include "cache_block_info.h"
#include "pr_l1_cache_block_info.h"
#include "pr_l2_cache_block_info.h"
#include "shared_cache_block_info.h"
#include "log.h"
#include "simulator.h"     // Added by Kleber Kruger
#include "epoch_manager.h" // Added by Kleber Kruger

const char* CacheBlockInfo::option_names[] =
{
   "prefetch",
   "warmup",
};

const char* CacheBlockInfo::getOptionName(option_t option)
{
   static_assert(NUM_OPTIONS == sizeof(option_names) / sizeof(char*), "Not enough values in CacheBlockInfo::option_names");

   if (option < NUM_OPTIONS)
      return option_names[option];

   return "invalid";
}


CacheBlockInfo::CacheBlockInfo(const IntPtr tag, const CacheState::cstate_t cstate, const UInt64 options):
   m_tag(tag),
   m_cstate(cstate),
   m_owner(0),
   m_used(0),
   m_options(options),
   m_eid(0)                         // Added by Kleber Kruger
{}

CacheBlockInfo::~CacheBlockInfo() = default;

CacheBlockInfo*
CacheBlockInfo::create(const CacheBase::cache_t cache_type)
{
   switch (cache_type)
   {
      case CacheBase::PR_L1_CACHE:
         return new PrL1CacheBlockInfo();

      case CacheBase::PR_L2_CACHE:
         return new PrL2CacheBlockInfo();

      case CacheBase::SHARED_CACHE:
         return new SharedCacheBlockInfo();

      default:
         LOG_PRINT_ERROR("Unrecognized cache type (%u)", cache_type);
   }
}

void
CacheBlockInfo::invalidate()
{
   m_tag = ~0;
   m_cstate = CacheState::INVALID;
   m_eid = 0;                       // Added by Kleber Kruger
}

void
CacheBlockInfo::clone(CacheBlockInfo* cache_block_info)
{
   m_tag = cache_block_info->getTag();
   m_cstate = cache_block_info->getCState();
   m_owner = cache_block_info->m_owner;
   m_used = cache_block_info->m_used;
   m_options = cache_block_info->m_options;
   m_eid = cache_block_info->m_eid; // Added by Kleber Kruger
}

bool
CacheBlockInfo::updateUsage(const UInt32 offset, const UInt32 size)
{
   const UInt64 first = offset >> BitsUsedOffset,
                last  = (offset + size - 1) >> BitsUsedOffset,
                first_mask = (1ull << first) - 1,
                last_mask = (1ull << (last + 1)) - 1,
                usage_mask = last_mask & ~first_mask;

   return updateUsage(usage_mask);
}

bool
CacheBlockInfo::updateUsage(const BitsUsedType used)
{
   const bool new_bits_set = used & ~m_used; // Are we setting any bits that were previously unset?
   m_used |= used;                           // Update usage mask
   return new_bits_set;
}

void // Modified by Kleber Kruger
CacheBlockInfo::setCState(const CacheState::cstate_t cstate)
{
   // Added by Kleber Kruger
   if (Sim()->getProjectType() == ProjectType::DONUTS && cstate == CacheState::MODIFIED)
   {
      const UInt64 eid = EpochManager::getGlobalSystemEID();
      // TODO: In the cache_cntlr, case the system tries to write to a cache block from a past epoch not committed, commit it!
      LOG_ASSERT_ERROR(m_cstate != CacheState::MODIFIED || m_eid == eid, "It's not allowed to write to an uncommitted block (%lu -> %lu)", m_eid, eid);
      m_eid = eid;
   }
   m_cstate = cstate;
}