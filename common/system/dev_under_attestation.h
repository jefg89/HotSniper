#ifndef _DEV_UNDER_ATTESTATION_H
#define _DEV_UNDER_ATTESTATION_H


#include "fixed_types.h"


class DevUnderAttestation{
public:
    DevUnderAttestation(thread_id_t thread_id);
    ~DevUnderAttestation();
    thread_id_t m_thread_id;
    void setChallengeId(UInt16 challenge_id)  { m_challenge_id = challenge_id; }
    void setChallengeHash(UInt128 challenge_hash) { m_challenge_hash = challenge_hash; }
    UInt16 getChallengeId() const {return m_challenge_id; }
    UInt128 getChallengeHash() const {return m_challenge_hash; }
    bool verifyChallenge(UInt128 challenge_result);
    void printHash();
    bool m_marked_for_delete = false;

private:
    
    UInt128 m_challenge_hash = 0;
    UInt16 m_challenge_id = 0;
};
#endif