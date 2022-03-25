#include "attestAll.h"

AttestAll::AttestAll(const PerformanceCounters * performanceCounters)
    : m_performance_counters(performanceCounters){
}

std::vector<int> AttestAll::getCandidates(const std::vector<bool> &activeCores) {
    std::vector<int> candidates;
    for (size_t i = 0; i < activeCores.size(); i++){
        if (activeCores.at(i))
            candidates.push_back(i);
    }
    return candidates;
}