#include "attestation_manager.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <list>
#include <unistd.h>



using namespace std;



AttestationManager::AttestationManager() {
    trustedHwPlatform = new TrustedHwPlatform;
    //TODO: read from file
    // seq_latency = new ComponentLatency(&seq_period, 1); //50000
}
AttestationManager::~AttestationManager() {
    delete(trustedHwPlatform);
}

void AttestationManager::setAttestation(thread_id_t thread_id){
    cout <<"[Attestation Manager]: Setting Attestation for Thread " << thread_id <<endl;
    m_curr_attest_threads.push_back(thread_id);
}

// This method needs to be called twice for a single Hash
// due to limitations on the Simulator return size.
// The word parameter determines which word to return
// 0 for Least Significant Word (LSW)
// 1 for Most Sifnigicant Word (MSW)
UInt64 AttestationManager::getChallengeHash(thread_id_t thread_id, UInt8 word){
    UInt128 challengeHash = trustedHwPlatform->getChallengeHash(thread_id);
    UInt64 returnHash;
    switch (word){
        case LSW:
            returnHash = (challengeHash << 64) >> 64;
            break;
        case MSW:
            returnHash = challengeHash >> 64;
            break;
        default:
            returnHash = (challengeHash << 64) >> 64;
            break;
    }

    return returnHash;
}

UInt16  AttestationManager::requestTurn(thread_id_t thread_id){
    TICKETS--;
    m_curr_attest_turn.push(static_cast<thread_id_t>(TICKETS));
    return TICKETS;
}

bool AttestationManager::checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result) {
    // TODO: The verification should be done by the magical "verifier"
    // through the trusted platform
    unsetAttestation(thread_id);
    return trustedHwPlatform->checkChallengeResult(thread_id, challenge_result);

}

bool AttestationManager::checkUnderAttestation(thread_id_t thread_id) {
    for (size_t i = 0; i < m_curr_attest_threads.size(); i++){
        if (m_curr_attest_threads.at(i) == thread_id)
            return true;
    }
    return false;
}

bool AttestationManager::checkAttestationTurn(UInt16 ticket) {
    return (m_curr_attest_turn.front() == ticket);
}

void AttestationManager::unsetAttestation(thread_id_t thread_id) {
    for (auto iter = m_curr_attest_threads.begin()  ; iter != m_curr_attest_threads.end() ; ){
        if (*iter == thread_id)
            iter = m_curr_attest_threads.erase(iter);
        else
            ++iter;
    }
}

bool AttestationManager::checkAllFinished() {
    return m_curr_attest_turn.empty();
}
void AttestationManager::updateSequencer() {
    if (!m_curr_attest_turn.empty()) {
        cout << "[SEQUENCER]: Serving ticket " << std::dec <<m_curr_attest_turn.front() <<endl;
        m_curr_attest_turn.pop();
    }
}

bool AttestationManager::checkUnderAttestationGlobal() {
    return (!m_curr_attest_threads.empty());
}