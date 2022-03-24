#include "attestation_manager.h"
//TODO: remove
#include <iostream>


using namespace std;


AttestationManager::AttestationManager() {
    //TODO: initialize stuff here
}
AttestationManager::~AttestationManager() {
    //TODO: free-up some memory or something
}



void AttestationManager::setAttestation(thread_id_t thread_id){
    m_under_attestation.at(thread_id) = true;
    Sim()->getThreadManager()->getThreadFromID(thread_id)->setUnderAttestation();
}

UInt128 AttestationManager::getChallengeHash(){
    std::uniform_int_distribution<> distrib;
    __int128_t out = (static_cast<__int128_t>(distrib(gentr)) << 96) | (static_cast<__int128_t>(distrib(gentr)) << 64) |
                     (static_cast<__int128_t>(distrib(gentr)) << 32) | (static_cast<__int128_t>(distrib(gentr)));
    return out;
}
UInt16  AttestationManager::getChallengeID(){
    std::uniform_int_distribution<> uchar(0, 2);
    return uchar(gentr);
}