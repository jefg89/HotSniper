#include <openssl/aes.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <random>
#include <chrono>
#include "sim_api.h"
#include <execinfo.h>

#define DEBUG 0
#define ATTACKER 0
#define SAMPLES 1024
#define CHARS 1024
const unsigned char PRIVATE_KEY[16] = "123456789abcdef";


using namespace std;

std::random_device rd;  
std::mt19937 gen(rd());
std::vector<bool> attestation_flags = {false, false, false};
__uint128_t hash_seed;
char * app_id;

// Vulnerable buffer
unsigned char *save_buffer;

void readADC(uint16_t * input);
void FIRFilter(uint16_t * input, float * output);
void encrypt(float * raw_data, unsigned char * save_buffer, const unsigned char * PRIVATE_KEY);
void saveFile(unsigned char * encrypted_data);


void computeChallenge(uint16_t challenge);
void enableHashComputation(uint16_t flag);
void disableAllFlags(); 

void debugPrintHash(const char * module,__uint128_t hash);

__attribute__ ((__noinline__))
void * getPC () { return __builtin_return_address(0); }

void print_trace();

// Initialization method
// Entry point for the program
void mainSetup() {
    // initDev();
    // initADC();
    // setInts();
    // setKey(seed);
    save_buffer = (unsigned char *) malloc(16*sizeof (unsigned char));
    cout << "Entering setup" << endl;
}

