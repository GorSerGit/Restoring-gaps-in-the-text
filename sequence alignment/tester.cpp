#include "tester.h"
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>
#include <iostream>

void Tester::run_tests(const std::string& test_file, int num_tests) {
    std::ifstream file(test_file);
    if (!file) {
        std::cerr << "Error opening test file: " << test_file << "\n";
        return;
    }
    
    std::vector<std::string> sentences;
    std::string line;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            sentences.push_back(line);
        }
    }
    
    if (sentences.empty()) {
        std::cerr << "No sentences found in test file\n";
        return;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sent_dist(0, sentences.size() - 1);
    
    int tests_done = 0;
    int correct_top1 = 0;
    int correct_top3 = 0;
    int correct_top5 = 0;
    int correct_top10 = 0;
    
    std::cout << "Starting " << num_tests << " tests...\n";
    
    while (tests_done < num_tests) {
        int sent_idx = sent_dist(gen);
        std::vector<std::string> words;
        std::istringstream iss(sentences[sent_idx]);
        std::string word;
        
        while (iss >> word) {
            words.push_back(word);
        }
        
        if (words.size() < 5) continue;
        
        std::uniform_int_distribution<> word_dist(1, words.size() - 2);
        int missing_pos = word_dist(gen);
        
        std::string target_word = words[missing_pos];
        
        std::vector<std::string> left_context;
        std::vector<std::string> right_context;
        
        int left_start = std::max(0, missing_pos - 2);
        int right_end = std::min(static_cast<int>(words.size()), missing_pos + 3);
        
        for (int i = left_start; i < missing_pos; ++i) {
            left_context.push_back(words[i]);
        }
        
        for (int i = missing_pos + 1; i < right_end; ++i) {
            right_context.push_back(words[i]);
        }
        
        auto predictions = predictor->predict_missing_word(left_context, right_context, 10);
        
        for (size_t i = 0; i < predictions.size(); ++i) {
            if (predictions[i].first == target_word) {
                if (i == 0) correct_top1++;
                if (i < 3) correct_top3++;
                if (i < 5) correct_top5++;
                if (i < 10) correct_top10++;
                break;
            }
        }
        
        if (tests_done % 100 == 0 && tests_done > 0) {
            std::cout << "Tests completed: " << tests_done << "/" << num_tests << "\n";
        }
        
        tests_done++;
    }
    
    std::cout << "\n========================================\n";
    std::cout << "TEST RESULTS (" << num_tests << " tests):\n";
    std::cout << "========================================\n";
    std::cout << "Top-1 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top1 / num_tests) << "% (" << correct_top1 << "/" << num_tests << ")\n";
    std::cout << "Top-3 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top3 / num_tests) << "% (" << correct_top3 << "/" << num_tests << ")\n";
    std::cout << "Top-5 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top5 / num_tests) << "% (" << correct_top5 << "/" << num_tests << ")\n";
    std::cout << "Top-10 Accuracy: " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top10 / num_tests) << "% (" << correct_top10 << "/" << num_tests << ")\n";
    std::cout << "========================================\n";
}