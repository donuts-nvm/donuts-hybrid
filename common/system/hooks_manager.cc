#include "hooks_manager.h"
#include "log.h"

const char* HookType::hook_type_names[] = {
   "HOOK_PERIODIC",
   "HOOK_PERIODIC_INS",
   "HOOK_SIM_START",
   "HOOK_SIM_END",
   "HOOK_ROI_BEGIN",
   "HOOK_ROI_END",
   "HOOK_CPUFREQ_CHANGE",
   "HOOK_MAGIC_MARKER",
   "HOOK_MAGIC_USER",
   "HOOK_INSTR_COUNT",
   "HOOK_THREAD_CREATE",
   "HOOK_THREAD_START",
   "HOOK_THREAD_EXIT",
   "HOOK_THREAD_STALL",
   "HOOK_THREAD_RESUME",
   "HOOK_THREAD_MIGRATE",
   "HOOK_INSTRUMENT_MODE",
   "HOOK_PRE_STAT_WRITE",
   "HOOK_SYSCALL_ENTER",
   "HOOK_SYSCALL_EXIT",
   "HOOK_APPLICATION_START",
   "HOOK_APPLICATION_EXIT",
   "HOOK_APPLICATION_ROI_BEGIN",
   "HOOK_APPLICATION_ROI_END",
   "HOOK_SIGUSR1",
   "HOOK_EPOCH_START",        // Added by Kleber Kruger
   "HOOK_EPOCH_END",          // Added by Kleber Kruger
   "HOOK_EPOCH_PERSISTED",    // Added by Kleber Kruger
   "HOOK_EPOCH_TIMEOUT",      // Added by Kleber Kruger
   "HOOK_EPOCH_TIMEOUT_INS"   // Added by Kleber Kruger
};
static_assert(HookType::HOOK_TYPES_MAX == std::size(HookType::hook_type_names), "Not enough values in HookType::hook_type_names");

HooksManager::HooksManager() = default;

void HooksManager::registerHook(const HookType::hook_type_t type, const HookCallbackFunc func, const UInt64 argument, const HookCallbackOrder order)
{
   m_registry[type].emplace_back(func, argument, order);
}

SInt64 HooksManager::callHooks(const HookType::hook_type_t type, const UInt64 argument, const bool expect_return)
{
   for(unsigned int order = 0; order < NUM_HOOK_ORDER; ++order)
   {
      for(const auto & it : m_registry[type])
      {
         if (it.order == static_cast<HookCallbackOrder>(order))
         {
            SInt64 result = it.func(it.arg, argument);
            if (expect_return && result != -1)
               return result;
         }
      }
   }

   return -1;
}
