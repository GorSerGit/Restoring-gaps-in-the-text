#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <string>
#include <vector>
#include <unordered_map>

class NGramManager;
class SimilarityScorer;

class SequenceAlignmentPredictor {
private:
    std::unordered_map<std::string, int> word_to_idx;
    std::unordered_map<std::string, int> unigram_counts;
    
    NGramManager* ngram_manager;
    SimilarityScorer* similarity_scorer;
    
    const double GAP_PENALTY = -1.0;
    const double FREQ_WEIGHT = 0.3;
    const double NGRAM_WEIGHT = 0.7;
    
    int total_words = 0;
    
    double align_sequences(const std::vector<std::string>& seq1, 
                          const std::vector<std::string>& seq2);
    
    std::vector<std::string> extract_ngrams_from_context(
        const std::vector<std::string>& left_context,
        const std::vector<std::string>& right_context);

public:
    SequenceAlignmentPredictor();
    ~SequenceAlignmentPredictor();
    
    bool load_vocabulary(const std::string& filename);
    void build_statistics(const std::string& corpus_file);
    
    std::vector<std::pair<std::string, double>> predict_missing_word(
        const std::vector<std::string>& left_context,
        const std::vector<std::string>& right_context,
        int top_k = 10);
};

#endif