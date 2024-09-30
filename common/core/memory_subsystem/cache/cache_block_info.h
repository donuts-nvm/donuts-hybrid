#ifndef CACHE_BLOCK_INFO_H_
#define CACHE_BLOCK_INFO_H_

#include "fixed_types.h"
#include "cache_state.h"
#include "cache_base.h"
#include "checkpoint_event.h" // Added by Kleber Kruger

class CacheBlockInfo
{
   public:
      enum option_t
      {
         PREFETCH,
         WARMUP,
         NUM_OPTIONS
      };

      static constexpr UInt8 BitsUsedOffset = 3; // Track usage on 1<<BitsUsedOffset granularity (per 64-bit / 8-byte)
      using BitsUsedType = UInt8; // Enough to store one bit per 1<<BitsUsedOffset byte element per cache line (8 8-byte elements for 64-byte cache lines)

   // This can be extended later to include other information
   // for different cache coherence protocols
   private:
      IntPtr m_tag;
      CacheState::cstate_t m_cstate;
      UInt64 m_owner;
      BitsUsedType m_used;
      UInt8 m_options;  // large enough to hold a bitfield for all available option_t's
      UInt64 m_eid;     // Added by Kleber Kruger

      static const char* option_names[];

   public:
      explicit CacheBlockInfo(IntPtr tag = ~0,
            CacheState::cstate_t cstate = CacheState::INVALID,
            UInt64 options = 0);
      virtual ~CacheBlockInfo();

      static CacheBlockInfo* create(CacheBase::cache_t cache_type);

      virtual void invalidate();
      virtual void clone(CacheBlockInfo* cache_block_info);

      [[nodiscard]] bool isValid() const { return m_tag != static_cast<IntPtr>(~0L); }
      [[nodiscard]] bool isDirty() const { return m_cstate == CacheState::MODIFIED; }        // Added by Kleber Kruger

      [[nodiscard]] IntPtr getTag() const { return m_tag; }
      [[nodiscard]] CacheState::cstate_t getCState() const { return m_cstate; }
      [[nodiscard]] char getCStateString() const { return CacheState(m_cstate).to_char(); }  // Added by Kleber Kruger

      void setTag(const IntPtr tag) { m_tag = tag; }
      void setCState(CacheState::cstate_t cstate);                                           // Modified by Kleber Kruger

      [[nodiscard]] UInt64 getOwner() const { return m_owner; }
      void setOwner(const UInt64 owner) { m_owner = owner; }

      [[nodiscard]] UInt64 getEpochID() const { return m_eid; }   // Added by Kleber Kruger
      void setEpochID(const UInt64 eid) { m_eid = eid; }          // Added by Kleber Kruger

      [[nodiscard]] bool hasOption(const option_t option) const { return m_options & 1 << option; }
      void setOption(const option_t option) { m_options |= 1 << option; }
      void clearOption(const option_t option) { m_options &= ~(static_cast<UInt64>(1) << option); }

      [[nodiscard]] BitsUsedType getUsage() const { return m_used; };
      bool updateUsage(UInt32 offset, UInt32 size);
      bool updateUsage(BitsUsedType used);

      static const char* getOptionName(option_t option);
};

class CacheCntlr
{
   public:
      virtual ~CacheCntlr() = default;

      virtual bool isInLowerLevelCache(CacheBlockInfo *block_info) { return false; }
      virtual void incrementQBSLookupCost() {}

      // Added by Kleber Kruger to support checkpoint
      virtual void checkpoint(const CheckpointReason reason, const UInt32 evicted_set_index) {};
};

#endif /* CACHE_BLOCK_INFO_H_ */