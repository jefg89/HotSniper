#ifndef __ATTEST_ALL_H
#define __ATTEST_ALL_H

#include "attestationpolicy.h"

class AttestAll : public AttestationPolicy {
public:
    AttestAll(const PerformanceCounters *performanceCounters);
    virtual std::vector<int> getCandidates(const std::vector<bool> &activeCores);

private:
    const PerformanceCounters *m_performance_counters;

};

#endif