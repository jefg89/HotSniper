#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif // _GNU_SOURCE

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
#include <unistd.h>


#define OUTBYTES 64

#define DEBUG 1
#define ATTACKER 0
#define SAMPLES 1024
#define CHARS 1024
const unsigned char PRIVATE_KEY[16] = "123456789abcdef";


using namespace std;

std::random_device rd;  
std::mt19937 gen(rd());
std::vector<bool> attestation_flags = {false, false, false};
//__uint128_t hash_seed;
unsigned char * hash_seed;
char * app_id;
int tries = 0;
// Vulnerable buffer
unsigned char **save_buffer;

void readADC(uint16_t * input);
void FIRFilter(uint16_t * input, float * output);
void encrypt(float * raw_data, unsigned char ** save_buffer, const unsigned char * PRIVATE_KEY);
void saveFile(unsigned char * encrypted_data);


// Initialization method
// Entry point for the program
void mainSetup() {
    save_buffer = (unsigned char **) malloc(SAMPLES/4*sizeof (unsigned char**));
    for (size_t i = 0; i < SAMPLES/4; i++){
        save_buffer[i] = (unsigned char *) malloc(16*sizeof (unsigned char*));
    }
    
    hash_seed = (unsigned char *) malloc(OUTBYTES*sizeof(unsigned char));
    cout << "Entering setup" << endl;
}

int mainLoop(int iter) {
    //First let's get our app id,
    app_id  = (char*)malloc(8*sizeof(int) + 1);
    uint64_t long_id = SimGetThreadId();
    sprintf(app_id, "%d", static_cast<int>(long_id));
    bool end = false;
    int i = 0;
    SimRoiStart();
    if (DEBUG)
        cout <<"Entering main loop for " <<iter << " iterations" <<endl;
    for (size_t i = 0; i < iter; i++){
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
            //if (long_id == 1) {
            if (ATTACKER == 1) {
                saveFile (reinterpret_cast<unsigned char *> (inputs)); 
                goto exit_;
                //goto saving_;
            }
            // Applies Low-Pass FIR Filter
            FIRFilter(inputs, outputs);
            // Encrypts private data 
            encrypt(outputs, save_buffer, PRIVATE_KEY);
           
         // Finally saves into file
        saving_:
            for (size_t i = 0; i < SAMPLES/4; i++){
                saveFile(save_buffer[i]);
            }
        exit_:
            end=!end;
    }

      SimRoiEnd();
      return 1;
} 

// Actual processing methods


void readADC(uint16_t * input) {
    std::uniform_int_distribution<> ushort(0, 511);
    //Get N Samples
    for (size_t i = 0; i < SAMPLES; i++)
        input[i] =  ushort(gen);
}

//Filters N SAMPLES 
void FIRFilter(uint16_t * input, float * output) {
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
}

//Encrypts data and save them to FILE
void encrypt(float * raw_data, unsigned char ** save_buffer, const unsigned char* PRIVATE_KEY) {
    AES_KEY *new_key = new AES_KEY ;
    new_key->rounds = 10;
    int retval = AES_set_encrypt_key(PRIVATE_KEY, 128, new_key);

    int idx = 0;
    for (size_t i = 0; i < SAMPLES; i = i + 4){
        unsigned char * B16_word = (unsigned char *) (raw_data + i);
        save_buffer[idx] = B16_word;
        AES_encrypt(save_buffer[idx], save_buffer[idx], new_key);
        idx++;
    }
}

//Save private encryted data into a file
void saveFile(unsigned char * encrypted_data) {
    FILE *fp;  
    char file_name [] = "encrypted_";
    strcat(file_name, app_id);
    fp = fopen(file_name, "a");  
    unsigned char tmp [16];
    //memccpy(tmp, encrypted_data, 0, 16*sizeof(unsigned char));
    fwrite (encrypted_data , sizeof(unsigned char), 16, fp); 
    fclose (fp);
    //for (size_t i = 0; i < 16; i++){
    //   cout <<std::hex <<(int)encrypted_data [i];
    //}
    //cout <<endl;
    
    
}

 // Main

int main(int argc, char* argv[]){
    mainSetup();
    int iter = atoi(argv[1]);
    mainLoop(iter);
}

