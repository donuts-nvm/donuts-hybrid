#ifndef CACHE_STATE_H
#define CACHE_STATE_H

#include <cassert>

#include "fixed_types.h"

class CacheState
{
   public:
      enum cstate_t
      {
         CSTATE_FIRST = 0,
         INVALID = CSTATE_FIRST,
         SHARED,
         SHARED_UPGRADING,
         EXCLUSIVE,
         OWNED,
         MODIFIED,
         NUM_CSTATE_STATES,
         /* Below are special states, used only for reporting */
         INVALID_COLD = NUM_CSTATE_STATES,
         INVALID_EVICT,
         INVALID_COHERENCY,
         NUM_CSTATE_SPECIAL_STATES
      };

      explicit CacheState(const cstate_t state = INVALID) : cstate(state) {}
      ~CacheState() = default;

      [[nodiscard]] bool readable() const
      {
         return (cstate == MODIFIED) || (cstate == OWNED) || (cstate == SHARED) || (cstate == EXCLUSIVE);
      }

      [[nodiscard]] bool writable() const
      {
         return (cstate == MODIFIED);
      }

      [[nodiscard]] char to_char() const // Added by Kleber Kruger
      {
         switch(cstate)
         {
            case INVALID:           return 'I';
            case SHARED:            return 'S';
            case SHARED_UPGRADING:  return 'u';
            case MODIFIED:          return 'M';
            case EXCLUSIVE:         return 'E';
            case OWNED:             return 'O';
            case INVALID_COLD:      return '_';
            case INVALID_EVICT:     return 'e';
            case INVALID_COHERENCY: return 'c';
            default:                return '?';
         }
      }

   private:
      cstate_t cstate;

};

#endif /* CACHE_STATE_H */