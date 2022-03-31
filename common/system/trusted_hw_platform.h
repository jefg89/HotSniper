/**
 * This header implements the Trusted Harware Plaftorm model class 
 */

#ifndef _TRUSTED_HW_PLATFORM_H
#define _TRUSTED_HW_PLATFORM_H

#include "fixed_types.h"
#include "dev_under_attestation.h"
#include <vector>
#include <random>


using namespace std; 



class TrustedHwPlatform {
public:
    TrustedHwPlatform();
    virtual ~TrustedHwPlatform();

    UInt128 getChallengeHash(thread_id_t thread_id);
    UInt16 getChallengeId(thread_id_t thread_id);
    bool checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result);
    
    
private:
    std::mt19937 gentr;
    UInt128 computeChallengeHash();
    UInt16 computeChallengeId();
    std::vector<DevUnderAttestation *> m_devices_table;
    DevUnderAttestation *  getDevicebyThreadId(thread_id_t thread_id);
    
};






#endif