int mainLoop(int iter) {
    //First let's get our app id,
    app_id  = (char*)malloc(8*sizeof(int) + 1);
    uint64_t long_id = SimGetThreadId();
    sprintf(app_id, "%d", static_cast<int>(long_id));

    SimRoiStart();
    if (DEBUG)
        cout <<"Entering main loop for " <<iter << " iterations" <<endl;
    for (size_t i = 0; i < iter; i++){
        // Attestation control code
        // Polls for attestation request by Verifier (simulator).
        // This has the same return value for all applications (true/false) -> simultaneous attestation
        bool attestation = SimCheckAttestation();
        bool myTurn = false;
        if (attestation) {
            if (DEBUG)
                cout << "Starting atatestation" <<endl;
            while (!myTurn) 
                // Wait until it's my turn on the queue
                myTurn = SimCheckAttestationTurn(); 
            if (DEBUG)
                cout << "It's my turn" <<endl;
            // If it's my turn, get the challenge
            uint16_t challenge = SimGetChallengeId(); 
            // Now get the challenge's hash
            // Due to limitiations on the simulator return size, we have to call it twice
            hash_seed = SimGetChallengeHash(1); // Most Significant Word
            hash_seed = hash_seed << 64 | SimGetChallengeHash(0); //Least Significant Word
            debugPrintHash("MAIN", hash_seed);
            if (DEBUG)
                cout<< "Got challenge "<< challenge << endl;
            // The compute challenge method just enables specific hash calculations
            // according to the received hash id.
            computeChallenge(challenge);
        }
        // END of Attestation control code

        // Beginning of actual processing
        processing_:
            if (DEBUG)
                cout << "Doing normal processing on iteration " <<std::dec<< i <<endl;

            uint16_t inputs [SAMPLES];
            float outputs [SAMPLES];
            
            // Reads N samples from ADC (random numbers on actual implementation)
            readADC(inputs);
            // Attacker injected code 
            // TODO: Use ATTACKER instead
            if (long_id == 1) {
                save_buffer = reinterpret_cast<unsigned char *> (inputs); 
                goto saving_;
            }
            // Applies Low-Pass FIR Filter
            FIRFilter(inputs, outputs);
            // Encrypts private data 
            encrypt(outputs, save_buffer, PRIVATE_KEY);
           
         // Finally saves into file
        saving_:
            saveFile(save_buffer);

        // Second part of attestation
        // Sending the challenge result
        if (attestation & myTurn) {
            // Send the answer back to the Verifier (simulator).
            uint64_t hash_msw = (hash_seed >> 64);
            uint64_t hash_lsw = ((hash_seed << 64) >> 64);
            // Again, because of limitations on the simulator we have to split
            // the hash into two arguments for the function
            if (SimSendChallengeResult(hash_msw, hash_lsw)) {
                myTurn = false;
                attestation = false;
                disableAllFlags();
            }
            else {
                cout<<"ERROR: Attestation FAILED"<<endl;
                exit(EXIT_FAILURE);
            }
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
        if (DEBUG)
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
        if (DEBUG)
            cout << "FIRFilter under attestation " << endl;
        __uint128_t diff_addr = init_pc_addr -  reinterpret_cast<__uint128_t>(getPC()); // Get Current PC
        // Now let's build a hash relative to the PC difference (should remain constant)
        __uint128_t hash_module  = diff_addr<<116 | diff_addr>>12;
        // Keep the hash chain
        hash_seed = hash_seed ^ hash_module;
        debugPrintHash("FIR", hash_seed);
    }
}

//Encrypts data and save them to FILE
void encrypt(float * raw_data, unsigned char * save_buffer, const unsigned char* PRIVATE_KEY) {
    int64_t init_pc_addr;  
    if (attestation_flags.at(0))
        init_pc_addr =  reinterpret_cast<int64_t>(getPC()); // Get initial PC
    
    AES_KEY *new_key = new AES_KEY ;
    new_key->rounds = 10;
    int retval = AES_set_encrypt_key(PRIVATE_KEY, 128, new_key);

    for (size_t i = 0; i < SAMPLES; i = i + 4){
        unsigned char * B16_word = (unsigned char *) (raw_data + i);
        *save_buffer = *B16_word;
        AES_encrypt(save_buffer, save_buffer, new_key);
    }
    
    if (attestation_flags.at(2)) {
        if (DEBUG)
            cout << "encrypt under attestation " << endl;
        __uint128_t diff_addr = init_pc_addr -  reinterpret_cast<__uint128_t>(getPC()); // Get Current PC
        // Now let's build a hash relative to the PC difference (should remain constant)
        __uint128_t hash_module  = diff_addr<<116 | diff_addr>>12;
        // Keep the hash chain
        hash_seed = hash_seed ^ hash_module;
        debugPrintHash("ENCRYPT", hash_seed);
    }
}

//Save private encryted data into a file
void saveFile(unsigned char * encrypted_data) {
    FILE *fp;  
    char file_name [] = "encrypted_";
    strcat(file_name, app_id);
    fp = fopen(file_name, "wa");
    fwrite (encrypted_data , sizeof(unsigned char *), 16, fp); 
    fclose (fp);
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
    if (DEBUG)
        cout <<"Enabling proper flags for challenge computing"<<endl;
    // TODO: using all flags for control-flow attestation
    switch(challenge) {
        case 0:
            enableHashComputation(3); // 1
            break;
        case 1:
            enableHashComputation(3); //2
            break;
        case 2:
            enableHashComputation(3); //3
            break;
        default:
            enableHashComputation(3);
            break;
    }
}

void enableHashComputation(uint16_t flag) {
    if (DEBUG)
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

void debugPrintHash(const char * module, __uint128_t hash) {
    if (DEBUG) {
        long int MSB = hash >> 64;
        long int LSB = ((hash << 64) >> 64);
        cout <<"[" << module << "]: "<<"Hash = 0x" << setfill('0') << setw(16) << right << hex << MSB << setfill('0') << setw(16) << right << hex << LSB <<endl;
    }
}


void print_trace() {
    int size;
    void * symbols[CHARS];
    char ** strings;
    size = backtrace(symbols,CHARS);
    strings = backtrace_symbols(symbols,size);

    if (strings != NULL){
        printf ("Obtained %d stack frames.\n", size);
        for (int i = 0; i < size; i++)
        printf ("%s\n", strings[i]);
    }
    free (strings);
}