#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "ACA_Buf.h"
#include "ACA_Grain.h"
#include "SampleDelay.h"

const int kNumPresets = 1;

enum EParams
{
  kWindowLength = 0,
  kGain,
  kPdcOnOff,
  kGlitchOnOff,
  kOverlapTime,
  kFadeInTime,
  kFadeOutTime,
  kDry,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class BlockMax final : public Plugin
{
public:
  BlockMax(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
private:
  ACA_Buf mBuf1;
  ACA_Buf mBuf2;
  ACA_SampleDelay mDelayL;
  ACA_SampleDelay mDelayR;
  int mPdcBlockLength = 2048;
  ACA_Grain mGrains[6];
  int mNumGrains = 6;
  double mGrainGainFactors[6] = { 0., 0., 0., 0., 0., 0.};
  int mCurrGrainL = 0;
  int mCurrGrainR = 1;
  double mPeakTarget = 1.0;
  double mDryGain = 1.0;
  double mCurrBufPos = 0.0;
  double mBufPosForGrains = 0.0;
  int mNumSamplesPerWindow = 4410;
  int mOverlapSamples = 0;
  int mInputCount;
  bool mProcessFlagForPDC = false; //set to true in Process() to say it is ok to call SetLatency()
  bool mPDCisEnabled = true;
  bool mGlitchIsEnabled = false;
  double mFadeInTime = 1.; //in ms
  double mFadeOutTime = 1.; //in ms
  double mOverlapTime = 0.; //in ms
#endif
};
