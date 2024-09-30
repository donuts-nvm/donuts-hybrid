#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "config.h"
#include "log.h"
#include "inst_mode.h"
#include "epoch_manager.h" // Added by Kleber Kruger

#include <decoder.h>
#include <optional>

class _Thread;
class SyscallServer;
class SyncServer;
class MagicServer;
class ClockSkewMinimizationServer;
class StatsManager;
class Transport;
class CoreManager;
class Thread;
class ThreadManager;
class ThreadStatsManager;
class SimThreadManager;
class HooksManager;
class ClockSkewMinimizationManager;
class FastForwardPerformanceManager;
class TraceManager;
class DvfsManager;
class SamplingManager;
class FaultinjectionManager;
class TagsManager;
class RoutineTracer;
class MemoryTracker;
namespace config { class Config; }

// Added by Kleber Kruger
class Project {
public:
   enum class Type
   {
      UNKNOWN,
      BASELINE,
      DONUTS
   };

   static String toName(Type project_type);
   static Type toType(const String &project_name);
};
using ProjectType = Project::Type;

class Simulator
{
public:
   Simulator();
   ~Simulator();

   void start();

   static Simulator* getSingleton() { return m_singleton; }
   static void setConfig(config::Config * cfg, Config::SimulationMode mode);
   static void allocate();
   static void release();

   [[nodiscard]] SyscallServer* getSyscallServer() const { return m_syscall_server; }
   [[nodiscard]] SyncServer* getSyncServer() const { return m_sync_server; }
   [[nodiscard]] MagicServer* getMagicServer() const { return m_magic_server; }
   [[nodiscard]] ClockSkewMinimizationServer* getClockSkewMinimizationServer() const { return m_clock_skew_minimization_server; }
   [[nodiscard]] CoreManager *getCoreManager() const { return m_core_manager; }
   [[nodiscard]] SimThreadManager *getSimThreadManager() const { return m_sim_thread_manager; }
   [[nodiscard]] ThreadManager *getThreadManager() const { return m_thread_manager; }
   [[nodiscard]] ClockSkewMinimizationManager *getClockSkewMinimizationManager() const { return m_clock_skew_minimization_manager; }
   [[nodiscard]] FastForwardPerformanceManager *getFastForwardPerformanceManager() const { return m_fastforward_performance_manager; }
   Config *getConfig() { return &m_config; }
   static config::Config *getCfg() {
      //if (! m_config_file_allowed)
      //   LOG_PRINT_ERROR("getCfg() called after init, this is not nice\n");
      return m_config_file;
   }
   static void hideCfg() { m_config_file_allowed = false; }
   [[nodiscard]] StatsManager *getStatsManager() const { return m_stats_manager; }
   [[nodiscard]] ThreadStatsManager *getThreadStatsManager() const { return m_thread_stats_manager; }
   [[nodiscard]] DvfsManager *getDvfsManager() const { return m_dvfs_manager; }
   [[nodiscard]] HooksManager *getHooksManager() const { return m_hooks_manager; }
   [[nodiscard]] SamplingManager *getSamplingManager() const { return m_sampling_manager; }
   [[nodiscard]] FaultinjectionManager *getFaultinjectionManager() const { return m_faultinjection_manager; }
   [[nodiscard]] TraceManager *getTraceManager() const { return m_trace_manager; }
   [[nodiscard]] TagsManager *getTagsManager() const { return m_tags_manager; }
   [[nodiscard]] const std::optional<EpochManager>& getEpochManager() const { return m_epoch_manager; }
   [[nodiscard]] RoutineTracer *getRoutineTracer() const { return m_rtn_tracer; }
   [[nodiscard]] MemoryTracker *getMemoryTracker() const { return m_memory_tracker; }
   void setMemoryTracker(MemoryTracker *memory_tracker) { m_memory_tracker = memory_tracker; }

   [[nodiscard]] bool isRunning() const { return m_running; }
   static void enablePerformanceModels();
   static void disablePerformanceModels();

   void setInstrumentationMode(InstMode::inst_mode_t new_mode, bool update_barrier) const;
   static InstMode::inst_mode_t getInstrumentationMode() { return InstMode::inst_mode; }

   // Access to the Decoder library for the simulator run
   void createDecoder();
   static dl::Decoder *getDecoder();

   [[nodiscard]] ProjectType getProjectType() const { return m_project_type; }  // Added by Kleber Kruger

private:
   Config m_config;
   Log m_log;
   TagsManager *m_tags_manager;
   SyscallServer *m_syscall_server;
   SyncServer *m_sync_server;
   MagicServer *m_magic_server;
   ClockSkewMinimizationServer *m_clock_skew_minimization_server;
   StatsManager *m_stats_manager;
   Transport *m_transport;
   CoreManager *m_core_manager;
   ThreadManager *m_thread_manager;
   ThreadStatsManager *m_thread_stats_manager;
   SimThreadManager *m_sim_thread_manager;
   ClockSkewMinimizationManager *m_clock_skew_minimization_manager;
   FastForwardPerformanceManager *m_fastforward_performance_manager;
   TraceManager *m_trace_manager;
   DvfsManager *m_dvfs_manager;
   HooksManager *m_hooks_manager;
   SamplingManager *m_sampling_manager;
   FaultinjectionManager *m_faultinjection_manager;
   std::optional<EpochManager> m_epoch_manager; // Added by Kleber Kruger
   RoutineTracer *m_rtn_tracer;
   MemoryTracker *m_memory_tracker;
   ProjectType m_project_type;                  // Added by Kleber Kruger

   bool m_running;
   bool m_inst_mode_output;

   static Simulator *m_singleton;

   static config::Config *m_config_file;
   static bool m_config_file_allowed;
   static Config::SimulationMode m_mode;

   static ProjectType loadProjectType();  // Added by Kleber Kruger

   // Object to access the decoder library with the correct configuration
   static dl::Decoder *m_decoder;
   // Surrogate to create a Decoder object for a specific architecture
   dl::DecoderFactory *m_factory;

   void printInstModeSummary();
};

__attribute__((unused)) static Simulator *Sim()
{
   return Simulator::getSingleton();
}

#endif // SIMULATOR_H
