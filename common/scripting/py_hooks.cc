#include "hooks_py.h"
#include "subsecond_time.h"
#include "simulator.h"
#include "hooks_manager.h"
#include "magic_server.h"
#include "syscall_model.h"
#include "sim_api.h"

static SInt64 hookCallbackResult(PyObject *pResult)
{
   SInt64 result = -1;
   if (pResult == nullptr)
      return -1;
   if (PyInt_Check(pResult))
      result = PyInt_AsLong(pResult);
   else if (PyLong_Check(pResult))
      result = PyLong_AsLong(pResult);
   Py_DECREF(pResult);
   return result;
}

static SInt64 hookCallbackNone(const UInt64 pFunc, const UInt64)
{
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, nullptr);
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackInt(const UInt64 pFunc, const UInt64 argument)
{
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(L)", argument));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackSubsecondTime(const UInt64 pFunc, const UInt64 argument)
{
   const SubsecondTime time(*reinterpret_cast<subsecond_time_t*>(argument));
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(L)", time.getFS()));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackString(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<const char*>(_argument);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(s)", argument));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackMagicMarkerType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<MagicServer::MagicMarkerType*>(_argument);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iiKKs)", argument->thread_id, argument->core_id, argument->arg0, argument->arg1, argument->str));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackThreadCreateType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<HooksManager::ThreadCreate*>(_argument);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(ii)", argument->thread_id, argument->creator_thread_id));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackThreadTimeType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<HooksManager::ThreadTime*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iL)", argument->thread_id, time.getFS()));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackThreadStallType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<HooksManager::ThreadStall*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(isL)", argument->thread_id, ThreadManager::stall_type_names[argument->reason], time.getFS()));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackThreadResumeType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<HooksManager::ThreadResume*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iiL)", argument->thread_id, argument->thread_by, time.getFS()));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackThreadMigrateType(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<HooksManager::ThreadMigrate*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iiL)", argument->thread_id, argument->core_id, time.getFS()));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackSyscallEnter(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<SyscallMdl::HookSyscallEnter*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iiLi(llllll))", argument->thread_id, argument->core_id, time.getFS(),
      argument->syscall_number, argument->args.arg0, argument->args.arg1, argument->args.arg2, argument->args.arg3, argument->args.arg4, argument->args.arg5));
   return hookCallbackResult(pResult);
}

static SInt64 hookCallbackSyscallExit(const UInt64 pFunc, const UInt64 _argument)
{
   const auto* argument = reinterpret_cast<SyscallMdl::HookSyscallExit*>(_argument);
   const SubsecondTime time(argument->time);
   PyObject *pResult = HooksPy::callPythonFunction((PyObject *)pFunc, Py_BuildValue("(iiLiO)", argument->thread_id, argument->core_id, time.getFS(),
      argument->ret_val, argument->emulated ? Py_True : Py_False));
   return hookCallbackResult(pResult);
}

