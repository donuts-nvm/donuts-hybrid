#ifndef EPOCH_MANAGER_H
#define EPOCH_MANAGER_H

#include "fixed_types.h"
#include "epoch_cntlr.h"

class EpochManager {
public:

  static UInt64 getGlobalSystemEID() { return 0; }

  [[nodiscard]] EpochCntlr *getEpochCntlr(core_id_t core_id) const { return nullptr; }
};



#endif //EPOCH_MANAGER_H
