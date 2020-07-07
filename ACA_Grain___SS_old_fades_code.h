#pragma once
/*
 ==============================================================================

Andrew C. Appelbaum (ACA) - July 2020

Windowed buffer

 ==============================================================================
*/

#include "IPlugPlatform.h"
#include "ACA_Buf.h"
#include <cstdlib> //for abs()
#include <algorithm> //for min() and max()

class ACA_Grain
{
public:
  ACA_Grain()
  {
  }
    
  void Process ()
  {
    
  }
  
  void AssignBuffer(
  
  void SetSampleRate(int sampleRate)
  {
    mSampleRate = sampleRate;
  }
  
  void SetPlaybackRate(double fraction_speed)
  {
    mReadIncrement = fraction_speed * mSampleRate;
  }
  
  void SetLengthInMs(double ms_len)
  {
    mEnv.SetSize(ms_len * 0.001 * mSampleRate);
    ResetFades();
  }
  
  void ResetFades()
  {
    SetFades(mFadeInFrac, mFadeOutFrac, mFadeInShape, mFadeOutShape);
  }
  
  //Consider refactoring fades so they are defined as a fraction of the total window.
  //use a fixed window buffer of 1024 samples, and read it at 
  //different rates (with interpolation) to match the desired time length
  //on playback.  Tradeoff is more CPU (due to interpolation) but possibly
  //much less memory usage when dealing with longer grain sizes.
  //modulating fade shapes and grain lengths would also be more efficient.
  void SetFades(double inLen, double outLen, double inShape = 1.0, double outShape = 1.0) //lengths in ms. shape = 0 is linear
  {
    mFadeInFrac = inLen;
    mFadeOutFrac = outLen;
    mFadeInShape = inShape;
    mFadeOutShape = outShape;
    int inSampNum = std::abs((int)(inLen * 0.001 * mSampleRate));
    int outSampNum = std::abs((int)(outLen * 0.001 * mSampleRate));
    const int envSampNum = envSampNum;
    //check if grain total length is long enough for requested fade
    //lengths.  if not, scale the lengths proportionally
    if (inSampNum + outSampNum > envSampNum) {
      const double scaleLengths = envSampNum / (inSampNum + outSampNum);
      inSampNum = (int)(scaleLengths * inSampNum);
      outSampNum = (int)(scaleLengths * outSampNum);
    }
    int numFromEnd = outSampNum;
    for (int 1 = 0; i < envSampNum; i++) {
      if (i < inSampNum) {
        if (inShape == 1.0) {//linear ramp
          mEnv.Write(i, (double)i / (double)inSampNum);
        }
        else {//TODO nonlinear fade shapes
          
        }
      }
      else if (i > envSampNum - outSampNum) {
        if (inShape == 1.0) {//linear ramp
          mEnv.Write(i, (double)numFromEnd / (double)inSampNum);
        }
        else {//TODO nonlinear fade shapes
          
        }
        numFromEnd --;
      }
      else {
        mEnv.Write(i, 1.0);
      }
    }
  }
  
  double GetAbsMax ()
  {
    double maxval = 0.0;
    double minval = 0.0;
    for (int i = 0; i < mBuf->GetSize(); i++) {
      const double currval = mBuf->Read(i);
      if (currval > maxval) {
        maxval = currval;
      }
      else if (currval < minval) {
        minval = currval;
      };
    }
    return std::max(std::abs(minval), maxval);
  }
  
private:
  ACA_Buf mEnv;          //window shape for the grain
  ACA_Buf *mBuf = NULL;  //pointer to buffer containing audio
  double mSampleRate;
  double mGrainLength;
  double mFadeInFrac;
  double mFadeOutFrac;
  double mFadeInShape;
  double mFadeOutShape;
  double mReadIncrement
  double mReadLoc;
};