#include <openssl/aes.h>
#include <iostream>
#include  <iomanip>
#include <fstream>
#include <stdlib.h>
#include <random>
#include <chrono>
#include "sim_api.h"

#define DEBUG 1
#define SAMPLES 1024


using namespace std;

std::random_device rd;  
std::mt19937 gen(rd());
std::vector<bool> attestation_flags = {false, false, false};
__uint128_t hash_seed;



void readADC(uint16_t * input);

void FIRFilter(uint16_t * input, float * output);


//bool simCheckAttestation();
//bool SimCheckAttestationTurn();
//uint16_t SimGetChallengeId();
//__uint128_t simGetChallengeSeed();
void computeChallenge(uint16_t challenge);
void simSendChallengeResult(__uint128_t result);
void enableHashComputation(uint16_t flag);
void disableAllFlags(); 

void debugPrintHash(const char * module,__uint128_t hash);

__attribute__ ((__noinline__))
void * getPC () { return __builtin_return_address(0); }


// Initialization method
// Entry point for the program
void mainSetup() {
    // initDev();
    // initADC();
    // setInts();
    // setKey(seed);
    cout << "Entering setup" << endl;
}

int mainLoop(int iter) {
    SimRoiStart();
    cout <<"Entering main loop for " <<iter << " iterations" <<endl;
    // float filtered;
    // float encrypted;

    // "Infinite" loop
    //while (true) {
    for (size_t i = 0; i < iter; i++){
        // ADDED CODE HERE
        // Polls for attestation request by Verifier (simulator).
        // This has the same return value for all applications (true/false) -> simultaneous attestation
        bool attestation = SimCheckAttestation();
        bool myTurn = false;
        if (attestation) {
            cout << "Starting atatestation" <<endl;
            while (!myTurn) 
                // Wait until it's my turn on the queue
                myTurn = SimCheckAttestationTurn(); //need id?
            cout << "It's my turn" <<endl;
            // If it's my turn, get the challenge
            uint16_t challenge = SimGetChallengeId(); //need id?
            // Now get the challenge's hash
            hash_seed = SimGetChallengeHashMSW(); // Most Significant Word
            hash_seed = hash_seed << 64 | SimGetChallengeHashLSW(); //Least Significant Word
            debugPrintHash("MAIN", hash_seed);
            cout<< "Got challenge "<< challenge << endl;
            // Compute Challenge
            computeChallenge(challenge);
      
        }
        // END of ADDED CODE

        processing_:
            cout << "Doing normal processing" <<endl;

            uint16_t inputs [SAMPLES];
            float outputs [SAMPLES];
            // Reads N samples from ADC 
            readADC(inputs);
            // Applies Low-Pass FIR Filter
            FIRFilter(inputs, outputs);

            // encrypted = AES_encrypt(filtered, KEY);
            // send(encrypted, time);

        if (attestation & myTurn) {
            // Send the answer back to the Verifier (simulator).
            uint64_t hash_msw = (hash_seed >> 64);
            uint64_t hash_lsw = ((hash_seed << 64) >> 64);
            SimSendChallengeResult(hash_msw, hash_lsw); //need id?
            myTurn = false;
            attestation = false;
            disableAllFlags();
        }
        SimRoiEnd();
    }
    return 1;
} 

// Actual processing methods


void readADC(uint16_t * input) {
    int64_t init_pc_addr;  
    if (attestation_flags.at(0))
        init_pc_addr =  reinterpret_cast<int64_t>(getPC()); // Get initial PC
    
    std::uniform_int_distribution<> ushort(0, 511);
    //Get N Samples
    for (size_t i = 0; i < SAMPLES; i++)
        input[i] =  ushort(gen);

    if (attestation_flags.at(0)) {
        cout << "readADC under attestation " << endl;
        __uint128_t diff_addr = init_pc_addr -  reinterpret_cast<__uint128_t>(getPC()); // Get Current PC
        // Now let's build a hash relative to the PC difference (should remain constant)
        __uint128_t hash_module  = diff_addr << 64 | (diff_addr << 120) >> 120;
        // Keep the hash chain
        hash_seed = hash_seed ^ hash_module;
        debugPrintHash("ADC", hash_seed);
    }
}


