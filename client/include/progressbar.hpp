#ifndef PROGRESS_BAR
#define PROGRESS_BAR

#include <stdexcept>

class ProgressBar {
   private:
    int totalSteps;
    int barWidth;
    char barChar;
    char emptyChar;
    int currentStep;

   public:
    ProgressBar(int totalSteps, int barWidth = 50, char barChar = '#', char emptyChar = ' ') : totalSteps(totalSteps), barWidth(barWidth), barChar(barChar), emptyChar(emptyChar), currentStep(currentStep) {
        if (totalSteps <= 0) {
            throw std::invalid_argument("total currentSteps must be > 0");
        }
    }
    void print();
    void increment();
    void update(int currentStep);
};

#endif