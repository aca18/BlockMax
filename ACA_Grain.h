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
    mEnv.SetSize(1024);
    mEnv.WriteAll(1.0);
  }
  
  double Process ()
  {
    const double retval = mEnv.Read3(mEnvReadLoc) * mBuf->Read3(mReadLoc);
    mEnvReadLoc += mEnvReadIncrement;
    mReadLoc += mReadIncrement;
    if (mReadLoc >= mBuf->GetSize()) {
      mReadLoc = 0.0;
    }
    if (mLoop) {
      if (mEnvReadLoc > mEnv.GetSize()) {
        mEnvReadLoc = 0.;
        mReadLoc = mInitReadLoc;
      }
    }
    return retval;
  }
  
  void SetPlaybackPosition (double newpos)
  {
    mInitReadLoc = newpos;
    mReadLoc = newpos;
  }

  void SetLooping(bool loop_is_true)
  {
    mLoop = loop_is_true;
  }

  void ResetWindow ()
  {
    mEnvReadLoc = 0.0;
  }
  
  void AssignBuffer(ACA_Buf *audio_buf)
  {
    mBuf = audio_buf;
  }
  
  void SetSampleRate(int sampleRate)
  {
    mSampleRate = sampleRate;
    SetLengthInMs(mGrainLength * 1000.);
  }
  
  void SetPlaybackRate(double fraction_speed)
  {
    mReadIncrement = fraction_speed;
    mEnvReadIncrement = mReadIncrement * mEnv.GetSize() / (mSampleRate * mGrainLength);
  }
  
  void SetLengthInMs(double ms_len)
  {
    mGrainLength = ms_len * 0.001;
    //mEnv.SetSize(mGrainLength * mSampleRate);
    SetPlaybackRate(mReadIncrement); //this updates mEnvReadIncrement
  }
  
  //Fades are defined as a fraction of the total window.
  //use a fixed window buffer of 1024 samples, and read it at 
  //different rates (with interpolation) to match the desired time length
  //on playback.  Tradeoff is more CPU (due to interpolation) but possibly
  //much less memory usage when dealing with longer grain sizes.
  //WIth this design, modulating fade shapes and grain lengths is also more efficient.
  void SetFades(double inFrac, double outFrac, double inShape = 1.0, double outShape = 1.0) //lengths in ms. shape = 0 is linear
  {
    mFadeInFrac = inFrac;
    mFadeOutFrac = outFrac;
    mFadeInShape = inShape;
    mFadeOutShape = outShape;
    const int envSampNum = mEnv.GetSize();
    int inSampNum = std::abs((int)(inFrac * envSampNum));
    int outSampNum = std::abs((int)(outFrac * envSampNum));
    
    //check if grain total length is long enough for requested fade
    //lengths.  if not, scale the lengths proportionally
    if (inSampNum + outSampNum > envSampNum) {
      const double scaleLengths = envSampNum / (inSampNum + outSampNum);
      inSampNum = (int)(scaleLengths * inSampNum);
      outSampNum = (int)(scaleLengths * outSampNum);
    }
    int numFromEnd = outSampNum;
    for (int i = 0; i < envSampNum; i++) {
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
    for (int i = 0; i < mGrainLength * mSampleRate; i++) {
      const double currval = mBuf->Read(i + mInitReadLoc);
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
  double mSampleRate = 44100.;
  double mGrainLength = 0.1;   //in seconds
  double mFadeInFrac = 0.1;
  double mFadeOutFrac = 0.1;
  double mFadeInShape = 1.0;
  double mFadeOutShape = 1.0;
  double mReadIncrement = 1.0;
  double mEnvReadIncrement = 1024./4410.;
  double mInitReadLoc = 0.0;
  double mReadLoc = 0.0;
  double mEnvReadLoc = 0.0;
  bool mLoop = true;
};