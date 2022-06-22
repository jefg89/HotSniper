#include <iostream>
#include <cstdlib>
#include <chrono>
#include <fstream>
// Need to instal fftw3 https://www.fftw.org/
#include <fftw3.h>
#include <cmath>
#include <cstring>
#include <string.h>
#include <thread> 
#include <sstream>

#define DEBUG 0
#define SAMPLING_PERIOD_MS 1
#define NUM_SAMPLES 50
#define PACKET_BITS 8
#define SAMPLING_FREQ 1000/SAMPLING_PERIOD_MS
#define DFT_SIZE NUM_SAMPLES/2 + 1

#define MIN_FREQ_CH_HZ 93
#define MAX_FREQ_CH_HZ 105
#define MAGNITUDE_THRESHOLD 22



using namespace std;
bool start_dft = false;
bool start_sampling = false;
bool start_detection = false;
float * dft_input;
double * dft_magnitudes;

ifstream temp_file("out.log");

void getTemperatures() {
    float old_samples [NUM_SAMPLES/2];
    float current_samples [NUM_SAMPLES];
    
    string line;

    // Filling initial samples (old)
    // for (size_t i = 0; i < NUM_SAMPLES; i++){
    //     old_samples [i] = 0; // base temperature - TODO: UpdateMe
    // }

    while(1) {
        while(start_sampling) {
            // "Thread - safe" printing
            std::stringstream msg;
            msg << "Reading " << NUM_SAMPLES << " temperature samples... \n";
            //std::cout << msg.str();

            for (size_t i = 0; i < NUM_SAMPLES; i++){
                    bool timeout = false;
                    if (temp_file.is_open()) {
                        getline(temp_file, line);
                        current_samples[i] = stof(line);
                    }

                    auto t1 = std::chrono::steady_clock::now();
                    while (!timeout){
                        auto t2 = std::chrono::steady_clock::now();
                        auto d_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
                        if (d_milli >= SAMPLING_PERIOD_MS) 
                            timeout = true;  
                    }     
            }
            // Copy old samples a the beginning of window
            //memcpy(dft_input, old_samples, (NUM_SAMPLES/2)*sizeof(float));
            // Copy new samples at the end of window
            //memcpy(&dft_input[NUM_SAMPLES/2], current_samples, (NUM_SAMPLES/2)*sizeof(float));
            memcpy(dft_input, current_samples, (NUM_SAMPLES)*sizeof(float));
            start_dft = true;

            if (DEBUG) {//Non thread-safe printing, too lazy to change it.
                cout<<"DFT Input "<<"[";
                for (size_t i = 0; i < NUM_SAMPLES - 1; i++)
                {
                    cout  << dft_input[i] <<", ";
                }
                cout << dft_input[NUM_SAMPLES - 1] << "]" << endl;
            }
            
            //memcpy(old_samples, current_samples, (NUM_SAMPLES/4)*sizeof(float));
        }
    }   
}

void doDFT() {
    while(1)
        while(start_dft) {
            // Memory allocation for input.
            // Copy data to a new pointer, since DFT destroys input.
            // Probably not necessary
            double * in = (double *) fftw_malloc(sizeof(double) * NUM_SAMPLES);
            for (size_t i = 0; i < NUM_SAMPLES; i++){
                in[i] = dft_input[i];
            }
           
            std::stringstream msg;
            msg << "Computing DFT \n";
            //std::cout << msg.str();
           
            // Actual DFT computation
            fftw_complex * out;
            out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES);
            fftw_plan p;
            p = fftw_plan_dft_r2c_1d(NUM_SAMPLES, in , out, FFTW_ESTIMATE);
            fftw_execute(p);
            fftw_destroy_plan(p);
            
            for (size_t i = 0; i < DFT_SIZE; i++){
                dft_magnitudes[i] = sqrt(pow(out[i][0], 2) + pow(out[i][1], 2)); 
            }
            // Freeing fft memory
            fftw_free(in); fftw_free(out);
            start_dft = false;
            start_detection = true;
            if (DEBUG) {
                cout << "DFT Magnitudes " <<endl;
                for (size_t i = 0; i < DFT_SIZE; i++){
                    cout << i << "\t" << dft_magnitudes[i] <<endl; 
                }   
            }
        }
}

void detectChannel() {
    // Detects where there is 1 or a 0 on the specified frequencies
    // MIN_FREQ_CH_HZ = left limit of channel
    // MAX_FREQ_CH_HZ = right limit of channel

    double frequencies [DFT_SIZE];
    float sfreq_float = static_cast<float> (SAMPLING_FREQ);
    float nsamples_float =  static_cast<float> (NUM_SAMPLES);
    double delta_freq = sfreq_float/nsamples_float;
    int min_idx = 0;
    int max_idx = 0;
    bool communication = false;
    bool found = false;
    int bits = 0;
    int packet = 0;
    
    //Convert sample number into frequency
    for (size_t i = 0; i < DFT_SIZE; i++){
        frequencies[i] = i * delta_freq ;
    }

    // Do boundary computation for channel detection
    for (size_t i = 0; i < DFT_SIZE/2; i++)
        if (frequencies[i] >= MIN_FREQ_CH_HZ) {
            min_idx = i;
            //cout << "Min index = " <<min_idx << " freq = " << frequencies[i] <<endl;
            break;
        }

    for (size_t i = 0; i < DFT_SIZE/2; i++)
        if (frequencies[i] >= MAX_FREQ_CH_HZ) {
            max_idx = i;
            //cout << "Max index = " <<max_idx << " freq = " << frequencies[i] <<endl;
            break;
        }

    while(1)
    //TODO: improve detection logic. How to extract valid info from the combination of two consecutive windows
    //Maybe it's better to implement the logic itself out of this thread.
    //This thread  just tells (prints :/ ) if there is a 1 or a 0 in the window. 
        while(start_detection) {
            std::stringstream msg;
            //msg << "Starting Detection...\n";
            //std::cout << msg.str();
            // msg<<frequencies[min_idx] <<"Hz : 
            //for (size_t i = min_idx; i <= max_idx ; i++){
            if (dft_magnitudes[min_idx] >= MAGNITUDE_THRESHOLD) {
                msg << "[1] " <<"Magnitude " <<dft_magnitudes[min_idx] <<endl;
                std::cout << msg.str();
                communication = true;
                packet = (packet << 1) | 1;
                bits++;
                }
            else {
                std::stringstream msg_;
                msg_ << "[0] " <<"Magnitude " <<dft_magnitudes[min_idx] <<endl;
                std::cout << msg_.str();
                if (communication) {
                    packet = packet << 1;
                    bits++;
                } 
            }
            if (bits >= PACKET_BITS) {
                std::stringstream msg_;
                msg_ << "Received a packet = " <<std::hex <<packet <<endl;
                std::cout << msg_.str();
                communication = false;
                bits = 0;
                packet = 0;
            }
            start_detection = false;
        }
        
}

int main(int argc, char const *argv[]){
    
    dft_input = (float *) malloc(NUM_SAMPLES  * sizeof(float));
    dft_magnitudes = (double *) malloc(DFT_SIZE * sizeof(double));
    
    // Thread definition
    std::thread sampling_thread(getTemperatures);
    std::thread dft_thread (doDFT);
    std::thread detection_thread (detectChannel);
    start_sampling = true;
    // Syncronize threads
    sampling_thread.join();
    dft_thread.join();
    detection_thread.join();
    

    free(dft_input);
    free(dft_magnitudes);
    
    return 0;
}
