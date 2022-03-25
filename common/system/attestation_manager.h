/**
 * This header implements the Attestation  Manager class 
 */

#ifndef _ATTESTATION_MANAGER_H
#define _ATTESTATION_MANAGER_H

#include "fixed_types.h"
#include "dev_under_attestation.h"
#include "simulator.h"
#include "thread.h"
#include "thread_manager.h"
#include <vector>
#include <queue>
#include <random>
#include <list>

using namespace std; 



class AttestationManager {
public:
    AttestationManager();
    virtual ~AttestationManager();

    void setAttestation(thread_id_t thread_id);
    UInt64 getChallengeHash(thread_id_t thread_id);
    UInt16 getChallengeId(thread_id_t thread_id);
    bool checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result);
    bool checkUnderAttestation(thread_id_t thread_id);
    bool checkAttestationTurn(thread_id_t thread_id);
    

    
private:
    std::mt19937 gentr;
    vector<DevUnderAttestation*> m_devices;
    DevUnderAttestation * getDevicebyThreadId(thread_id_t);
    UInt128 computeChallengeHash();
    UInt16 computeChallengeId();
    void unsetAttestation(thread_id_t thread_id);
    queue<DevUnderAttestation *> m_fifo;    
};

#endif
