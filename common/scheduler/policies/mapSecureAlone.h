/**
 * This header implements the "map to specific tile" policy
 */

#ifndef __MAP_SECURE_ALONE_H
#define __MAP_SECURE_ALONE_H

#include "mappingpolicy.h"

class MapSecureAlone: public MappingPolicy {
public:
    MapSecureAlone(unsigned int numTasks, unsigned int coreRows, unsigned int coreColumns, std::vector<int> preferredTile);
    virtual std::vector<int> map(UInt32 taskID, int taskCoreRequirement, const std::vector<bool> &availableCores, const std::vector<bool> &activeCores);

private:
    unsigned int m_core_rows;
    unsigned int m_core_columns;
    std::vector<int> m_preferred_tile;
    std::vector<std::vector<int>> m_preferred_cores_order;

};

#endif