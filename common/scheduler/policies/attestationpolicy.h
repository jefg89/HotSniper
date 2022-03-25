/**
 * This header implements the attestation policy interface.
 * This policy is responsable for selecting which cores(threads) 
 * should be attestated
 */

#ifndef __ATTESTPOLICY_H
#define __ATTESTPOLICY_H

#include <vector>
#include "performance_counters.h"

class AttestationPolicy {
public:
    virtual ~AttestationPolicy() {}
    //virtual std::vector<int> getFrequencies(const std::vector<int> &oldFrequencies, const std::vector<bool> &activeCores) = 0;
    virtual std::vector<int> getCandidates(const std::vector<bool> &activeCores) = 0;
};

#endif