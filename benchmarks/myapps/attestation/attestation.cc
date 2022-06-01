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
#include "blake2.h"

#define OUTBYTES 64

#define DEBUG 0
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
unsigned char *save_buffer;

void readADC(uint16_t * input);
void FIRFilter(uint16_t * input, float * output);
void encrypt(float * raw_data, unsigned char * save_buffer, const unsigned char * PRIVATE_KEY);
void saveFile(unsigned char * encrypted_data);

void askForHash();
void computeChallenge(uint16_t challenge);
void enableHashComputation(uint16_t flag);
void disableAllFlags(); 

void debugPrintHash(const char * module,unsigned char * hash);

__attribute__ ((__noinline__))
void * getPC () { return __builtin_return_address(0); }


//Blake2 functions
/* This will help compatibility with coreutils */
int blake2s_stream( FILE *stream, void *resstream, size_t outbytes )
{
  int ret = -1;
  size_t sum, n;
  blake2s_state S[1];
  static const size_t buffer_length = 32768;
  uint8_t *buffer = ( uint8_t * )malloc( buffer_length );

  if( !buffer ) return -1;

  blake2s_init( S, outbytes );

  while( 1 )
  {
    sum = 0;

    while( 1 )
    {
      n = fread( buffer + sum, 1, buffer_length - sum, stream );
      sum += n;

      if( buffer_length == sum )
        break;

      if( 0 == n )
      {
        if( ferror( stream ) )
          goto cleanup_buffer;

        goto final_process;
      }

      if( feof( stream ) )
        goto final_process;
    }

    blake2s_update( S, buffer, buffer_length );
  }

final_process:;

  if( sum > 0 ) blake2s_update( S, buffer, sum );

  blake2s_final( S, resstream, outbytes );
  ret = 0;
cleanup_buffer:
  free( buffer );
  return ret;
}

int blake2b_stream( FILE *stream, void *resstream, size_t outbytes )
{
  int ret = -1;
  size_t sum, n;
  blake2b_state S[1];
  static const size_t buffer_length = 32768;
  uint8_t *buffer = ( uint8_t * )malloc( buffer_length );

  if( !buffer ) return -1;

  blake2b_init( S, outbytes );

  while( 1 )
  {
    sum = 0;

    while( 1 )
    {
      n = fread( buffer + sum, 1, buffer_length - sum, stream );
      sum += n;

      if( buffer_length == sum )
        break;

      if( 0 == n )
      {
        if( ferror( stream ) )
          goto cleanup_buffer;

        goto final_process;
      }

      if( feof( stream ) )
        goto final_process;
    }

    blake2b_update( S, buffer, buffer_length );
  }

final_process:;

  if( sum > 0 ) blake2b_update( S, buffer, sum );

  blake2b_final( S, resstream, outbytes );
  ret = 0;
cleanup_buffer:
  free( buffer );
  return ret;
}

int blake2sp_stream( FILE *stream, void *resstream, size_t outbytes )
{
  int ret = -1;
  size_t sum, n;
  blake2sp_state S[1];
  static const size_t buffer_length = 16 * ( 1UL << 20 );
  uint8_t *buffer = ( uint8_t * )malloc( buffer_length );

  if( !buffer ) return -1;

  blake2sp_init( S, outbytes );

  while( 1 )
  {
    sum = 0;

    while( 1 )
    {
      n = fread( buffer + sum, 1, buffer_length - sum, stream );
      sum += n;

      if( buffer_length == sum )
        break;

      if( 0 == n )
      {
        if( ferror( stream ) )
          goto cleanup_buffer;

        goto final_process;
      }

      if( feof( stream ) )
        goto final_process;
    }

    blake2sp_update( S, buffer, buffer_length );
  }

final_process:;

  if( sum > 0 ) blake2sp_update( S, buffer, sum );

  blake2sp_final( S, resstream, outbytes );
  ret = 0;
cleanup_buffer:
  free( buffer );
  return ret;
}


