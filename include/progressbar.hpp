#ifndef PRGOGRESS_BAR
#define PROGRESS_BAR

#include <iostream>

class ProgressBar {
    private:
    int totalSteps;
    int barWidth;
    char barChar;
    char emptyChar;
    int currentStep;

    public:
    ProgressBar (int totalSteps, int barWidth = 50, char barChar = '#', char emptyChar = ' ')
    : totalSteps (totalSteps), barWidth (barWidth), barChar (barChar), emptyChar (emptyChar), currentStep (currentStep) {
        if (totalSteps <= 0) {
            throw std::invalid_argument ("total currentSteps must be > 0");
        }
    }

    void print () {
        int progress =
        static_cast<int> ((static_cast<float> (currentStep) / totalSteps) * barWidth);
        int percentage =
        static_cast<int> ((static_cast<float> (currentStep) / totalSteps) * 100);

        std::string bar (progress, barChar);
        std::string empty (barWidth - progress, emptyChar);

        std::cout << "\r[" << bar << empty << "] " << percentage << "%" << std::flush;
    }

    void increment () {
        if (currentStep < totalSteps) {
            ++currentStep;
            print ();
        }
    }
    void update (int currentStep) {
        if (currentStep < 0 || currentStep > totalSteps) {
            throw std::out_of_range (
            "currentSteps must be >= 0 and <= totalSteps");
        }
        this->currentStep = currentStep;
        print ();
    }
};

#endif