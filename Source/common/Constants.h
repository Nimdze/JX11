#pragma once

inline constexpr float PI = 3.1415926535897932f;

// Tuning: one equal-tempered semitone as a frequency ratio and its natural log,
// so note-to-period math can use either pow() or exp().
inline constexpr float kSemitoneRatio = 1.0594630943592953f; // 2^(1/12)
inline constexpr float kSemitoneLog = 0.05776226505f;        // ln(2) / 12

// Oscillator / voice
inline constexpr float kMinPeriodSamples = 6.0f; // highest playable pitch
inline constexpr float kSawLeak = 0.997f;        // leaky-integrator pole

// Filter modulation smoothing per LFO update step.
inline constexpr float kFilterModSmoothing = 0.005f;

// Ramp time for continuous parameter smoothing (seconds).
inline constexpr double kParamSmoothingSeconds = 0.02;

// Pitch-bend exponent per MIDI bend unit (2 semitones over the 14-bit range).
inline constexpr float kPitchBendRate = 0.000014102f;

// Filter cutoff limits. The upper bound is deliberately below Nyquist so the
// bilinear-transform tan() stays positive on any sample rate.
inline constexpr float kMinFilterCutoff = 20.0f;
inline constexpr float kMaxCutoffRatio = 0.45f;
