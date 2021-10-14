#include "mapSecureAlone.h"
#include "scheduler_open.h"
#include "fixed_types.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>

using namespace std;

MapSecureAlone::MapSecureAlone(unsigned int numTasks, unsigned int coreRows, unsigned int coreColumns, std::vector<int> preferredTile)
	: m_core_rows(coreRows), m_core_columns(coreColumns), m_preferred_tile(preferredTile) {
	int coresPerTile = Sim()->getConfig()->getCoresPerTile();
    std::vector<int> cores0;
    for (int core = 0; core < coresPerTile; core++) 
                cores0.push_back(m_preferred_tile.at(0) * coresPerTile + core);
            m_preferred_cores_order.push_back(cores0);

    for (size_t i = 1; i < numTasks; i++)
    {
        // If we have a preferred tile, add those cores
        if (m_preferred_tile.at(i) > 0) {
            std::vector<int> cores;
            for (int core = 0; core < coresPerTile; core++) 
                cores.push_back(m_preferred_tile.at(i) * coresPerTile + core);
            m_preferred_cores_order.push_back(cores);
        }
        //Otherwise add all cores except the ones on tile 0
        else {
            std::vector<int> cores;
            for (size_t j = coresPerTile; j < coreRows * coreColumns; j++) 
                cores.push_back(j);
            m_preferred_cores_order.push_back(cores);
        }
    }   
}

std::vector<int> MapSecureAlone::map(UInt32 taskID, int taskCoreRequirement, const std::vector<bool> &availableCores, const std::vector<bool> &activeCores) {
	std::vector<int> cores;

	// try to fill with preferred cores
	for (const int &c : m_preferred_cores_order.at(taskID)) {
		if (availableCores.at(c)) {
			cores.push_back(c);
			if ((int)cores.size() == taskCoreRequirement) {
				return cores;
			}
		}
	}

	std::vector<int> empty;
	return empty;
}