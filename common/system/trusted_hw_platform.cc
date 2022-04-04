#include "trusted_hw_platform.h"
#include <algorithm>
#include <iostream>

using namespace std;


TrustedHwPlatform::TrustedHwPlatform() {
}
TrustedHwPlatform::~TrustedHwPlatform() {
    for (size_t i = 0; i < m_devices_table.size(); i++) 
        delete(m_devices_table.at(i));
}

UInt128 TrustedHwPlatform::getChallengeHash(thread_id_t thread_id){
//     DevUnderAttestation * curr_dev = getDevicebyThreadId(thread_id);
//     UInt128 challengeHash;
//     if (curr_dev) {
//         if (curr_dev->getChallengeHash()!= 0)
//             return curr_dev->getChallengeHash();
//         else {
//             challengeHash = computeChallengeHash();
//             curr_dev->setChallengeHash(challengeHash);
//             curr_dev->printHash();
//             return challengeHash;
//         }
//     }
//     else {
//         curr_dev = new  DevUnderAttestation(thread_id);
//         challengeHash = computeChallengeHash();
//         curr_dev->setChallengeHash(challengeHash);
//         curr_dev->printHash();
//         m_devices_table.push_back(curr_dev);
//         return challengeHash;
//     }
    return computeChallengeHash();
}

UInt16  TrustedHwPlatform::getChallengeId(thread_id_t thread_id){
    DevUnderAttestation * curr_dev = getDevicebyThreadId(thread_id);
    if (curr_dev) 
        return curr_dev->getChallengeId();
    
    curr_dev = new  DevUnderAttestation(thread_id);
    UInt16 challengeId = computeChallengeId();
    curr_dev->setChallengeId(challengeId);
    curr_dev->printChallengeId();
    m_devices_table.push_back(curr_dev);
    return challengeId;
}

bool TrustedHwPlatform::checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result) {
    cout << "[Trusted HW Platform] [Thread/" << thread_id <<"]: Checking challenge result" <<endl;
    for (auto iter = m_devices_table.begin()  ; iter != m_devices_table.end() ; ){
        if ((*iter)->m_thread_id == thread_id)
            iter = m_devices_table.erase(iter);
        else
            ++iter;
    }
    //TODO: Just returns true for now
    return true;
}

//Generates random 128 bit hash
UInt128 TrustedHwPlatform::computeChallengeHash() {
    std::uniform_int_distribution<> distrib;
    __int128_t out = (static_cast<__int128_t>(distrib(gentr)) << 96) | (static_cast<__int128_t>(distrib(gentr)) << 64) |
                     (static_cast<__int128_t>(distrib(gentr)) << 32) | (static_cast<__int128_t>(distrib(gentr)));
    return out;
}

//Generates random 16 bit ID
UInt16 TrustedHwPlatform::computeChallengeId() {
    std::uniform_int_distribution<> uchar(0, 2);
    return uchar(gentr);
}

DevUnderAttestation *  TrustedHwPlatform::getDevicebyThreadId(thread_id_t thread_id) {
    for (size_t i = 0; i < m_devices_table.size(); i++) {
        if ((m_devices_table.at(i)->m_thread_id) == thread_id) {
            return m_devices_table.at(i);  
        }      
    }
    return NULL;
    
}