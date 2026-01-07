#ifndef TESTER_H
#define TESTER_H

#include <string>
#include "predictor.h"

class Tester {
private:
    SequenceAlignmentPredictor* predictor;

public:
    Tester(SequenceAlignmentPredictor* pred) : predictor(pred) {}
    
    void run_tests(const std::string& test_file, int num_tests = 1000);
};

#endif