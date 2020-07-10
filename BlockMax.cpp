#include "BlockMax.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <iostream>
#include <ctime>

BlockMax::BlockMax(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kWindowLength)->InitDouble("Window Length", 100., 1., 100., 0.1, "ms");

  GetParam(kOverlapTime)->InitDouble("Grain Overlap", 0., 0., 50., 0.1, "%");
  GetParam(kGlitchOnOff)->InitBool("Glitch", false);
  //GetParam(kFadeInTime)->InitDouble("Fade In", 0., 0., 100., 0.1, "%");
  //GetParam(kFadeOutTime)->InitDouble("Fade Out", 0., 0., 100., 0.1, "%");

  GetParam(kGain)->InitDouble("Gain", 0., -24., 0., 0.01, "dB");
  GetParam(kPdcOnOff)->InitBool("PDC", true);
  //----------stuff for DSP
  mInputCount = 0;
  for (int i = 0; i < mNumGrains; i++) {
    mGrains[i].SetLooping(false);                  //my instinct is to make false, but curious if looping gets rid of weird crackle
    mGrains[i].SetSampleRate(GetSampleRate());
    mGrains[i].SetFades(0., 0., 1.0, 1.0);
    mGrains[i].SetLengthInMs(GetParam(kWindowLength)->Value());
    mGrains[i].SetPlaybackPosition(mBufPosForGrains);
    if (i % 2 == 0) {//right channel grains
      mGrains[i].AssignBuffer(&mBuf2);
    }
    else {//left channel grains
      mGrains[i].AssignBuffer(&mBuf1);
    }
  }
  BlockMax::OnReset();
  std::srand(std::time(0));  //seed c++ random number generator
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

    const IRECT leftGrid = ctrlRect.GetGridCell(0, 1, 5, EDirection::Horizontal, 2);
    const IRECT centerGrid = ctrlRect.GetGridCell(2, 1, 5, EDirection::Horizontal, 2);
    const IRECT rightGrid = ctrlRect.GetGridCell(4, 1, 5, EDirection::Horizontal, 1);

    /*
    const IRECT centerTop = centerGrid.GetGridCell(0, 3, 1, EDirection::Vertical, 1);
    const IRECT centerMid = centerGrid.GetGridCell(1, 3, 1, EDirection::Vertical, 1);
    const IRECT centerBot = centerGrid.GetGridCell(2, 3, 1, EDirection::Vertical, 1);
    */
    const IRECT leftTopTwo = leftGrid.GetGridCell(0, 3, 1, EDirection::Vertical, 2);
    const IRECT leftBot = leftGrid.GetGridCell(2, 3, 1, EDirection::Vertical, 1);

    const IRECT centerTopTwo = centerGrid.GetGridCell(0, 3, 1, EDirection::Vertical, 2);
    const IRECT centerBot = centerGrid.GetGridCell(2, 3, 1, EDirection::Vertical, 1);

    const IRECT rightTopTwo = rightGrid.GetGridCell(0, 3, 1, EDirection::Vertical, 2);
    const IRECT rightBot = rightGrid.GetGridCell(2, 3, 1, EDirection::Vertical, 1);

    pGraphics->AttachControl(new ITextControl(titleRect, "-  B L O C K    M A X  -", IText(20)));
    pGraphics->AttachControl(new IVSliderControl(leftTopTwo.GetScaledAboutCentre(0.9), kWindowLength, "", DEFAULT_STYLE, true, EDirection::Horizontal));
    
    pGraphics->AttachControl(new IVSliderControl(centerTopTwo.GetScaledAboutCentre(0.9), kOverlapTime, "", DEFAULT_STYLE, true, EDirection::Horizontal));
    pGraphics->AttachControl(new IVToggleControl(centerBot, kGlitchOnOff));
    /*
    pGraphics->AttachControl(new IVKnobControl(centerTop, kOverlapTime));
    pGraphics->AttachControl(new IVKnobControl(centerMid, kFadeInTime));
    pGraphics->AttachControl(new IVKnobControl(centerBot, kFadeOutTime));
    */
    pGraphics->AttachControl(new IVKnobControl(rightTopTwo, kGain));
    pGraphics->AttachControl(new IVToggleControl(rightBot, kPdcOnOff));
  };
#endif
}

