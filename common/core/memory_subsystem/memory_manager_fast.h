#ifndef MEMORY_MANAGER_FAST_H
#define MEMORY_MANAGER_FAST_H

#include "memory_manager_base.h"
#include "mem_component.h"
#include "fixed_types.h"
#include "subsecond_time.h"

class MemoryManagerFast : public MemoryManagerBase
{
   protected:
      static constexpr UInt64 CACHE_LINE_BITS = 6;
      static constexpr UInt64 CACHE_LINE_SIZE = 1 << CACHE_LINE_BITS;

   public:
      MemoryManagerFast(Core* core, Network* network, ShmemPerfModel* shmem_perf_model)
         : MemoryManagerBase(core, network, shmem_perf_model)
      {}
      ~MemoryManagerFast() override = default;

      HitWhere::where_t coreInitiateMemoryAccess(
            const MemComponent::component_t mem_component,
            const Core::lock_signal_t lock_signal,
            const Core::mem_op_t mem_op_type,
            const IntPtr address, const UInt32 offset,
            Byte* data_buf, const UInt32 data_length,
            const Core::MemModeled modeled,
            const IntPtr eip) override // Added by Kleber Kruger
      {
         // Emulate slow interface by calling into fast interface
         assert(data_buf == nullptr);
         const SubsecondTime latency = coreInitiateMemoryAccessFast(mem_component == MemComponent::L1_ICACHE, mem_op_type, address);
         getShmemPerfModel()->incrElapsedTime(latency,  ShmemPerfModel::_USER_THREAD);
         if (latency > SubsecondTime::Zero())
            return HitWhere::MISS;

         return HitWhere::where_t(mem_component);
      }

      SubsecondTime coreInitiateMemoryAccessFast(
            bool icache,
            Core::mem_op_t mem_op_type,
            IntPtr address) override = 0;

      void handleMsgFromNetwork(NetPacket& packet) override { assert(false); }

      void enableModels() override {}
      void disableModels() override {}

      core_id_t getShmemRequester(const void* pkt_data) const override { assert(false); }
      UInt32 getModeledLength(const void* pkt_data) const override { assert(false); }

      #ifndef OPT_CACHEBLOCKSIZE
      [[nodiscard]] UInt64 getCacheBlockSize() const override { return CACHE_LINE_SIZE; }
      #endif

      void sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t sender_mem_component, MemComponent::component_t receiver_mem_component, core_id_t requester, core_id_t receiver, IntPtr address, Byte* data_buf = nullptr, UInt32 data_length = 0, HitWhere::where_t where = HitWhere::UNKNOWN, ShmemPerf *perf = nullptr, ShmemPerfModel::Thread_t thread_num = ShmemPerfModel::NUM_CORE_THREADS) override { assert(false); }
      void broadcastMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t sender_mem_component, MemComponent::component_t receiver_mem_component, core_id_t requester, IntPtr address, Byte* data_buf = nullptr, UInt32 data_length = 0, ShmemPerf *perf = nullptr, ShmemPerfModel::Thread_t thread_num = ShmemPerfModel::NUM_CORE_THREADS) override { assert(false); }

      [[nodiscard]] SubsecondTime getL1HitLatency() const override { return SubsecondTime::Zero(); }
      void addL1Hits(bool icache, Core::mem_op_t mem_op_type, UInt64 hits) override {}
};

#endif // MEMORY_MANAGER_FAST_H