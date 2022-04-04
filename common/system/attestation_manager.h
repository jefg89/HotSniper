/**
 * This header implements the Attestation  Manager class 
 */

#ifndef _ATTESTATION_MANAGER_H
#define _ATTESTATION_MANAGER_H

#include "fixed_types.h"
#include "trusted_hw_platform.h"
#include "dev_under_attestation.h"
#include "subsecond_time.h"
//#include "simulator.h"
//#include "thread.h"
//#include "thread_manager.h"
#include <vector>
#include <queue>
#include <random>


#define MSW 1
#define LSW 0
 

using namespace std; 



class AttestationManager {
public:
    AttestationManager();
    virtual ~AttestationManager();

    void setAttestation(thread_id_t thread_id);
    UInt64 getChallengeHash(thread_id_t thread_id, UInt8 word);
    UInt16 requestTurn(thread_id_t thread_id);
    bool checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result);
    bool checkUnderAttestation(thread_id_t thread_id);
    bool checkAttestationTurn(UInt16 ticket);
    bool checkAllFinished();
    void updateSequencer();
    bool checkUnderAttestationGlobal();
    

    
private:
    TrustedHwPlatform * trustedHwPlatform;
    UInt128 computeChallengeHash();
    UInt16 computeChallengeId();
    void unsetAttestation(thread_id_t thread_id);
    vector<thread_id_t> m_curr_attest_threads;
    queue<thread_id_t> m_curr_attest_turn;
    UInt16 TICKETS =  2000;
//     ComponentLatency *seq_latency;
//     ComponentPeriod  seq_period = ComponentPeriod::fromFreqHz(1000000000);
//     //const ComponentPeriod *seq_period_ptr =&seq_period;    
};

#endif
