#include "tester.h"
#include <fstream>
#include <sstream>
#include <random>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <climits>
#include <vector>

// Функция для вычисления расстояния Левенштейна
int levenshtein_distance(const std::string& s1, const std::string& s2) {
    int m = s1.length();
    int n = s2.length();
    
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    
    for (int i = 0; i <= m; ++i) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= n; ++j) {
        dp[0][j] = j;
    }
    
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i-1][j] + 1,      // удаление
                dp[i][j-1] + 1,      // вставка
                dp[i-1][j-1] + cost  // замена
            });
        }
    }
    
    return dp[m][n];
}

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
    
    // Метрики Левенштейна
    double total_levenshtein_top1 = 0.0;
    double total_levenshtein_top3 = 0.0;
    double total_levenshtein_top5 = 0.0;
    double total_levenshtein_top10 = 0.0;
    
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
        
        // Проверка точности
        bool found_in_top3 = false;
        bool found_in_top5 = false;
        bool found_in_top10 = false;
        
        // Вычисление минимального расстояния Левенштейна для каждого топа
        int min_levenshtein_top1 = INT_MAX;
        int min_levenshtein_top3 = INT_MAX;
        int min_levenshtein_top5 = INT_MAX;
        int min_levenshtein_top10 = INT_MAX;
        
        for (size_t i = 0; i < predictions.size(); ++i) {
            int distance = levenshtein_distance(predictions[i].first, target_word);
            
            if (i == 0) {
                min_levenshtein_top1 = distance;
                if (distance == 0) {
                    correct_top1++;
                }
            }
            
            if (i < 3) {
                min_levenshtein_top3 = std::min(min_levenshtein_top3, distance);
                if (distance == 0) {
                    found_in_top3 = true;
                }
            }
            
            if (i < 5) {
                min_levenshtein_top5 = std::min(min_levenshtein_top5, distance);
                if (distance == 0) {
                    found_in_top5 = true;
                }
            }
            
            if (i < 10) {
                min_levenshtein_top10 = std::min(min_levenshtein_top10, distance);
                if (distance == 0) {
                    found_in_top10 = true;
                }
            }
        }
        
        if (found_in_top3) correct_top3++;
        if (found_in_top5) correct_top5++;
        if (found_in_top10) correct_top10++;
        
        // Суммирование расстояний Левенштейна
        total_levenshtein_top1 += min_levenshtein_top1;
        total_levenshtein_top3 += min_levenshtein_top3;
        total_levenshtein_top5 += min_levenshtein_top5;
        total_levenshtein_top10 += min_levenshtein_top10;
        
        if (tests_done % 100 == 0 && tests_done > 0) {
            std::cout << "Tests completed: " << tests_done << "/" << num_tests << "\n";
        }
        
        tests_done++;
    }
    
    // Вывод результатов
    std::cout << "\n===================================================\n";
    std::cout << "TEST RESULTS (" << num_tests << " tests):\n";
    std::cout << "===================================================\n";
    std::cout << "Top-1 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top1 / num_tests) << "% (" << correct_top1 << "/" << num_tests << ")\n";
    std::cout << "Top-3 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top3 / num_tests) << "% (" << correct_top3 << "/" << num_tests << ")\n";
    std::cout << "Top-5 Accuracy:  " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top5 / num_tests) << "% (" << correct_top5 << "/" << num_tests << ")\n";
    std::cout << "Top-10 Accuracy: " << std::fixed << std::setprecision(2) 
              << (100.0 * correct_top10 / num_tests) << "% (" << correct_top10 << "/" << num_tests << ")\n";
    
    std::cout << "\nLevenshtein Distance Metrics:\n";
    std::cout << "-----------------------------------\n";
    std::cout << "Top-1 Avg Distance:  " << std::fixed << std::setprecision(2) 
              << (total_levenshtein_top1 / num_tests) << "\n";
    std::cout << "Top-3 Avg Distance:  " << std::fixed << std::setprecision(2) 
              << (total_levenshtein_top3 / num_tests) << "\n";
    std::cout << "Top-5 Avg Distance:  " << std::fixed << std::setprecision(2) 
              << (total_levenshtein_top5 / num_tests) << "\n";
    std::cout << "Top-10 Avg Distance: " << std::fixed << std::setprecision(2) 
              << (total_levenshtein_top10 / num_tests) << "\n";
    
    // Дополнительная метрика: процент случаев, когда расстояние ≤ 1 (почти правильное)
    std::cout << "\nSemantic Similarity (Distance ≤ 1):\n";
    std::cout << "-----------------------------------\n";
    std::cout << "Top-1 Semantic Match:  " << std::fixed << std::setprecision(2) 
              << (100.0 * (1 - (total_levenshtein_top1 / num_tests / 10.0))) << "%\n";
    std::cout << "(Lower distance = better semantic match)\n";
    std::cout << "===================================================\n";
}