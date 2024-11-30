#ifndef PRGOGRESS_BAR
#define PROGRESS_BAR

#include<iostream>

class ProgressBar{
    private:
        int totalSteps;
        int barWidth;
        char barChar;
        char emptyChar;
        int currentStep;

    public:
        ProgressBar(int totalSteps, int barWidth = 50, char barChar = '#', char emptyChar = ' '){
            if(totalSteps<=0){
                throw std::invalid_argument("total currentSteps must be > 0");
            }
            this->totalSteps = totalSteps;
            this->barWidth = barWidth;
            this->barChar = barChar;
            this->emptyChar = emptyChar;
            this->currentStep = 0;          
        }

        void print(){
            int progress = static_cast<int>( ( static_cast<float>(this->currentStep)/this->totalSteps ) * this->barWidth );
            int percentage = static_cast<int>( ( static_cast<float>(this->currentStep)/this->totalSteps ) * 100 );
            
            std::string bar(progress, this->barChar);
            std::string empty(this->barWidth-progress, this->emptyChar);

            std::cout<< "\r["<<bar<<empty<<"] "<<percentage<<"%"<<std::flush;
        }

        void increment(){
            if(this->currentStep < this->totalSteps){
                ++this->currentStep;
                print();
            }
        }
        void update(int currentStep){
            if(currentStep<0 || currentStep> totalSteps){
                throw std::out_of_range("currentSteps must be >= 0 and <= totalSteps");
            }
            this->currentStep = currentStep;
            print();
        }    

};

#endif