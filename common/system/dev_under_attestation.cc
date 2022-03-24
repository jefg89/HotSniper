#include "dev_under_attestation.h"
#include <iostream>
#include  <iomanip>

using namespace std;

DevUnderAttestation::DevUnderAttestation(thread_id_t thread_id) {
    m_thread_id = thread_id; 
    //TODO: initialize stuff here
}
DevUnderAttestation::~DevUnderAttestation() {
    //TODO: free-up some memory or something
}

void DevUnderAttestation::printHash() {
    long int MSB = m_challenge_hash >> 64;
    long int LSB = ((m_challenge_hash << 64) >> 64);
    cout<<"New Hash = 0x" << setfill('0') << setw(16) << right << hex << MSB << setfill('0') << setw(16) << right << hex << LSB <<endl;
}

bool DevUnderAttestation::verifyChallenge(UInt128 challenge_result) {
    //TODO: perform some calculations here to verify that the input challenge result
    //corresponds with the stored hash and challege id.
    //This returns true for now
    
    long int MSB = challenge_result >> 64;
    long int LSB = ((challenge_result << 64) >> 64);
    cout<<"Received Hash = 0x" << setfill('0') << setw(16) << right << hex << MSB << setfill('0') << setw(16) << right << hex << LSB <<endl;
    //Do not forget to clean stuff
    m_challenge_hash = 0;
    m_challenge_id = 0;
    return true;
}