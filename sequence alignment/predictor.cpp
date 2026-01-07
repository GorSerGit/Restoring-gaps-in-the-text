#include "predictor.h"
#include "ngram_manager.h"
#include "similarity_scorer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <cmath>

SequenceAlignmentPredictor::SequenceAlignmentPredictor() {
    ngram_manager = new NGramManager();
    similarity_scorer = new SimilarityScorer();
}

SequenceAlignmentPredictor::~SequenceAlignmentPredictor() {
    delete ngram_manager;
    delete similarity_scorer;
}

bool SequenceAlignmentPredictor::load_vocabulary(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening vocabulary: " << filename << "\n";
        return false;
    }
    
    word_to_idx.clear();
    unigram_counts.clear();
    total_words = 0;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        int idx, freq;
        std::string word;
        
        if (iss >> idx >> word >> freq) {
            word_to_idx[word] = idx;
            unigram_counts[word] = freq;
            total_words += freq;
        }
    }
    
    similarity_scorer->set_unigram_counts(&unigram_counts);
    similarity_scorer->set_total_words(total_words);
    
    return true;
}

void SequenceAlignmentPredictor::build_statistics(const std::string& corpus_file) {
    ngram_manager->build_statistics(corpus_file);
    similarity_scorer->set_bigram_counts(ngram_manager->get_bigram_counts_ptr());
    similarity_scorer->set_total_bigrams(ngram_manager->get_total_bigrams());
}

double SequenceAlignmentPredictor::align_sequences(const std::vector<std::string>& seq1, 
                                                  const std::vector<std::string>& seq2) {
    int n = seq1.size();
    int m = seq2.size();
    
    std::vector<std::vector<double>> dp(n + 1, std::vector<double>(m + 1, 0.0));
    
    for (int i = 1; i <= n; ++i) {
        dp[i][0] = dp[i-1][0] + GAP_PENALTY;
    }
    for (int j = 1; j <= m; ++j) {
        dp[0][j] = dp[0][j-1] + GAP_PENALTY;
    }
    
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            double match = dp[i-1][j-1] + similarity_scorer->similarity_score(seq1[i-1], seq2[j-1]);
            double del = dp[i-1][j] + GAP_PENALTY;
            double ins = dp[i][j-1] + GAP_PENALTY;
            dp[i][j] = std::max({match, del, ins});
        }
    }
    
    return dp[n][m];
}

std::vector<std::string> SequenceAlignmentPredictor::extract_ngrams_from_context(
    const std::vector<std::string>& left_context,
    const std::vector<std::string>& right_context) {
    
    std::vector<std::string> ngrams;
    
    for (size_t i = 0; i + 1 < left_context.size(); ++i) {
        ngrams.push_back(left_context[i] + " " + left_context[i+1]);
    }
    
    for (size_t i = 0; i + 1 < right_context.size(); ++i) {
        ngrams.push_back(right_context[i] + " " + right_context[i+1]);
    }
    
    if (!left_context.empty() && !right_context.empty()) {
        ngrams.push_back(left_context.back() + " " + right_context.front());
    }
    
    return ngrams;
}

std::vector<std::pair<std::string, double>> SequenceAlignmentPredictor::predict_missing_word(
    const std::vector<std::string>& left_context,
    const std::vector<std::string>& right_context,
    int top_k) {
    
    std::vector<std::pair<std::string, double>> predictions;
    
    if (word_to_idx.empty()) {
        return predictions;
    }
    
    std::vector<std::string> context_ngrams = extract_ngrams_from_context(left_context, right_context);
    
    for (const auto& word_entry : word_to_idx) {
        const std::string& candidate = word_entry.first;
        
        if (std::find(left_context.begin(), left_context.end(), candidate) != left_context.end() ||
            std::find(right_context.begin(), right_context.end(), candidate) != right_context.end()) {
            continue;
        }
        
        std::vector<std::string> candidate_ngrams;
        
        if (!left_context.empty()) {
            candidate_ngrams.push_back(left_context.back() + " " + candidate);
        }
        
        if (!right_context.empty()) {
            candidate_ngrams.push_back(candidate + " " + right_context.front());
        }
        
        double alignment_score = 0.0;
        if (!context_ngrams.empty() && !candidate_ngrams.empty()) {
            alignment_score = align_sequences(context_ngrams, candidate_ngrams);
            alignment_score /= std::max(context_ngrams.size(), candidate_ngrams.size());
        }
        
        double freq_score = 0.0;
        auto freq_it = unigram_counts.find(candidate);
        if (freq_it != unigram_counts.end()) {
            freq_score = log(static_cast<double>(freq_it->second) + 1);
        }
        
        double final_score = NGRAM_WEIGHT * alignment_score + FREQ_WEIGHT * freq_score;
        predictions.emplace_back(candidate, final_score);
    }
    
    std::sort(predictions.begin(), predictions.end(),
             [](const std::pair<std::string, double>& a, 
                const std::pair<std::string, double>& b) {
                 return a.second > b.second;
             });
    
    if (top_k > 0 && predictions.size() > static_cast<size_t>(top_k)) {
        predictions.resize(top_k);
    }
    
    return predictions;
}