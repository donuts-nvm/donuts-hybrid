#pragma once

#include "mem_component.h"
#include "fixed_types.h"
#include "hit_where.h"

class ShmemPerf;

namespace PrL1PrL2DramDirectoryMSI
{
   class ShmemMsg
   {
      public:
         enum msg_t
         {
            INVALID_MSG_TYPE = 0,
            MIN_MSG_TYPE,
            // Cache > tag directory
            EX_REQ = MIN_MSG_TYPE,
            SH_REQ,
            UPGRADE_REQ,
            INV_REQ,
            FLUSH_REQ,
            WB_REQ,
            // Tag directory > cache
            EX_REP,
            SH_REP,
            UPGRADE_REP,
            INV_REP,
            FLUSH_REP,
            WB_REP,
            NULLIFY_REQ,
            // NVM Checkpoint Support (Added by Kleber Kruger)
            COMMIT,
            PERSIST,
            // Tag directory > DRAM
            DRAM_READ_REQ,
            DRAM_WRITE_REQ,
            // DRAM > tag directory
            DRAM_READ_REP,

            MAX_MSG_TYPE = NULLIFY_REQ,
            NUM_MSG_TYPES = MAX_MSG_TYPE - MIN_MSG_TYPE + 1
         };

      private:
         msg_t m_msg_type;
         MemComponent::component_t m_sender_mem_component;
         MemComponent::component_t m_receiver_mem_component;
         core_id_t m_requester;
         HitWhere::where_t m_where;
         IntPtr m_address;
         Byte* m_data_buf;
         UInt32 m_data_length;
         ShmemPerf* m_perf;

      public:
         ShmemMsg() = delete;
         explicit ShmemMsg(ShmemPerf* perf);
         ShmemMsg(msg_t msg_type,
               MemComponent::component_t sender_mem_component,
               MemComponent::component_t receiver_mem_component,
               core_id_t requester,
               IntPtr address,
               Byte* data_buf,
               UInt32 data_length,
               ShmemPerf* perf);
         explicit ShmemMsg(const ShmemMsg* shmem_msg);

         ~ShmemMsg();

         static ShmemMsg* getShmemMsg(const Byte* msg_buf, ShmemPerf* perf);
         [[nodiscard]] Byte* makeMsgBuf() const;
         [[nodiscard]] UInt32 getMsgLen() const;

         // Modeling
         [[nodiscard]] UInt32 getModeledLength() const;

         [[nodiscard]] msg_t getMsgType() const { return m_msg_type; }
         [[nodiscard]] MemComponent::component_t getSenderMemComponent() const { return m_sender_mem_component; }
         [[nodiscard]] MemComponent::component_t getReceiverMemComponent() const { return m_receiver_mem_component; }
         [[nodiscard]] core_id_t getRequester() const { return m_requester; }
         [[nodiscard]] IntPtr getAddress() const { return m_address; }
         [[nodiscard]] Byte* getDataBuf() const { return m_data_buf; }
         [[nodiscard]] UInt32 getDataLength() const { return m_data_length; }
         [[nodiscard]] HitWhere::where_t getWhere() const { return m_where; }

         void setDataBuf(Byte* data_buf) { m_data_buf = data_buf; }
         void setWhere(const HitWhere::where_t where) { m_where = where; }

         [[nodiscard]] ShmemPerf* getPerf() const { return m_perf; }

   };

}