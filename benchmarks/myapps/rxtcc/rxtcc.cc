#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
#include "sim_api.h"
using namespace std;
using namespace std::chrono;

#define BPS 20
#define FREQ_CHANNEL_HZ  100
#define DELTA_THRESHOLD 3
#define PACKET_BITS 5*8 



void preciseMilliSleep(double milli_seconds) {
    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (milli_seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(microseconds(100));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e6; //Diff in milliseconds
        milli_seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }
    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e6 < milli_seconds);
}


/* void encodeOne(double milli_seconds) {
    bool exit = false;
    auto start = high_resolution_clock::now();
    while (!exit) {
        auto end = high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end-start;
        exit = elapsed.count() >= milli_seconds;
        if (exit) cout << "One: Done processing for: " << elapsed.count() <<"ms" <<endl;
    }
    
    start = high_resolution_clock::now();
    preciseMilliSleep(milli_seconds);
    auto end = high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    cout << "One: Done waiting for another: " << elapsed.count()<<"ms"  <<endl;
}

void encodeZero(double milli_seconds) {
    auto start = high_resolution_clock::now();
    preciseMilliSleep(2*milli_seconds);
    auto end = high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    cout << "Zero: Done waiting for another: " << elapsed.count()<<"ms"  <<endl;
    
} */


uint64_t getCurrTemperature() {
    // Call the simulator to get our current temperature
    uint64_t temp = SimGetTemperature(); // TODO: replace with actual Sim call
    cout << "Current Temperature = " << std::dec <<temp <<endl;
    return temp;
}




int main(int argc, char const *argv[])
{
    cout << "Hello from main RX " <<endl;
    float bit_period_ms = 1000 / FREQ_CHANNEL_HZ;
    uint16_t base_temp = static_cast<uint16_t>(getCurrTemperature());
    for (size_t i = 0; i < 5; i++){
        base_temp = static_cast<uint16_t>(getCurrTemperature());
        preciseMilliSleep(1);
    }
    
    //float num_periods = (1000/BPS) / bit_period_ms;  


    
    bool exit = false;
    bool rx = false;
    uint8_t rx_bits = 0;
    uint64_t packet = 0;

    while (!exit){
        uint16_t new_temp = static_cast<uint16_t>(getCurrTemperature());
        int16_t delta = new_temp - base_temp;
       
        if (delta >= DELTA_THRESHOLD){
            cout << "Initial bit detected" <<endl;
            rx = true;
            rx_bits++;
            packet = (packet  << 1 ) | 1;
            // If we got a 1, wait for a period to start receiving the bits.
            preciseMilliSleep(bit_period_ms);
             // Maybe we update the base here
            base_temp = static_cast<uint16_t>(getCurrTemperature());
        }
       
        while(rx) {
            uint16_t new_temp = static_cast<uint16_t>(getCurrTemperature());	
            int16_t delta = new_temp - base_temp;
            cout << "new = " <<new_temp << " base = " <<base_temp << " delta = " << delta<<endl;
            if (delta >= DELTA_THRESHOLD){
                cout << "One detected" << endl;
                packet = (packet  << 1 ) | 1;
                preciseMilliSleep(bit_period_ms);
            }
            else {
                cout <<"Zero inferred" <<endl;
                packet = packet  << 1 ;
                preciseMilliSleep(bit_period_ms);
            }
            cout << "Currently received: "<< std::hex << packet << endl;
            rx_bits++;
            rx = (rx_bits < PACKET_BITS);
            exit = true;
        }
       
    }
    cout << "Received " << std::hex << packet << endl;
    return 0;
}
