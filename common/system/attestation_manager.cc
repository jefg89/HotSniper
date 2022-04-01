#include "attestation_manager.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <list>

using namespace std;

#define MSW 1
#define LSW 0

AttestationManager::AttestationManager() {
    trustedHwPlatform = new TrustedHwPlatform;
}
AttestationManager::~AttestationManager() {
    delete(trustedHwPlatform);
}

void AttestationManager::setAttestation(thread_id_t thread_id){
    cout <<"[Attestation Manager]: Setting Attestation for Thread " << thread_id <<endl;
    m_curr_attest_threads.push_back(thread_id);
    m_curr_attest_turn.push(thread_id);
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
    // After getting the hash, we unset the flag for this thread
    // as the attestation verification will be done asynchronously
    // by the hardware platform
    unsetAttestation(thread_id);
    // remove the thread for the current turn queue;
    m_curr_attest_turn.pop();
    return returnHash;
}

UInt16  AttestationManager::getChallengeId(thread_id_t thread_id){
    return trustedHwPlatform->getChallengeId(thread_id);
}

bool AttestationManager::checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result) {
    // The verification is done by the Trusted Hardware Platform.
    return trustedHwPlatform->checkChallengeResult(thread_id, challenge_result);

}

bool AttestationManager::checkUnderAttestation(thread_id_t thread_id) {
    for (size_t i = 0; i < m_curr_attest_threads.size(); i++){
        if (m_curr_attest_threads.at(i) == thread_id)
            return true;
    }
    return false;
}

bool AttestationManager::checkAttestationTurn(thread_id_t thread_id) {
    return (m_curr_attest_turn.front() == thread_id);
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
    cout << "Currently on top " <<std::dec <<m_curr_attest_turn.front() << "empty? = " << m_curr_attest_turn.empty() <<endl;
    return m_curr_attest_turn.empty();
}

// void AttestationManager::sequencer() {
//     for (size_t i = 0; i < m_curr_attest_threads.size(); i++){
//         trustedHwPlatform->getChallengeId(m_curr_attest_threads.at(i));
//         trustedHwPlatform->getChallengeHash(m_curr_attest_threads.at(i));
//     }
    
// }