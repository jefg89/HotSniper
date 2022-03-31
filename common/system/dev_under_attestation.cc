#include "dev_under_attestation.h"
#include <iostream>
#include  <iomanip>

using namespace std;

DevUnderAttestation::DevUnderAttestation(thread_id_t thread_id) {
    m_thread_id = thread_id; 
    m_challenge_hash = 0;
    //TODO: initialize stuff here
}
DevUnderAttestation::~DevUnderAttestation() {
    //TODO: free-up some memory or something
}

void DevUnderAttestation::printHash() {
    long int MSB = m_challenge_hash >> 64;
    long int LSB = ((m_challenge_hash << 64) >> 64);
     cout<<"[Trusted HW Platform] [Thread/"<< m_thread_id <<"]:" <<" Seed Hash = 0x" << setfill('0') << setw(16) << right << hex << MSB << setfill('0') << setw(16) << right << hex << LSB <<endl;
}

void DevUnderAttestation::printChallengeId() {
    cout<<"[Trusted HW Platform] [Thread/"<< m_thread_id <<"]:" << " Challenge ID = 0x" << m_challenge_id <<endl;
}