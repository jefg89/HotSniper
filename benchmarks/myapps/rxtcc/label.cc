#include <iostream>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <cmath>
#include <cstring>
#include <string.h>
#include <thread> 
#include <sstream>

#define PACKET_BITS 64
#define NUM_SAMPLES 100
#define NUM_PACKETS 16
#define SAMPLES_PER_BIT 50
#define TOTAL_BITS NUM_PACKETS * PACKET_BITS

#define IGNORE 5
using namespace std;

int main(int argc, char const *argv[])
{
    ifstream temp_file("c0.out");
    ifstream packet_file("received.out");
    float current_samples [NUM_SAMPLES];
    string line;

    const size_t window_size = ((TOTAL_BITS * SAMPLES_PER_BIT) / NUM_SAMPLES);
    uint64_t tx_bits [window_size];
    uint64_t tx_packets [NUM_PACKETS];

    // Let's ignore the first measurements
    for (size_t i = 0; i < IGNORE; i++) {
        getline(temp_file, line);
    }
    

    for (size_t i = 0; i < NUM_PACKETS; i++){
        if (packet_file.is_open()) {
                uint64_t temp;
                getline(packet_file, line);
                std::istringstream iss(line);
                iss >> std::hex >> tx_packets[i];
                //cout << std::hex << tx_packets[i] <<endl;
                
            }   
    }
    int idx = 0;
    for (size_t i = 0; i < NUM_PACKETS; i++){
        for (size_t j = 0; j < PACKET_BITS / 4; j++){
          uint64_t msb =  (tx_packets[i] >> j*4 & (0xF));
          tx_bits[window_size - idx - 1]  = (msb & 0x1) || ((msb >> 1) & 0x1);
          tx_bits[window_size - (idx + 1) - 1] = ((msb >> 2) & 0x1) || ((msb >> 3) & 0x1);
          //tx_bits[idx] = (int) (fbit);
          //tx_bits[idx + 1] = sbit
          //cout << std::hex<<msb << " " << tx_bits[window_size - idx - 1]<<" " << tx_bits[window_size - (idx + 1) - 1]<<endl;
          idx += 2;
        }    
    }

    // for (size_t i = 0; i < window_size; i++){
    //     cout <<tx_bits[i] << endl;
    // }
    
    
    ofstream train_file("4G_100HZ_20bps.csv");
    
    
    for (size_t j = 0; j < window_size ; j++)   {
        // Getting samples
        for (size_t i = 0; i < NUM_SAMPLES; i++){
            //cout << "Line " << (j * NUM_SAMPLES) + i << endl;
            if (temp_file.is_open()) {
                getline(temp_file, line);
                current_samples[i] = stof(line);
                train_file << current_samples[i] << ",";
            }   
        }
        train_file << tx_bits[j] << endl;
    }



    return 0;
}
