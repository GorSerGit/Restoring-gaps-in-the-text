#include "similarity_scorer.h"
#include <sstream>
#include <cmath>

double SimilarityScorer::similarity_score(const std::string& gram1, const std::string& gram2) {
    if (gram1 == gram2) return MATCH_SCORE;
    
    if (gram1.find(' ') != std::string::npos && gram2.find(' ') != std::string::npos) {
        std::istringstream iss1(gram1), iss2(gram2);
        std::string w11, w12, w21, w22;
        iss1 >> w11 >> w12;
        iss2 >> w21 >> w22;
        
        double score = 0.0;
        if (w11 == w21) score += 0.5;
        if (w12 == w22) score += 0.5;
        
        double pmi1 = calculate_pmi(w11, w12);
        double pmi2 = calculate_pmi(w21, w22);
        score += 0.3 * std::min(pmi1, pmi2);
        
        return score;
    }
    
    return MISMATCH_SCORE;
}

double SimilarityScorer::calculate_pmi(const std::string& w1, const std::string& w2) {
    if (!bigram_counts || !unigram_counts) return 0.0;
    
    std::string bigram = w1 + " " + w2;
    
    auto bigram_it = bigram_counts->find(bigram);
    if (bigram_it == bigram_counts->end()) return 0.0;
    
    auto unigram1_it = unigram_counts->find(w1);
    auto unigram2_it = unigram_counts->find(w2);
    
    if (unigram1_it == unigram_counts->end() || unigram2_it == unigram_counts->end()) 
        return 0.0;
    
    double p_xy = static_cast<double>(bigram_it->second) / total_bigrams;
    double p_x = static_cast<double>(unigram1_it->second) / total_words;
    double p_y = static_cast<double>(unigram2_it->second) / total_words;
    
    if (p_x == 0 || p_y == 0) return 0.0;
    
    return log(p_xy / (p_x * p_y));
}