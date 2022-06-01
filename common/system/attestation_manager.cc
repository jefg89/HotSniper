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
    //cout <<"[Attestation Manager]: Setting Attestation for Thread " << thread_id <<endl;
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
    if (!checkUnderAttestation(thread_id))
        setAttestation(thread_id);
    
    cout <<"[Attestation Manager]: Assigning ticket " <<m_curr_tickets<< " to Thread " << thread_id <<endl;
    m_curr_attest_turn.push_back(std::make_pair(thread_id, m_curr_tickets));
    m_curr_tickets--;
    return m_curr_tickets + 1;
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
    return (m_curr_attest_turn.front().second == ticket);
}

void AttestationManager::unsetAttestation(thread_id_t thread_id) {
    for (auto iter = m_curr_attest_threads.begin()  ; iter != m_curr_attest_threads.end() ; ){
        if (*iter == thread_id) {
            iter = m_curr_attest_threads.erase(iter);
            }
        else
            ++iter;
    }
}

bool AttestationManager::checkAllFinished() {
    return m_curr_attest_turn.empty() && m_curr_sw.empty();
}
void AttestationManager::updateSequencer(SubsecondTime time) {
    if (!m_curr_attest_turn.empty()) {
        cout << "[SEQUENCER] @" << (float) time.getNS()/1000 <<"us: "  <<"Serving ticket "  <<m_curr_attest_turn.front().second << " owned by thread " <<m_curr_attest_turn.front().first <<endl;
        m_curr_attest_turn.pop_front();
        m_curr_tickets++;
    }
}

bool AttestationManager::checkUnderAttestationGlobal() {
    return (!m_curr_attest_threads.empty());
}

void AttestationManager::setFinishedSW(thread_id_t thread_id) {
    m_curr_sw.pop();
}

void AttestationManager::setAttestationSW(thread_id_t thread_id) {
    m_curr_sw.push(thread_id);
}
UInt8 AttestationManager::getElementsInQueue() {
    return  m_curr_attest_turn.size();
}

bool AttestationManager::checkInQueue(thread_id_t thread_id) {
    for (auto& p : m_curr_attest_turn) {
        if (p.first == thread_id)
            return true;
    }
    return false;
}

void AttestationManager::setInitialElementsInQueue(UInt16 elements) {
    m_curr_tickets = elements;
    MAX_TICKETS = elements;
}

bool AttestationManager::getAttestationMode() {
    return (getElementsInQueue() < MAX_TICKETS);   
}