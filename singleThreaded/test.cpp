#include <iostream>
#include <iomanip> // for setw, setprecision, setfill
#include <chrono>
#include <thread> // simulate work on cpu

int main()
{
    int batch_size = 4000;
    int num_bars = 50;
    int batch_per_bar = batch_size / num_bars;

    int progress = 0;

    for (int i = 0; i < batch_size; i++) {
        if (i % batch_per_bar == 0) {    
            std::cout << std::setprecision(3) <<
                      // fill bar with = up to current progress
                      '[' << std::setfill('=') << std::setw(progress) << '>'
                      // fill the rest of the bar with spaces
                      << std::setfill(' ') << std::setw(num_bars - progress + 1)
                      // display bar percentage, \r brings it back to the beginning
                      << ']' << std::setw(3) << ((i + 1) * 100 / batch_size) << '%'
                      << "\r";
            progress++;
        }
        
        // simulate work
        std::this_thread::sleep_for(std::chrono::nanoseconds(1000000));
    }
}