static PyObject *
registerHook(PyObject *self, PyObject *args)
{
   int hook = -1;
   PyObject *pFunc = nullptr;

   if (!PyArg_ParseTuple(args, "lO", &hook, &pFunc))
      return nullptr;

   if (hook < 0 || hook >= HookType::HOOK_TYPES_MAX) {
      PyErr_SetString(PyExc_ValueError, "Hook type out of range");
      return nullptr;
   }
   if (!PyCallable_Check(pFunc)) {
      PyErr_SetString(PyExc_TypeError, "Second argument must be callable");
      return nullptr;
   }

   Py_INCREF(pFunc);

   switch(const auto type = HookType::hook_type_t(hook)) {
      case HookType::HOOK_PERIODIC:
      case HookType::HOOK_EPOCH_TIMEOUT:     // Added by Kleber Kruger
         Sim()->getHooksManager()->registerHook(type, hookCallbackSubsecondTime, (UInt64)pFunc);
         break;
      case HookType::HOOK_SIM_START:
      case HookType::HOOK_SIM_END:
      case HookType::HOOK_ROI_BEGIN:
      case HookType::HOOK_ROI_END:
      case HookType::HOOK_APPLICATION_ROI_BEGIN:
      case HookType::HOOK_APPLICATION_ROI_END:
      case HookType::HOOK_SIGUSR1:
         Sim()->getHooksManager()->registerHook(type, hookCallbackNone, (UInt64)pFunc);
         break;
      case HookType::HOOK_PERIODIC_INS:
      case HookType::HOOK_CPUFREQ_CHANGE:
      case HookType::HOOK_INSTR_COUNT:
      case HookType::HOOK_INSTRUMENT_MODE:
      case HookType::HOOK_APPLICATION_START:
      case HookType::HOOK_APPLICATION_EXIT:
      case HookType::HOOK_EPOCH_START:       // Added by Kleber Kruger
      case HookType::HOOK_EPOCH_END:         // Added by Kleber Kruger
      case HookType::HOOK_EPOCH_PERSISTED:   // Added by Kleber Kruger
      case HookType::HOOK_EPOCH_TIMEOUT_INS: // Added by Kleber Kruger
         Sim()->getHooksManager()->registerHook(type, hookCallbackInt, (UInt64)pFunc);
         break;
      case HookType::HOOK_PRE_STAT_WRITE:
         Sim()->getHooksManager()->registerHook(type, hookCallbackString, (UInt64)pFunc);
         break;
      case HookType::HOOK_MAGIC_MARKER:
      case HookType::HOOK_MAGIC_USER:
         Sim()->getHooksManager()->registerHook(type, hookCallbackMagicMarkerType, (UInt64)pFunc);
         break;
      case HookType::HOOK_THREAD_CREATE:
         Sim()->getHooksManager()->registerHook(type, hookCallbackThreadCreateType, (UInt64)pFunc);
         break;
      case HookType::HOOK_THREAD_START:
      case HookType::HOOK_THREAD_EXIT:
         Sim()->getHooksManager()->registerHook(type, hookCallbackThreadTimeType, (UInt64)pFunc);
         break;
      case HookType::HOOK_THREAD_STALL:
         Sim()->getHooksManager()->registerHook(type, hookCallbackThreadStallType, (UInt64)pFunc);
         break;
      case HookType::HOOK_THREAD_RESUME:
         Sim()->getHooksManager()->registerHook(type, hookCallbackThreadResumeType, (UInt64)pFunc);
         break;
      case HookType::HOOK_THREAD_MIGRATE:
         Sim()->getHooksManager()->registerHook(type, hookCallbackThreadMigrateType, (UInt64)pFunc);
         break;
      case HookType::HOOK_SYSCALL_ENTER:
         Sim()->getHooksManager()->registerHook(type, hookCallbackSyscallEnter, (UInt64)pFunc);
         break;
      case HookType::HOOK_SYSCALL_EXIT:
         Sim()->getHooksManager()->registerHook(type, hookCallbackSyscallExit, (UInt64)pFunc);
         break;
      case HookType::HOOK_TYPES_MAX:
         assert(0);
   }

   Py_RETURN_NONE;
}

static PyObject *
triggerHookMagicUser(PyObject *self, PyObject *args)
{
   UInt64 a, b;

   if (!PyArg_ParseTuple(args, "ll", &a, &b))
      return nullptr;

   const UInt64 res = Sim()->getMagicServer()->Magic_unlocked(INVALID_THREAD_ID, INVALID_CORE_ID, SIM_CMD_USER, a, b);

   return PyInt_FromLong(static_cast<SInt64>(res));
}

static PyMethodDef PyHooksMethods[] = {
   {"register",  registerHook, METH_VARARGS, "Register callback function to a Sniper hook."},
   {"trigger_magic_user", triggerHookMagicUser, METH_VARARGS, "Trigger HOOK_MAGIC_USER hook."},
   {nullptr, nullptr, 0, nullptr} /* Sentinel */
};

void HooksPy::PyHooks::setup()
{
   PyObject *pModule = Py_InitModule("sim_hooks", PyHooksMethods);
   PyObject *pHooks = PyDict_New();
   PyObject_SetAttrString(pModule, "hooks", pHooks);

   for(int i = 0; i < static_cast<int>(HookType::HOOK_TYPES_MAX); ++i) {
      PyObject *pGlobalConst = PyInt_FromLong(i);
      PyObject_SetAttrString(pModule, HookType::hook_type_names[i], pGlobalConst);
      PyDict_SetItemString(pHooks, HookType::hook_type_names[i], pGlobalConst);
      Py_DECREF(pGlobalConst);
   }
   Py_DECREF(pHooks);
}