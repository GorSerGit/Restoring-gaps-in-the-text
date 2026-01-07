#ifndef SIMILARITY_SCORER_H
#define SIMILARITY_SCORER_H

#include <string>
#include <unordered_map>

class SimilarityScorer {
private:
    const double MATCH_SCORE = 2.0;
    const double MISMATCH_SCORE = -1.0;
    
    std::unordered_map<std::string, int>* unigram_counts = nullptr;
    std::unordered_map<std::string, int>* bigram_counts = nullptr;
    
    int total_words = 0;
    int total_bigrams = 0;

public:
    void set_unigram_counts(std::unordered_map<std::string, int>* counts) { unigram_counts = counts; }
    void set_bigram_counts(std::unordered_map<std::string, int>* counts) { bigram_counts = counts; }
    void set_total_words(int total) { total_words = total; }
    void set_total_bigrams(int total) { total_bigrams = total; }
    
    double similarity_score(const std::string& gram1, const std::string& gram2);
    double calculate_pmi(const std::string& w1, const std::string& w2);
};

#endif