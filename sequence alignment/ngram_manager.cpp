#include "ngram_manager.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

void NGramManager::build_statistics(const std::string& corpus_file) {
    std::ifstream file(corpus_file);
    if (!file) {
        std::cerr << "Error opening corpus: " << corpus_file << "\n";
        return;
    }
    
    bigram_counts.clear();
    trigram_counts.clear();
    total_bigrams = 0;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i + 1 < tokens.size()) {
                std::string bigram = tokens[i] + " " + tokens[i+1];
                bigram_counts[bigram]++;
                total_bigrams++;
            }
            
            if (i + 2 < tokens.size()) {
                std::string trigram = tokens[i] + " " + tokens[i+1] + " " + tokens[i+2];
                trigram_counts[trigram]++;
            }
        }
    }
}