#if IPLUG_DSP
void BlockMax::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  mProcessFlagForPDC = true;
  const int nChans = NOutChansConnected();
  const int numOverlapSamples = (int)(GetSampleRate() * mOverlapTime * 0.001);

  //write to audio buffers, and count up.  Once count exceeds the window length,
  //switch to new grains(s) and pass them the correct position to read from.
  //calculate the current gain factor to apply to all samples from the grain.
  //will need to modify some things to implement window overlap and/or length jitter
  for (int s = 0; s < nFrames; s++) {
    mBuf1.Write(mCurrBufPos, inputs[0][s]);
    if (nChans > 1) {
      mBuf2.Write(mCurrBufPos, inputs[1][s]);
    }
    mInputCount++;
    if (mInputCount >= mNumSamplesPerWindow - numOverlapSamples) {
      mInputCount = 0;
      mCurrGrainL = (mCurrGrainL + 2) % mNumGrains;
      mCurrGrainR = (mCurrGrainR + 2) % mNumGrains;
      mGrains[mCurrGrainL].SetPlaybackPosition(mCurrBufPos - mNumSamplesPerWindow + numOverlapSamples);
      mGrains[mCurrGrainR].SetPlaybackPosition(mCurrBufPos - mNumSamplesPerWindow + numOverlapSamples);
      mGrains[mCurrGrainL].ResetWindow();
      mGrains[mCurrGrainR].ResetWindow();
      mGrainGainFactors[mCurrGrainL] = mPeakTarget / mGrains[mCurrGrainL].GetAbsMax();
      mGrainGainFactors[mCurrGrainR] = mPeakTarget / mGrains[mCurrGrainR].GetAbsMax();
    }

    //read from grains and scale by gain factor
    //since grains are set to not loop, out-of-bounds index requests
    //should just return 0.0
    for (int i = 0; i < mNumGrains - 1; i += 2) {
      outputs[0][s] += mGrains[i].Process() * mGrainGainFactors[i];
      if (nChans > 1) {
        outputs[1][s] += mGrains[i + 1].Process() * mGrainGainFactors[i + 1];
      }
    }

    mCurrBufPos = mCurrBufPos + 1.;
    if (mCurrBufPos > mBuf1.GetSize()) {
      mCurrBufPos = 0.;
    }
  }
}

void BlockMax::OnReset()
{
  const double bufferLength = 2000.; //dummy val for now;  ms
  mBuf1.Reset(GetSampleRate() * 0.001 * bufferLength);
  mBuf2.Reset(GetSampleRate() * 0.001 * bufferLength);
  for (int i = 0; i < mNumGrains; i++) {
    mGrains[i].SetSampleRate(GetSampleRate());
    mGrains[i].ResetWindow();
  }
  mNumSamplesPerWindow = GetSampleRate() * GetParam(kWindowLength)->Value() * 0.001;
}

void BlockMax::OnParamChange(int paramIdx)
{
  switch (paramIdx) {
  case kGain:
  {
    mPeakTarget = DBToAmp(GetParam(kGain)->Value());
    std::cout<<mPeakTarget<<"/n";
    break;
  }
  case kWindowLength:
  case kOverlapTime:
  case kFadeInTime:
  case kFadeOutTime:
  {
    const double newLength = GetParam(kWindowLength)->Value();
    for (int i = 0; i < mNumGrains; i++) {
      mGrains[i].SetLengthInMs(newLength);
    }
    mNumSamplesPerWindow = GetSampleRate() * newLength * 0.001;
    if (mProcessFlagForPDC && mPDCisEnabled) {
      try {
        SetLatency(mNumSamplesPerWindow);
      }
      catch (...) {}
    }
    //overlap time
    if (!mGlitchIsEnabled) {
      mOverlapTime = GetParam(kOverlapTime)->Value() * 0.01 * newLength;
    }
    //fade times -- set equal to overlap time
    //const double windowLength = GetParam(kWindowLength)->Value();
    const double fadeInLength = GetParam(kOverlapTime)->Value() * 0.01;
    const double fadeOutLength = GetParam(kOverlapTime)->Value() * 0.01;

    for (int i = 0; i < mNumGrains; i++) {
      mGrains[i].SetFades(fadeInLength, fadeOutLength, 1.0, 1.0);
    }
    break;
  }
  case kPdcOnOff:
  {
    mPDCisEnabled = GetParam(kPdcOnOff)->Value();
    if (mProcessFlagForPDC) {
      if (mPDCisEnabled) {
        try {
          SetLatency(mNumSamplesPerWindow);
        }
        catch (...) {}
      }
      else {
        try {
          SetLatency(0);
        }
        catch (...) {}
      }
    }
    break;
  }
  case kGlitchOnOff:
  {
    mGlitchIsEnabled = GetParam(kGlitchOnOff)->Value();
    if (mGlitchIsEnabled) {
      double randNum = (double)std::rand() / RAND_MAX;  //random between 0-1
      mOverlapTime = 1. + 2. * randNum * GetParam(kWindowLength)->Value();
      randNum = (double)std::rand() / RAND_MAX;
      const double randNum2 = (double)std::rand() / RAND_MAX;
      for (int i = 0; i < mNumGrains; i++) {
        mGrains[i].SetFades(randNum, randNum2, 1.0, 1.0);
      }
    }
    else {
      mOverlapTime = GetParam(kOverlapTime)->Value() * 0.01 * GetParam(kWindowLength)->Value();
      const double fadeInLength = GetParam(kOverlapTime)->Value() * 0.01;
      const double fadeOutLength = GetParam(kOverlapTime)->Value() * 0.01;
      for (int i = 0; i < mNumGrains; i++) {
        mGrains[i].SetFades(fadeInLength, fadeOutLength, 1.0, 1.0);
      }
    }
    break;
  }
  }
}

//-------------------------
#endif
