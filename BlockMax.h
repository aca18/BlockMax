#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "ACA_Buf.h"

const int kNumPresets = 1;

enum EParams
{
  kWindowLength = 0,
  kGain,
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
  ACA_Grain mGrains[6];
  int mNumGrains = 6;
  double mGrainGainFactors[6] = { 0., 0., 0., 0., 0., 0.};
  int mCurrGrainL = 0;
  int mCurrGrainR = 1;
  double mPeakTarget = 1.0;
  double mCurrBuffPos = 0.0;
  double mBuffPosForGrains = 0.0;
#endif
};
