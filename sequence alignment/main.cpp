#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "predictor.h"
#include "tester.h"

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    
    SequenceAlignmentPredictor predictor;
    
    std::cout << "=== SEQUENCE ALIGNMENT WORD PREDICTOR ===\n\n";
    
    std::cout << "Loading vocabulary...\n";
    if (!predictor.load_vocabulary("preprocessed/vocab.txt")) {
        std::cerr << "Failed to load vocabulary!\n";
        return 1;
    }
    std::cout << "Vocabulary loaded successfully.\n";
    
    std::cout << "Building n-gram statistics...\n";
    predictor.build_statistics("preprocessed/processed.txt");
    std::cout << "Statistics built.\n\n";
    
    Tester tester(&predictor);
    std::cout << "Starting model testing...\n";
    tester.run_tests("preprocessed/processed.txt", 1000);
    
    std::cout << "\n\n=== EXAMPLE PREDICTION ===\n";
    std::vector<std::string> left_context = {"человек", "хороший"};
    std::vector<std::string> right_context = {"говорить"};
    
    auto predictions = predictor.predict_missing_word(left_context, right_context, 10);
    
    std::cout << "Context: ... человек хороший _____ говорить ...\n";
    std::cout << "Top-10 predictions:\n";
    std::cout << "--------------------------------------------------\n";
    std::cout << "#   | Word           | Score\n";
    std::cout << "--------------------------------------------------\n";
    
    for (size_t i = 0; i < predictions.size(); ++i) {
        std::cout << std::setw(3) << i+1 << " | " 
                  << std::setw(15) << std::left << predictions[i].first << " | " 
                  << std::fixed << std::setprecision(4) << predictions[i].second << "\n";
    }
    
    return 0;
}