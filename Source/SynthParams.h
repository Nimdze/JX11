// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

    // All per-block control values written by the processor on the audio thread.                      
   // Plain POD: no JUCE, no atomics, no allocation. Synth reads this from the                        
   // audio thread only.                                                                              
   struct SynthParams                                                                                 
   {                                                                                                  
       // master / voicing                                                                            
       int   numVoices          = 1;                                                                  
       float volumeTrim         = 1.0f;                                                               
       float noiseMix           = 0.0f;                                                               
       float oscMix             = 0.0f;                                                               
       float velocitySensitivity = 1.0f;                                                              
       bool  ignoreVelocity     = false;                                                              
                                                                                                      
       // oscillators / tuning                                                                        
       float tune               = 0.0f;                                                               
       float detune             = 1.0f;                                                               
                                                                                                      
       // amp envelope                                                                                
       float envAttack          = 0.0f;                                                               
       float envDecay           = 0.0f;                                                               
       float envSustain         = 1.0f;                                                               
       float envRelease         = 0.0f;                                                               
                                                                                                      
       // glide                                                                                       
       int   glideMode          = 0;                                                                  
       float glideRate          = 1.0f;                                                               
       float glideBend          = 1.0f;                                                               
                                                                                                      
       // LFO / vibrato                                                                               
       float lfoInc             = 0.0f;                                                               
       float vibrato            = 0.0f;                                                               
       float pwmDepth           = 0.0f;                                                               
                                                                                                      
       // filter + filter envelope                                                                    
       float filterKeyTracking  = 1.0f;                                                               
       float filterQ            = 1.0f;                                                               
       float filterLFODepth     = 0.0f; 

       float filterAttack       = 0.0f;                                                               
       float filterDecay        = 0.0f;                                                               
       float filterSustain      = 1.0f;                                                               
       float filterRelease      = 0.0f;                                                               
       float filterEnvDepth     = 0.0f;                                                               
   };

   struct UpdateContext
   {
        float sampleRate;
        float inverseSampleRate;
        float inverseUpdateRate;
   };

   using ApplyFn = void (*) (SynthParams&, float value, const UpdateContext&);

   namespace SynthLimits { constexpr int MAX_VOICES = 8; }