int blake2bp_stream( FILE *stream, void *resstream, size_t outbytes )
{
  int ret = -1;
  size_t sum, n;
  blake2bp_state S[1];
  static const size_t buffer_length = 16 * ( 1UL << 20 );
  uint8_t *buffer = ( uint8_t * )malloc( buffer_length );

  if( !buffer ) return -1;

  blake2bp_init( S, outbytes );

  while( 1 )
  {
    sum = 0;

    while( 1 )
    {
      n = fread( buffer + sum, 1, buffer_length - sum, stream );
      sum += n;

      if( buffer_length == sum )
        break;

      if( 0 == n )
      {
        if( ferror( stream ) )
          goto cleanup_buffer;

        goto final_process;
      }

      if( feof( stream ) )
        goto final_process;
    }

    blake2bp_update( S, buffer, buffer_length );
  }

final_process:;

  if( sum > 0 ) blake2bp_update( S, buffer, sum );

  blake2bp_final( S, resstream, outbytes );
  ret = 0;
cleanup_buffer:
  free( buffer );
  return ret;
}

typedef int ( *blake2fn )( FILE *, void *, size_t );

void blake2_api(unsigned char *input, unsigned char * output ){
    blake2fn blake2_stream = blake2b_stream;
    unsigned long maxbytes = BLAKE2B_OUTBYTES;
    const char *algorithm = "BLAKE2b";
    unsigned long outbytes = 0;
    bool bsdstyle = false;
    int c, i;
    opterr = 1;

    if(outbytes > maxbytes){
        printf( "Invalid length argument: %lu\n", outbytes * 8 );
        printf( "Maximum digest length for %s is %lu\n", algorithm, maxbytes * 8 );
    }
    else if( outbytes == 0 )
        outbytes = maxbytes;

    FILE *f = NULL;

    unsigned char * input_hash = (unsigned char*) malloc(maxbytes *sizeof(unsigned char));
    input_hash = input;
    f = fmemopen(input_hash, maxbytes * sizeof(unsigned char), "r");


    if( blake2_stream( f, output, outbytes ) < 0 ){
        printf("Failed to hash argument \n");
        exit(EXIT_FAILURE);
    }
    if(f != stdin) fclose( f );
}




