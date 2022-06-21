#include <iostream>
#include <chrono>
#include <thread>
#include <math.h>
using namespace std;
using namespace std::chrono;

#define BPS 20
#define FREQ_CHANNEL_HZ  100 



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


void encodeOne(double milli_seconds) {
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
    
}
int main(int argc, char const *argv[])
{
    cout << "Hello from main " <<endl;
    preciseMilliSleep(6);
    float bit_period_ms = 1000 / FREQ_CHANNEL_HZ;
    float num_periods = (1000/BPS) / bit_period_ms;  
    uint8_t data = static_cast<uint8_t> (atoi (argv[1]));
    for (size_t i = 0; i < 8; i++){
        uint8_t bit = data >>  7;
        data = data << 1;
        for (size_t j = 0; j < num_periods; j++)
        {
            if (bit == 0) encodeZero(bit_period_ms / 2);
            else encodeOne(bit_period_ms / 2);
        }
         
    }
    return 0;
}
