#include "progressbar.hpp"

#include <iostream>

void ProgressBar::print() {
    int progress = static_cast<int>((static_cast<float>(currentStep) / totalSteps) * barWidth);
    int percentage = static_cast<int>((static_cast<float>(currentStep) / totalSteps) * 100);

    std::string bar(progress, barChar);
    std::string empty(barWidth - progress, emptyChar);

    std::cout << "\r[" << bar << empty << "] " << percentage << "%" << std::flush;
}

void ProgressBar::increment() {
    if (currentStep < totalSteps) {
        ++currentStep;
        print();
    }
}
void ProgressBar::update(int currentStep) {
    if (currentStep < 0 || currentStep > totalSteps) {
        throw std::out_of_range("currentSteps must be >= 0 and <= totalSteps");
    }
    this->currentStep = currentStep;
    print();
}