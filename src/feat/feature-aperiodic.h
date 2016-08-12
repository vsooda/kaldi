// feat/feature-aperiodic.h

// Copyright 2013  Arnab Ghoshal

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef KALDI_FEAT_FEATURE_APERIODIC_H_
#define KALDI_FEAT_FEATURE_APERIODIC_H_

#include <string>
#include <vector>

#include "feat/feature-functions.h"
#include "feat/mel-computations.h"
#include "feat/feature-window.h"

namespace kaldi {

/// AperiodicEnergyOptions contains basic options for computing the aperiodic
/// energy features.
struct AperiodicEnergyOptions {
  FrameExtractionOptions frame_opts;
  MelBanksOptions mel_opts;
  // TODO(arnab): add a class to generalize uniform & mel scales
  BaseFloat energy_floor;
    BaseFloat f0_max, f0_min, f0_width, quality_threshold;
  int32 max_iters;  // Max iterations to refine the aperiodic energy estimate
  int32 min_sq_error;  // Minimum squared error between the aperiodic spectrum
                       // estimates in successive iterations to stop iterating.
  int32 frame_diff_tolerance;  // Tolerate some difference between #frames
                               // extracted by Kaldi and get_f0. This will be
                               // ultimately removed when using Kaldi F0.
  bool use_hts_bands;
    bool debug_aperiodic;
   

  AperiodicEnergyOptions() : mel_opts(5),
                             energy_floor(FLT_EPSILON),
                             f0_max(350.0),
                             f0_min(70.0),
                             f0_width(0.1),
                             quality_threshold(0.4),
                             max_iters(100),
                             min_sq_error(1e-6),
                             frame_diff_tolerance(1),
	                     use_hts_bands(false),
                             debug_aperiodic(false) {
    frame_opts.round_to_power_of_two = false;
  }

  void Register(OptionsItf *po) {
    frame_opts.Register(po);
    mel_opts.Register(po);
    po->Register("energy-floor", &energy_floor,
                 "Floor on energy (absolute, not relative) in aperiodic energy "
                 "computation");
    po->Register("debug-aperiodic", &debug_aperiodic,
                 "Generate loads of debug information");
    po->Register("use-hts-bands", &use_hts_bands,
                 "Use HTS bands instead of triangular mel-frequency bands");
    po->Register("f0-max", &f0_max,
                 "Maximum F0 value (for lower end of cepstral comb lifter).");
    po->Register("f0-min", &f0_min,
                 "Minimum F0 value (for upper end of cepstral comb lifter).");
    po->Register("max-iters", &max_iters,
                 "Maximum number of reconstruction iterations.");
    po->Register("peak-width", &f0_width,
		 "Width of pitch cepstral peak, relative to pitch frequency.");
    po->Register("peak-quality", &quality_threshold,
                 "Minimum quality of Cepstral peak to use ceptral index instead of F0 one.");
    po->Register("min-sq-error", &min_sq_error,
                 "Minimum squared error between the aperiodic spectrum estimates"
                 " in successive iterations to stop iterating.");
  }
};


/**
 * Class for computing aperiodic energy features. This class implements the
 * algorithm described in B. Yegnanarayana, C. d'Alessandro, and V. Darsinos,
 * "An iterative algorithm for decomposition of speech signals into periodic
 * and aperiodic components," IEEE Trans. Speech & Audio Process., vol. 6(1),
 * pp 1-11, 1998.
 * The differences from the algorithm of the paper is that we operate on the
 * spectrum of the waveform itself, and not that of its LP residual. Also, we
 * are not explicitly doing F0 estimation as in the paper, but obtaining it from
 * a separate piece of code.
 */
class AperiodicEnergy {
 public:
  explicit AperiodicEnergy(const AperiodicEnergyOptions &opts);
  ~AperiodicEnergy();

  int32 Dim() { return opts_.mel_opts.num_bins; }

  void Compute(const VectorBase<BaseFloat> &wave,
               const VectorBase<BaseFloat> &voicing_prob,
               const VectorBase<BaseFloat> &f0,
               Matrix<BaseFloat> *output,
               Vector<BaseFloat> *wave_remainder = NULL);

 private:
  void IdentifyNoiseRegions(const VectorBase<BaseFloat> &power_spectrum,
                            BaseFloat f0,
                            std::vector<bool> *noise_indices);
  void ObtainNoiseSpectrum(const VectorBase<BaseFloat> &power_spectrum,
                           const std::vector<bool> &noise_indices,
                           Vector<BaseFloat> *noise_spectrum);
  void Compute_HTS_bands(const VectorBase<BaseFloat> &power_spectrum, 
			 Vector<BaseFloat> *output);

  AperiodicEnergyOptions opts_;
  FeatureWindowFunction feature_window_function_;
  SplitRadixRealFft<BaseFloat> *srfft_;
  MelBanks *mel_banks_;
  int32 padded_window_size_;

  KALDI_DISALLOW_COPY_AND_ASSIGN(AperiodicEnergy);
};

}  // namespace kaldi


#endif  // KALDI_FEAT_FEATURE_APERIODIC_H_