//Filters N SAMPLES 
void FIRFilter(uint16_t * input, float * output) {
    int64_t init_pc_addr;
     if (attestation_flags.at(1))
        init_pc_addr =  reinterpret_cast<int64_t>(getPC()); // Get initial PC  
    float delayLine [9] = {0,0,0,0,0,0,0,0,0}; 
    float filterTaps [9] = {0.01156111,0.05773286,0.12773940,0.19304879,0.21983567,0.19304879,0.12773940,0.05773286,0.01156111}; 
    float filtered;
    for (size_t sample = 0; sample < SAMPLES; sample++){
        // moving the samples in the delay line
        for (int i = 7; i >=0; i--) { // delayLine being 9 values long
            delayLine[i + 1] = delayLine[i];
            delayLine[0] = input[sample];
        }
        filtered = 0;
        for (int i = 0; i < 9; i++) { // for each tap
            filtered = filtered + delayLine[i] * filterTaps[i]; // multiply     
        }
       output[sample] = filtered;
    }
    if (attestation_flags.at(1)) {
        cout << "FIRFilter under attestation " << endl;
        __uint128_t diff_addr = init_pc_addr -  reinterpret_cast<__uint128_t>(getPC()); // Get Current PC
        // Now let's build a hash relative to the PC difference (should remain constant)
        __uint128_t hash_module  = diff_addr<<116 | diff_addr>>12;
        // Keep the hash chain
        hash_seed = hash_seed ^ hash_module;
        debugPrintHash("FIR", hash_seed);
    }
}


 // Main

int main(int argc, char* argv[]){
    mainSetup();
    int iter = atoi(argv[1]);
    mainLoop(iter);
}


// Attestation Only - methods
// This method sets global  flags to enable
// different hash computations on attestation
void computeChallenge(uint16_t challenge) {
    cout <<"Enabling proper flags for challenge computing"<<endl;
    switch(challenge) {
        case 0:
            enableHashComputation(1);
            break;
        case 1:
            enableHashComputation(2);
            break;
        case 2:
            enableHashComputation(3);
            break;
        default:
            enableHashComputation(3);
            break;
    }
}

void enableHashComputation(uint16_t flag) {
    cout << "Enable Hash Called with " << flag <<endl;
    for (size_t i = 0; i < flag; i++){
        attestation_flags.at(i) = true;
    }
}
void disableAllFlags(){
    for (size_t i = 0; i < attestation_flags.size(); i++){
        attestation_flags.at(i) = false;
    }
    
}



 // Simulator Mockup Methods
// bool simCheckAttestation(){
//     cout << "Check Attestation Called" <<endl;
//     std::uniform_int_distribution<> uchar(0, 255);
//     return (uchar(gen) > 127);
// } 
// bool SimCheckAttestationTurn(){
//     cout << "Check Turn Called "<< endl;
//     std::uniform_int_distribution<> uchar(0, 255);
//     return (uchar(gen) > 150);
// }
// uint16_t SimGetChallengeId(){
//     cout<< "Get Challenge Called "<< endl;
//     std::uniform_int_distribution<> uchar(0, 2);
//     return uchar(gen);
// }
// __uint128_t simGetChallengeSeed(){
//     cout<< "Get Challenge Seed Called "<< endl;
//     std::uniform_int_distribution<> distrib;
//     __uint128_t out = (static_cast<__uint128_t>(distrib(gen)) << 96) | (static_cast<__uint128_t>(distrib(gen)) << 64) |
//                      (static_cast<__uint128_t>(distrib(gen)) << 32) | (static_cast<__uint128_t>(distrib(gen)));
//     return out;
// }
void simSendChallengeResult(__uint128_t result) {
    cout <<"Send Challenge Called" <<endl;
    debugPrintHash("SIM", result);
}


void debugPrintHash(const char * module, __uint128_t hash) {
    if (DEBUG) {
        long int MSB = hash >> 64;
        long int LSB = ((hash << 64) >> 64);
        cout <<"[" << module << "]: "<<"Hash = 0x" << setfill('0') << setw(16) << right << hex << MSB << setfill('0') << setw(16) << right << hex << LSB <<endl;
    }
}
