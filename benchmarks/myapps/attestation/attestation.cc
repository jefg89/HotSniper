#include <openssl/aes.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <random>
#include <chrono>
#include "sim_api.h"

using namespace std;

uint64_t result;

// Initialization method
// Entry point for the program
void main_setup() {
    // initDev();
    // initADC();
    // setInts();
    // setKey(seed);
    cout << "Entering setup" << endl;
}


bool simCheckAttestation();
bool simCheckAttestationTurn();
uint8_t simGetChallenge();
void computeChallenge(uint8_t challenge);
void simSendChallengeResult(uint64_t result);
void enableHashComputation(uint8_t flag);

int main_loop() {
    cout <<"Entering main loop" <<endl;
    // float value = 0.f;
    // float filtered;
    // float encrypted;

    // "Infinite" loop
    //while (true) {
        // ADDED CODE HERE
        // Polls for attestation request by Verifier (simulator).
        // This has the same return value for all applications (true/false) -> simultaneous attestation
        bool attestation = simCheckAttestation();
        bool myTurn = false;
        if (attestation) {
            cout << "Starting statestation" <<endl;
            while (!myTurn) 
                // Wait until it's my turn on the queue
                myTurn = simCheckAttestationTurn(); //need id?
            cout << "It's my turn" <<endl;
            // If it's my turn, get the challenge
            uint8_t challenge = simGetChallenge(); //need id?
            cout << "Got challenge " << challenge << endl;
            // Compute Challenge
            computeChallenge(challenge);
            // Send the answer back to the Verifier (simulator).
        }
        // END of ADDED CODE

        processing_:
            // value = readADC();
            // filtered = FIR_filter();
            // encrypted = AES_encrypt(filtered, KEY);
            // send(encrypted, time);
            cout << "Doing normal processing" <<endl;
            if (attestation & myTurn) {
                simSendChallengeResult(result); //need id?
                myTurn = false;
                attestation = false;
            }
    //}
    return 1;
} 


// This method sets global  flags to enable
// different hash computations
void computeChallenge(uint8_t challenge) {
    cout <<"Enabling proper flags for challenge computing"<<endl;
    switch(challenge) {
        case 1:
            enableHashComputation(1);
        case 2:
            enableHashComputation(2);
        case 255:
            enableHashComputation(3);
    }
}







bool simCheckAttestation(){
    cout << "Check Attestation Called" <<endl;
    return true;
}
bool simCheckAttestationTurn(){
    cout << "Check Turn Called "<< endl;
    return true;
}
uint8_t simGetChallenge(){
    cout<< "Get Challenge Called "<< endl;
    return 2;
}
void simSendChallengeResult(uint64_t result) {
    cout <<"Send Challenge Called with " << result << endl;
}
void enableHashComputation(uint8_t flag) {
    cout << "Enable Hash Called with" << flag <<endl;
}





int main(int argc, char* argv[]){
    main_setup();
    main_loop();
}