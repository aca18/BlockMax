#include "BlockMax.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

BlockMax::BlockMax(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kWindowLength)->InitDouble("Window Length", 100., 1., 500., 0.1, "ms");
  GetParam(kGain)->InitDouble("Peak Target", 0., -24., 0., 0.01, "dB");


  //----------stuff for DSP
  for (int i = 0; i < mNumGrains; i++) {
    mGrains[i].SetLooping(false);
    mGrains[i].SetSampleRate(GetSamplePos);
    mGrains[i].SetFades(0., 0., 1.0, 1.0);
    mGrains[i].SetLengthInMs(GetParam(kWindowLength)->Value());
    if (i % 2 == 0) {//right channel grains
      mGrains[i].AssignBuffer(&mBuf2);
    }
    else {//left channel grains
      mGrains[i].AssignBuffer(&mBuf1);
    }
  }
  //TODO:  set plugin latency for current sample rate and window length
  //----------
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    //pGraphics->AttachPanelBackground(IColor::GetRandomColor ());
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    const IRECT titleRect = b.FracRectVertical(0.1, true).GetScaledAboutCentre(0.6);
    const IRECT ctrlRect = b.FracRectVertical(0.9, false).GetScaledAboutCentre(0.9);
    const IRECT leftGrid = ctrlRect.GetGridCell(0, 1, 2, EDirection::Horizontal, 1);
    const IRECT rightGrid = ctrlRect.GetGridCell(1, 1, 2, EDirection::Horizontal, 1);
    
    pGraphics->AttachControl(new ITextControl(titleRect, "-  B L O C K || M A X  -", IText(20)));
    pGraphics->AttachControl(new IVKnobControl(leftGrid, kWindowLength));
    pGraphics->AttachControl(new IVKnobControl(rightGrid, kGain));
  };
#endif
}

#if IPLUG_DSP
void BlockMax::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();

  //write to audio buffers, and count up.  Once count exceeds the window length,
  //switch to new grains(s) and pass them the correct position to read from.
  //calculate the current gain factor to apply to all samples from the grain.
  
  for (int s = 0; s < nFrames; s++) {


    if (nChans > 1) {

      outputs[1][s] = mGrains[mCurrGrainR].Process() * mGrainGainFactors[mCurrGrainR];
    }

  }
}

void BlockMax::OnReset()
{
  const double windowLength = 1000.; //dummy val for now;  ms
  mBuf1.Reset(GetSampleRate() * 0.001 * windowLength);
  mBuf2.Reset(GetSampleRate() * 0.001 * windowLength);
  for (i = 0; i < mNumGrains; i++) {
    mGrains[i].SetSampleRate(GetSampleRate());
  }
  //TODO:  set plugin latency for current sample rate and window length
}

void BlockMax::OnParamChange(int paramIdx)
{
  switch (paramIdx) {
  case kGain:
  {
    mPeakTarget = DBToAmp(GetParam(kGain)->Value());
    break;
  }
  case kWindowLength:
  {
    const double newLength = GetParam(kWindowLength)->Value();
    for (int i = 0; i < mNumGrains; i++) {
      mGrains[i].SetLengthInMs(newLength);
    }
    //TODO:  set plugin latency for current sample rate and window length
    break;
  }
  }
}

//-------------------------
#endif
