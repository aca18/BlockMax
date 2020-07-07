#pragma once
/*
 ==============================================================================

Andrew C. Appelbaum (ACA) - July 2020

Implement a buffer with interpolation options on read

Essentially a port of AJAX SOUND CookDSP 'buffer.jsfx-inc'
<add link when I have internet again>

 ==============================================================================
*/

#include "IPlugPlatform.h"

class ACA_Buf
{
 public:
   ACA_Buf()
   {
   }
   
   ~ACA_Buf()
   {
     if (mBuf) {
       delete [] mBuf;
     }
   }

   void Reset(int max_size)
   {
     if (mBuf) {
       delete[] mBuf;
     }
     mSize = max_size;
     mTotalMemSlots = mSize + 6; //3 extra points at either end to help with interpolation algos; technically overkill
     mBuf = new double[mTotalMemSlots];
     mHead = 2;
     mTail = mHead + mSize;
     WriteAll(0.0);
   }

   void WriteAll (double val)
   {
     if (mBuf) {
       for (int i = 0; i < mTotalMemSlots; i++) {
         mBuf [i] = val;
       }
     }
   }
   
   void Write (double pos, double val)
   {
     if (mBuf && pos >= 0 && pos <= mSize) {
       mBuf [mHead + (int)pos] = val;
       //set extra points outside of normal range to help with interpolation
       mBuf [mTail] = mBuf [mHead];
       mBuf [mTail + 1] = mBuf [mHead + 1];
       mBuf [mHead - 1] = mBuf [mTail - 1];
     }
   }
   
   double Read (double pos) //read value without interpolation
   {
     if (mBuf && pos >= 0. && pos <= mSize) {
       return mBuf [mHead + (int) pos];
     } else {
       return 0.0;
     }
   }
   
   double Read2 (double pos) //read value with linear interpolation
   {
     if (mBuf && pos >= 0. && pos <= mSize) {
       pos += mHead;
       const int i = (int)pos;
       return mBuf [i] + (mBuf [i+1] - mBuf [i]) * (pos - (double)i);
     } else {
       return 0.0;
     };
   }
   
   double Read3 (double pos) //read value with cubic interpolation
   {
     if (mBuf && pos >= 0. && pos <= mSize) {
       pos += mHead;
       const int i = (int)pos;
       const double f = pos - (double)i;
       const double x0 = mBuf [i - 1];
       const double x1 = mBuf [i];
       const double x2 = mBuf [i + 1];
       const double x3 = mBuf [i + 2];
       //cubic interpolation algo obtained from AJAX SOUND COOK_DSP Library for Reaper JSFX
       double a3 = f * f;
       a3 -= 1.0;
       a3 *= (1.0 / 6.0);
       double a2 = (f + 1.0) * 0.5;
       double a0 = a2 - 1.0;
       double a1 = a3 * 3.0;
       a2 -= a1;
       a0 -= a3;
       a1 -= f;
       a0 *= f;
       a1 *= f;
       a2 *= f;
       a3 *= f;
       a1 += 1.0;
       return a0 * x0 + a1 * x1 + a2 * x2 + a3 * x3;
     } else {
       return 0.0;
     }
   }

   void SetSize(int newsize)
   {
     if (newsize < 1) {
       newsize = 1;
     }
     if (!mBuf || newsize > GetMaxSize()) {
       Reset(newsize);
     }
     mSize = newsize;
     mTail = mHead + mSize;
     mBuf[mTail] = mBuf[mHead];
     mBuf[mTail + 1] = mBuf[mHead + 1];
     mBuf[mHead - 1] = mBuf[mTail - 1];
   }

   int GetSize()
   {
     return mSize;
   }

   int GetMaxSize()
   {
     return mTotalMemSlots - 6;
   }

 private:
   int mTotalMemSlots = 0;
   int mSize = 0;
   double *mBuf = NULL;
   int mHead = 0;
   int mTail = 0;
};  