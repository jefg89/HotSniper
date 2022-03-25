#include "attestation_manager.h"
#include <algorithm>

using namespace std;


AttestationManager::AttestationManager() {
    //TODO: initialize stuff here
}
AttestationManager::~AttestationManager() {
    for (size_t i = 0; i < m_devices.size(); i++) 
        free(m_devices.at(i));
}

void AttestationManager::setAttestation(thread_id_t thread_id){
    cout <<"[Attestation Manager]: Setting Attestation for Thread " << thread_id <<endl;
    DevUnderAttestation * curr_device = getDevicebyThreadId(thread_id);
    if (!curr_device) {
        curr_device = new DevUnderAttestation(thread_id);
        m_devices.push_back(curr_device);
        m_fifo.push(curr_device);
    }
}

// This method needs to be called twice for a single Hash
// due to limitations on the Simulator return size
// the first time we return the Most Significant Word (MSW)
// the second time we return the Least Significicant Word (LSW)
UInt64 AttestationManager::getChallengeHash(thread_id_t thread_id){
    DevUnderAttestation * curr_device = getDevicebyThreadId(thread_id);
    if (!curr_device) {
        curr_device = new DevUnderAttestation(thread_id);
        m_devices.push_back(curr_device);
    }
    //If this is the first time we get called
    if(!curr_device->getChallengeHash()) {
        // compute and store a new hash
        curr_device->setChallengeHash(computeChallengeHash());
        curr_device->printHash();
        //return MSW
        return curr_device->getChallengeHash() >> 64;
    }
    //if this is the second time we get called
    //return LSW
    return (curr_device->getChallengeHash() << 64) >> 64;
}

UInt16  AttestationManager::getChallengeId(thread_id_t thread_id){
    DevUnderAttestation * curr_device = getDevicebyThreadId(thread_id);
    if (!curr_device) {
        curr_device = new DevUnderAttestation(thread_id);
        m_devices.push_back(curr_device);
    }
    curr_device->setChallengeId(computeChallengeId());
    curr_device->printChallengeId();
    return curr_device->getChallengeId();
}

bool AttestationManager::checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result) {
    DevUnderAttestation * curr_device = getDevicebyThreadId(thread_id);
    // The verification is done with by the DUA module itself
    // let's return what they say
    if (curr_device->verifyChallenge(challenge_result)) {
        unsetAttestation(thread_id);
        m_fifo.pop();
        return true;
    }
    return false;

}

bool AttestationManager::checkUnderAttestation(thread_id_t thread_id) {
    DevUnderAttestation * curr_device = getDevicebyThreadId(thread_id);
    if (curr_device)
        return true;
    return false;
}

bool AttestationManager::checkAttestationTurn(thread_id_t thread_id) {
    return (m_fifo.front()->m_thread_id == thread_id);
}

// Private methods
DevUnderAttestation *  AttestationManager::getDevicebyThreadId(thread_id_t thread_id) {
    for (size_t i = 0; i < m_devices.size(); i++) {
        if (m_devices.at(i)->m_thread_id == thread_id)
            return m_devices.at(i);
    }
    return NULL;
    
}
UInt128 AttestationManager::computeChallengeHash() {
    std::uniform_int_distribution<> distrib;
    __int128_t out = (static_cast<__int128_t>(distrib(gentr)) << 96) | (static_cast<__int128_t>(distrib(gentr)) << 64) |
                     (static_cast<__int128_t>(distrib(gentr)) << 32) | (static_cast<__int128_t>(distrib(gentr)));
    return out;
}

UInt16 AttestationManager::computeChallengeId() {
    std::uniform_int_distribution<> uchar(0, 2);
    return uchar(gentr);
}

void AttestationManager::unsetAttestation(thread_id_t thread_id) {
    for (size_t i = 0; i < m_devices.size(); i++) {
        if (m_devices.at(i)->m_thread_id == thread_id)
            m_devices.at(i)->m_marked_for_delete = true;
    }
              
    m_devices.erase(std::remove_if(m_devices.begin(), m_devices.end(), 
                    [] (DevUnderAttestation * t) {return t->m_marked_for_delete;}),
                    m_devices.end());
}