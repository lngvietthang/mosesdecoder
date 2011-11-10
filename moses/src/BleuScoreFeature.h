#ifndef BLUESCOREFEATURE_H
#define BLUESCOREFEATURE_H

#include <map>
#include <utility>
#include <string>
#include <vector>

#include "FeatureFunction.h"

#include "FFState.h"
#include "Phrase.h"

namespace Moses {

class BleuScoreFeature;

class BleuScoreState : public FFState {
public:
    friend class BleuScoreFeature;
    static size_t bleu_order;

    BleuScoreState();
    virtual int Compare(const FFState& other) const;
    void print(std::ostream& out) const;

private:
    Phrase m_words;
    size_t m_source_length;
    size_t m_target_length;

    // scaled reference length is needed for scoring incomplete hypotheses against reference translation
    float m_scaled_ref_length;

    std::vector< size_t > m_ngram_counts;
    std::vector< size_t > m_ngram_matches;
};

std::ostream& operator<<(std::ostream& out, const BleuScoreState& state);

typedef std::map< Phrase, size_t > NGrams;

class BleuScoreFeature : public StatefulFeatureFunction {
public:
	BleuScoreFeature():
	                                 StatefulFeatureFunction("BleuScore",1),
	                                 m_count_history(BleuScoreState::bleu_order),
	                                 m_match_history(BleuScoreState::bleu_order),
	                                 m_source_length_history(0),
	                                 m_target_length_history(0),
	                                 m_ref_length_history(0),
	                                 m_scale_by_input_length(true),
	                                 m_scale_by_ref_length(false),
	                                 m_scale_by_avg_length(false),
	                                 m_scale_by_x(1),
	                                 m_historySmoothing(0.7),
	                                 m_smoothing_scheme(PLUS_ONE) {}

	BleuScoreFeature(bool scaleByInputLength, bool scaleByTargetLength, bool scaleByAvgLength, float scaleByX, float historySmoothing):
	                                 StatefulFeatureFunction("BleuScore",1),
	                                 m_count_history(BleuScoreState::bleu_order),
	                                 m_match_history(BleuScoreState::bleu_order),
	                                 m_source_length_history(0),
	                                 m_target_length_history(0),
	                                 m_ref_length_history(0),
	                                 m_scale_by_input_length(scaleByInputLength),
	                                 m_scale_by_ref_length(scaleByTargetLength),
	                                 m_scale_by_avg_length(scaleByAvgLength),
	                                 m_scale_by_x(scaleByX),
	                                 m_historySmoothing(historySmoothing),
	                                 m_smoothing_scheme(PLUS_ONE) {}

    std::string GetScoreProducerDescription() const
    {
    	return "BleuScoreFeature";
    }

    std::string GetScoreProducerWeightShortName(unsigned) const
    {
        return "bl";
    }

    void PrintHistory(std::ostream& out) const;
    void LoadReferences(const std::vector< std::vector< std::string > > &);
    void SetCurrentSourceLength(size_t);
    void SetCurrentReference(size_t);
    void UpdateHistory(const std::vector< const Word* >&);
    void UpdateHistory(const std::vector< std::vector< const Word* > >& hypos, std::vector<size_t>& sourceLengths, std::vector<size_t>& ref_ids, size_t rank, size_t epoch);
    void PrintReferenceLength(const std::vector<size_t>& ref_ids);
    size_t GetReferenceLength(size_t ref_id);
    void SetBleuSmoothingScheme(size_t scheme);
    void GetNgramMatchCounts(Phrase&,
                             const NGrams&,
                             std::vector< size_t >&,
                             std::vector< size_t >&,
                             size_t skip = 0) const;
    void GetClippedNgramMatchesAndCounts(Phrase&,
                                 const NGrams&,
                                 std::vector< size_t >&,
                                 std::vector< size_t >&,
                                 size_t skip = 0) const;

    FFState* Evaluate( const Hypothesis& cur_hypo, 
                       const FFState* prev_state, 
                       ScoreComponentCollection* accumulator) const;
    virtual FFState* EvaluateChart( const ChartHypothesis& /* cur_hypo */,
                                    int /* featureID */,
                                    ScoreComponentCollection* ) const
                                    {
                                      /* Not implemented */
                                      assert(0);
                                    }
    float CalculateBleu(BleuScoreState*) const;
    const FFState* EmptyHypothesisState(const InputType&) const;

private:
    // counts for pseudo-document
    std::vector< float > m_count_history;
    std::vector< float > m_match_history;
    float m_source_length_history;
    float m_target_length_history;
    float m_ref_length_history;

    size_t m_cur_source_length;
    std::map< size_t, std::pair< size_t, NGrams > > m_refs;
    NGrams m_cur_ref_ngrams;
    size_t m_cur_ref_length;

    // scale BLEU score by history of input length
    bool m_scale_by_input_length;

    // scale BLEU score by (history of) reference length
    bool m_scale_by_ref_length;

    // scale BLEU score by (history of) the average of input and reference length
    bool m_scale_by_avg_length;

    float m_scale_by_x;

    // smoothing factor for history counts
    float m_historySmoothing;

    enum SmoothingScheme { PLUS_ONE = 1, LIGHT = 2, PAPINENI = 3 };
    SmoothingScheme m_smoothing_scheme;
};

} // Namespace.

#endif //BLUESCOREFEATURE_H