// Initialization method
// Entry point for the program
void mainSetup() {
    save_buffer = (unsigned char *) malloc(16*sizeof (unsigned char));
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
    //for (size_t i = 0; i < iter; i++){
        // Attestation control code
        // Polls for attestation request by Verifier (simulator).
        // This has the same return value for all applications (true/false) -> simultaneous attestation
        bool attestation =  true;//SimCheckAttestation();
        //if (!HW) {SimSetAttestationSW();}
        if (attestation) {
            if (DEBUG)
                cout << "Starting atatestation" <<endl;
            // Try get the challenge
            uint16_t challenge = 3; 
            // The compute challenge method just enables specific hash calculations
            // according to the received hash id.
            computeChallenge(challenge);
            // if(attestation_flags.at(0)) {
            //     if (HW)
            //       askForHash();
            //     else
            //       blake2_api(reinterpret_cast<unsigned char*>(app_id), hash_seed);
            //debugPrintHash("MAIN", hash_seed);
            //    if (DEBUG)
            //        cout<< "Got challenge "<< challenge << endl;
           //}
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
            // if (long_id == 1) {
            //     save_buffer = reinterpret_cast<unsigned char *> (inputs); 
            //     goto saving_;
            // }
            // Applies Low-Pass FIR Filter
            //FIRFilter(inputs, outputs);
            // Encrypts private data 
            //encrypt(outputs, save_buffer, PRIVATE_KEY);
           
         // Finally saves into file
        saving_:
            //saveFile(save_buffer);



        disableAllFlags();
        if (attestation) {
          computeChallenge(3);    
        }
        for (size_t i = 0 ; i < iter; i++){
            cout <<" Iter =  "<< i <<endl;
          
            readADC(inputs);
        
        }

        // Second part of attestation
        // Waiting for all applications to finish
      
       if (attestation) {
          attestation = false;
          //if (HW) {
          bool finished = false;
          while (!finished) {
            finished = SimCheckAllFinished();
            if (tries%100000 == 0)
              cout<< "Finished? " << finished << " on tries " <<tries <<endl;
            tries ++;
          }
          //}
          //For software attestation
          //else {
          //   SimNotifyFinishSW();
          //   bool finished = false;
          //   while (!finished) {
          //     finished = SimCheckAllFinished(is_hardware);
          //     if (tries%100000 == 0)
          //       cout<< "(SW) Finished? " << finished << " on tries " <<tries <<endl;
          //     tries ++;
          //   }
          // }
          if (DEBUG)
              cout << "All applications have finished their attestation computation" <<endl;
        }
      SimRoiEnd();
      return 1;
} 

// Actual processing methods


void readADC(uint16_t * input) {
    int64_t init_pc_addr;  
    // if (attestation_flags.at(0))
    //     init_pc_addr =  reinterpret_cast<int64_t>(getPC()); // Get initial PC
    
    std::uniform_int_distribution<> ushort(0, 511);
    //Get N Samples
    for (size_t i = 0; i < SAMPLES; i++)
        input[i] =  ushort(gen);

    if (attestation_flags.at(0)) {
        if (DEBUG)
            cout << "readADC under attestation " << endl;
        __uint128_t diff_addr = init_pc_addr -  reinterpret_cast<__uint128_t>(getPC()); // Get Current PC
        // Now let's build a hash relative to the PC difference (should remain constant)
      bool proc = false;
      int wait = 0;
      //Check if we  have a previous request on the queue
      if (SimCheckOnQueue())
        //If so, let's wait until we have no previous requests
        while (!proc) {
          proc = !SimCheckOnQueue();
          if (wait % 100000 == 0 && !proc)
            cout << "waiting for " <<wait <<"iterations" <<endl;
          wait++;
        }

      // Check if we can run in HW mode
      bool HW = SimGetAttestationMode();
      //If it's better for us to run on Hardware
      // then, run on hardware. 
      if (HW)
        askForHash();
      else {
        SimSetAttestationSW();
        blake2_api(hash_seed, hash_seed);
        SimNotifyFinishSW();
        debugPrintHash("ADC", hash_seed);
        }
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

      bool proc = false;
      int wait = 0;
      //Check if we  have a previous request on the queue
      if (SimCheckOnQueue())
        //If so, let's wait until we have no previous requests
        while (!proc) {
          proc = !SimCheckOnQueue();
          if (wait % 100000 == 0 && !proc)
            cout << "waiting for " <<wait <<"iterations" <<endl;
          wait++;
        }

      // Check if we can run in HW mode
      bool HW = SimGetAttestationMode();
      //If it's better for us to run on Hardware
      // then, run on hardware. 
      if (HW)
        askForHash();
      else {
        SimSetAttestationSW();
        blake2_api(hash_seed, hash_seed);
        SimNotifyFinishSW();
        debugPrintHash("ADC", hash_seed);
        }
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
        bool HW = SimGetAttestationMode();
        if (HW)
          askForHash();
        else {
          blake2_api(hash_seed, hash_seed);
          debugPrintHash("ENCRYPT", hash_seed);
        }
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
    if (attestation_flags.at(2)) {
         if (DEBUG)
            cout << "save under attestation " << endl;
          bool HW = SimGetAttestationMode();
          if (HW)
            askForHash();//diff_addr<<116 | diff_addr>>12;
          else {
            blake2_api(hash_seed, hash_seed);
            debugPrintHash("SAVE", hash_seed);
        }
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

void debugPrintHash(const char * module, unsigned char * hash) {
    cout <<"[" << module << "]: "<<"Hash = 0x";
    size_t j;
    for( j = 0; j < OUTBYTES; ++j )
        printf( "%02x", hash[j] );
    cout<<endl;
}

void askForHash() {
    // Request for a turn on the queue and keep processing
    uint16_t ticket = SimGetRequestTurn();
    if (DEBUG){
        cout << "Got ticket " << std::dec <<ticket <<endl;
    }
}