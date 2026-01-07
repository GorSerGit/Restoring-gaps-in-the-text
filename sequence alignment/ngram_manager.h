#ifndef NGRAM_MANAGER_H
#define NGRAM_MANAGER_H

#include <string>
#include <unordered_map>

class NGramManager {
private:
    std::unordered_map<std::string, int> bigram_counts;
    std::unordered_map<std::string, int> trigram_counts;
    int total_bigrams = 0;

public:
    void build_statistics(const std::string& corpus_file);
    
    std::unordered_map<std::string, int>& get_bigram_counts() { return bigram_counts; }
    std::unordered_map<std::string, int>* get_bigram_counts_ptr() { return &bigram_counts; }
    std::unordered_map<std::string, int>& get_trigram_counts() { return trigram_counts; }
    int get_total_bigrams() const { return total_bigrams; }
};

#endif