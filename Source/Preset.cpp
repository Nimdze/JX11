#include "Preset.h"
#include "Parameters.h"

Preset::Preset (const char* presetName, std::initializer_list<PresetPatch> overrides)
    : name (presetName)
{
    for (int i = 0; i < Params::NumParams; ++i)
        param[i] = Params::kSpecs[i].defaultValue;

    for (const auto& o : overrides)
        param[static_cast<int> (o.index)] = o.value;
}

namespace
{
Preset makePreset (const char* name, std::initializer_list<PresetPatch> overrides)
{
    return Preset (name, overrides);
}
} // namespace

std::vector<Preset> createFactoryPresets()
{
    std::vector<Preset> presets;
    presets.reserve (53);

    presets.push_back (makePreset (
        "Init", {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},      {Params::oscFine, 0.00f},
                 {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                 {Params::filterFreq, 100.00f}, {Params::filterReso, 15.00f},    {Params::filterEnv, 50.00f},
                 {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                 {Params::filterDecay, 30.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                 {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                 {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                 {Params::noise, 0.00f},        {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                 {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("5th Sweep Pad",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -7.00f},       {Params::oscFine, -6.30f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 32.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 90.00f},  {Params::filterReso, 60.00f},    {Params::filterEnv, -76.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 90.00f},
                     {Params::filterDecay, 89.00f}, {Params::filterSustain, 90.00f}, {Params::filterRelease, 73.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 71.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 30.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Echo Pad [SA]",
                    {{Params::oscMix, 88.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 49.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 46.00f},  {Params::filterReso, 76.00f},     {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 10.00f},   {Params::filterVelocity, 38.00f}, {Params::filterAttack, 100.00f},
                     {Params::filterDecay, 86.00f}, {Params::filterSustain, 76.00f},  {Params::filterRelease, 57.00f},
                     {Params::envAttack, 30.00f},   {Params::envDecay, 80.00f},       {Params::envSustain, 68.00f},
                     {Params::envRelease, 66.00f},  {Params::lfoRate, 0.79f},         {Params::vibrato, -74.00f},
                     {Params::noise, 25.00f},       {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Space Chimes [SA]",
                    {{Params::oscMix, 88.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 49.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 49.00f},  {Params::filterReso, 82.00f},     {Params::filterEnv, 32.00f},
                     {Params::filterLFO, 8.00f},    {Params::filterVelocity, 78.00f}, {Params::filterAttack, 85.00f},
                     {Params::filterDecay, 69.00f}, {Params::filterSustain, 76.00f},  {Params::filterRelease, 47.00f},
                     {Params::envAttack, 12.00f},   {Params::envDecay, 22.00f},       {Params::envSustain, 55.00f},
                     {Params::envRelease, 66.00f},  {Params::lfoRate, 0.89f},         {Params::vibrato, -32.00f},
                     {Params::noise, 0.00f},        {Params::octave, 2.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Solid Backing",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -12.00f},       {Params::oscFine, -18.70f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 30.00f},  {Params::filterReso, 25.00f},     {Params::filterEnv, 40.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 26.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 35.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 25.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},         {Params::vibrato, 0.00f},
                     {Params::noise, 50.00f},       {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Velocity Backing [SA]",
                    {{Params::oscMix, 41.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, 9.70f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 8.00f},       {Params::glideBend, -1.68f},
                     {Params::filterFreq, 49.00f},  {Params::filterReso, 1.00f},      {Params::filterEnv, -32.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 86.00f}, {Params::filterAttack, 61.00f},
                     {Params::filterDecay, 87.00f}, {Params::filterSustain, 100.00f}, {Params::filterRelease, 93.00f},
                     {Params::envAttack, 11.00f},   {Params::envDecay, 48.00f},       {Params::envSustain, 98.00f},
                     {Params::envRelease, 32.00f},  {Params::lfoRate, 0.81f},         {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Rubber Backing [ZF]",
                    {{Params::oscMix, 29.00f},      {Params::oscTune, 12.00f},       {Params::oscFine, -5.60f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 18.00f},     {Params::glideBend, 5.06f},
                     {Params::filterFreq, 35.00f},  {Params::filterReso, 15.00f},    {Params::filterEnv, 54.00f},
                     {Params::filterLFO, 14.00f},   {Params::filterVelocity, 8.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 42.00f}, {Params::filterSustain, 13.00f}, {Params::filterRelease, 21.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 56.00f},      {Params::envSustain, 0.00f},
                     {Params::envRelease, 32.00f},  {Params::lfoRate, 0.20f},        {Params::vibrato, 16.00f},
                     {Params::noise, 22.00f},       {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("808 State Lead",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, 7.00f},        {Params::oscFine, -7.10f},
                     {Params::glideMode, 2.00f},    {Params::glideRate, 34.00f},     {Params::glideBend, 12.35f},
                     {Params::filterFreq, 65.00f},  {Params::filterReso, 63.00f},    {Params::filterEnv, 50.00f},
                     {Params::filterLFO, 16.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 30.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 17.00f},   {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 3.00f},   {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, 1.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset ("Mono Glide", {{Params::oscMix, 0.00f},
                                                  {Params::oscTune, -12.00f},
                                                  {Params::oscFine, 0.00f},
                                                  {Params::glideMode, 2.00f},
                                                  {Params::glideRate, 46.00f},
                                                  {Params::glideBend, 0.00f},
                                                  {Params::filterFreq, 51.00f},
                                                  {Params::filterReso, 0.00f},
                                                  {Params::filterEnv, 0.00f},
                                                  {Params::filterLFO, 0.00f},
                                                  {Params::filterVelocity, -100.00f},
                                                  {Params::filterAttack, 0.00f},
                                                  {Params::filterDecay, 30.00f},
                                                  {Params::filterSustain, 0.00f},
                                                  {Params::filterRelease, 25.00f},
                                                  {Params::envAttack, 37.00f},
                                                  {Params::envDecay, 50.00f},
                                                  {Params::envSustain, 100.00f},
                                                  {Params::envRelease, 38.00f},
                                                  {Params::lfoRate, 0.81f},
                                                  {Params::vibrato, 24.00f},
                                                  {Params::noise, 0.00f},
                                                  {Params::octave, 0.00f},
                                                  {Params::tuning, 0.00f},
                                                  {Params::outputLevel, 0.00f},
                                                  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Detuned Techno Lead",
                    {{Params::oscMix, 84.00f},     {Params::oscTune, 0.00f},         {Params::oscFine, -17.20f},
                     {Params::glideMode, 2.00f},   {Params::glideRate, 41.00f},      {Params::glideBend, -0.15f},
                     {Params::filterFreq, 54.00f}, {Params::filterReso, 1.00f},      {Params::filterEnv, 16.00f},
                     {Params::filterLFO, 21.00f},  {Params::filterVelocity, 34.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 9.00f}, {Params::filterSustain, 100.00f}, {Params::filterRelease, 25.00f},
                     {Params::envAttack, 20.00f},  {Params::envDecay, 85.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 30.00f}, {Params::lfoRate, 0.83f},         {Params::vibrato, -82.00f},
                     {Params::noise, 40.00f},      {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f}, {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Hard Lead [SA]",
                    {{Params::oscMix, 71.00f},      {Params::oscTune, 12.00f},        {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 24.00f},      {Params::glideBend, 36.00f},
                     {Params::filterFreq, 56.00f},  {Params::filterReso, 52.00f},     {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 19.00f},   {Params::filterVelocity, 40.00f}, {Params::filterAttack, 100.00f},
                     {Params::filterDecay, 14.00f}, {Params::filterSustain, 65.00f},  {Params::filterRelease, 95.00f},
                     {Params::envAttack, 7.00f},    {Params::envDecay, 91.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 15.00f},  {Params::lfoRate, 0.84f},         {Params::vibrato, -34.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Bubble", {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},       {Params::oscFine, -0.20f},
                   {Params::glideMode, 0.00f},    {Params::glideRate, 71.00f},      {Params::glideBend, -0.00f},
                   {Params::filterFreq, 23.00f},  {Params::filterReso, 77.00f},     {Params::filterEnv, 60.00f},
                   {Params::filterLFO, 32.00f},   {Params::filterVelocity, 26.00f}, {Params::filterAttack, 40.00f},
                   {Params::filterDecay, 18.00f}, {Params::filterSustain, 66.00f},  {Params::filterRelease, 14.00f},
                   {Params::envAttack, 0.00f},    {Params::envDecay, 38.00f},       {Params::envSustain, 65.00f},
                   {Params::envRelease, 16.00f},  {Params::lfoRate, 0.48f},         {Params::vibrato, 0.00f},
                   {Params::noise, 0.00f},        {Params::octave, 1.00f},          {Params::tuning, 0.00f},
                   {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Monosynth",
                    {{Params::oscMix, 62.00f},      {Params::oscTune, -12.00f},         {Params::oscFine, 0.00f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 35.00f},        {Params::glideBend, 0.02f},
                     {Params::filterFreq, 64.00f},  {Params::filterReso, 39.00f},       {Params::filterEnv, 2.00f},
                     {Params::filterLFO, 65.00f},   {Params::filterVelocity, -100.00f}, {Params::filterAttack, 7.00f},
                     {Params::filterDecay, 52.00f}, {Params::filterSustain, 24.00f},    {Params::filterRelease, 84.00f},
                     {Params::envAttack, 13.00f},   {Params::envDecay, 30.00f},         {Params::envSustain, 76.00f},
                     {Params::envRelease, 21.00f},  {Params::lfoRate, 0.58f},           {Params::vibrato, -40.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Moogcury Lite",
                    {{Params::oscMix, 81.00f},      {Params::oscTune, 24.00f},        {Params::oscFine, -9.80f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 15.00f},      {Params::glideBend, -0.97f},
                     {Params::filterFreq, 39.00f},  {Params::filterReso, 17.00f},     {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 40.00f},   {Params::filterVelocity, 24.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 47.00f}, {Params::filterSustain, 19.00f},  {Params::filterRelease, 37.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},       {Params::envSustain, 20.00f},
                     {Params::envRelease, 33.00f},  {Params::lfoRate, 0.38f},         {Params::vibrato, 6.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (makePreset ("Gangsta Whine", {{Params::oscMix, 0.00f},
                                                     {Params::oscTune, 0.00f},
                                                     {Params::oscFine, 0.00f},
                                                     {Params::glideMode, 2.00f},
                                                     {Params::glideRate, 44.00f},
                                                     {Params::glideBend, 0.00f},
                                                     {Params::filterFreq, 41.00f},
                                                     {Params::filterReso, 46.00f},
                                                     {Params::filterEnv, 0.00f},
                                                     {Params::filterLFO, 0.00f},
                                                     {Params::filterVelocity, -100.00f},
                                                     {Params::filterAttack, 0.00f},
                                                     {Params::filterDecay, 0.00f},
                                                     {Params::filterSustain, 100.00f},
                                                     {Params::filterRelease, 25.00f},
                                                     {Params::envAttack, 15.00f},
                                                     {Params::envDecay, 50.00f},
                                                     {Params::envSustain, 100.00f},
                                                     {Params::envRelease, 32.00f},
                                                     {Params::lfoRate, 0.81f},
                                                     {Params::vibrato, -2.00f},
                                                     {Params::noise, 0.00f},
                                                     {Params::octave, 2.00f},
                                                     {Params::tuning, 0.00f},
                                                     {Params::outputLevel, 0.00f},
                                                     {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Higher Synth [ZF]",
                    {{Params::oscMix, 48.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, -8.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 0.00f},       {Params::glideBend, 0.00f},
                     {Params::filterFreq, 50.00f},  {Params::filterReso, 47.00f},     {Params::filterEnv, 46.00f},
                     {Params::filterLFO, 30.00f},   {Params::filterVelocity, 60.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 10.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 7.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 42.00f},       {Params::envSustain, 0.00f},
                     {Params::envRelease, 22.00f},  {Params::lfoRate, 0.21f},         {Params::vibrato, 18.00f},
                     {Params::noise, 16.00f},       {Params::octave, 2.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("303 Saw Bass",
                    {{Params::oscMix, 0.00f},       {Params::oscTune, 0.00f},        {Params::oscFine, 0.00f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 49.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 55.00f},  {Params::filterReso, 75.00f},    {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 35.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 56.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 56.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 80.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 24.00f},  {Params::lfoRate, 0.26f},        {Params::vibrato, -2.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("303 Square Bass",
                    {{Params::oscMix, 75.00f},      {Params::oscTune, 0.00f},        {Params::oscFine, 0.00f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 49.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 55.00f},  {Params::filterReso, 75.00f},    {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 35.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 14.00f},
                     {Params::filterDecay, 49.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 39.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 80.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 24.00f},  {Params::lfoRate, 0.26f},        {Params::vibrato, -2.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Analog Bass",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -12.00f},         {Params::oscFine, -10.90f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 19.00f},        {Params::glideBend, 0.00f},
                     {Params::filterFreq, 30.00f},  {Params::filterReso, 51.00f},       {Params::filterEnv, 70.00f},
                     {Params::filterLFO, 9.00f},    {Params::filterVelocity, -100.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 88.00f}, {Params::filterSustain, 0.00f},     {Params::filterRelease, 21.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},         {Params::envSustain, 100.00f},
                     {Params::envRelease, 46.00f},  {Params::lfoRate, 0.81f},           {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Analog Bass 2",
                    {{Params::oscMix, 100.00f},    {Params::oscTune, -12.00f},       {Params::oscFine, -10.90f},
                     {Params::glideMode, 0.00f},   {Params::glideRate, 19.00f},      {Params::glideBend, 13.44f},
                     {Params::filterFreq, 48.00f}, {Params::filterReso, 43.00f},     {Params::filterEnv, 88.00f},
                     {Params::filterLFO, 0.00f},   {Params::filterVelocity, 60.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 0.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 0.00f},
                     {Params::envAttack, 0.00f},   {Params::envDecay, 61.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 32.00f}, {Params::lfoRate, 0.81f},         {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},       {Params::octave, -1.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f}, {Params::polyMode, 0.00f}}));
    presets.push_back (makePreset (
        "Low Pulses", {{Params::oscMix, 97.00f},      {Params::oscTune, -12.00f},      {Params::oscFine, -3.30f},
                       {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                       {Params::filterFreq, 80.00f},  {Params::filterReso, 40.00f},    {Params::filterEnv, 4.00f},
                       {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                       {Params::filterDecay, 77.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                       {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                       {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, -68.00f},
                       {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                       {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Sine Infra-Bass",
                    {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},      {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 33.00f},  {Params::filterReso, 76.00f},    {Params::filterEnv, 6.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 30.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 55.00f},      {Params::envSustain, 25.00f},
                     {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 4.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Wobble Bass [SA]",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -12.00f},       {Params::oscFine, -8.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 82.00f},      {Params::glideBend, 0.21f},
                     {Params::filterFreq, 72.00f},  {Params::filterReso, 47.00f},     {Params::filterEnv, -32.00f},
                     {Params::filterLFO, 34.00f},   {Params::filterVelocity, 64.00f}, {Params::filterAttack, 20.00f},
                     {Params::filterDecay, 69.00f}, {Params::filterSustain, 100.00f}, {Params::filterRelease, 15.00f},
                     {Params::envAttack, 9.00f},    {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 7.00f},   {Params::lfoRate, 0.81f},         {Params::vibrato, -8.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Squelch Bass",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -12.00f},       {Params::oscFine, -8.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 67.00f},  {Params::filterReso, 70.00f},     {Params::filterEnv, -48.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f},  {Params::filterAttack, 48.00f},
                     {Params::filterDecay, 69.00f}, {Params::filterSustain, 100.00f}, {Params::filterRelease, 15.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 7.00f},   {Params::lfoRate, 0.81f},         {Params::vibrato, -8.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Rubber Bass [ZF]",
                    {{Params::oscMix, 49.00f},      {Params::oscTune, -12.00f},      {Params::oscFine, 1.60f},
                     {Params::glideMode, 1.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 36.00f},  {Params::filterReso, 15.00f},    {Params::filterEnv, 50.00f},
                     {Params::filterLFO, 20.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 38.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 60.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 22.00f},  {Params::lfoRate, 0.19f},        {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Soft Pick Bass",
                    {{Params::oscMix, 37.00f},     {Params::oscTune, 0.00f},         {Params::oscFine, 7.80f},
                     {Params::glideMode, 0.00f},   {Params::glideRate, 22.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 33.00f}, {Params::filterReso, 47.00f},     {Params::filterEnv, 42.00f},
                     {Params::filterLFO, 16.00f},  {Params::filterVelocity, 18.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 0.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 25.00f},
                     {Params::envAttack, 4.00f},   {Params::envDecay, 58.00f},       {Params::envSustain, 0.00f},
                     {Params::envRelease, 22.00f}, {Params::lfoRate, 0.15f},         {Params::vibrato, -12.00f},
                     {Params::noise, 33.00f},      {Params::octave, -2.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f}, {Params::polyMode, 0.00f}}));
    presets.push_back (
        makePreset ("Fretless Bass",
                    {{Params::oscMix, 50.00f},     {Params::oscTune, 0.00f},         {Params::oscFine, -14.40f},
                     {Params::glideMode, 1.00f},   {Params::glideRate, 34.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 51.00f}, {Params::filterReso, 0.00f},      {Params::filterEnv, 16.00f},
                     {Params::filterLFO, 0.00f},   {Params::filterVelocity, 34.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 9.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 25.00f},
                     {Params::envAttack, 20.00f},  {Params::envDecay, 85.00f},       {Params::envSustain, 0.00f},
                     {Params::envRelease, 30.00f}, {Params::lfoRate, 0.81f},         {Params::vibrato, 40.00f},
                     {Params::noise, 0.00f},       {Params::octave, -2.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f}, {Params::polyMode, 0.00f}}));
    presets.push_back (makePreset (
        "Whistler", {{Params::oscMix, 23.00f},      {Params::oscTune, 0.00f},        {Params::oscFine, -0.70f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 33.00f},  {Params::filterReso, 100.00f},   {Params::filterEnv, 0.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 29.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 68.00f},   {Params::envDecay, 39.00f},      {Params::envSustain, 58.00f},
                     {Params::envRelease, 36.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 28.00f},
                     {Params::noise, 38.00f},       {Params::octave, 2.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Very Soft Pad",
                    {{Params::oscMix, 39.00f},      {Params::oscTune, 0.00f},        {Params::oscFine, -4.90f},
                     {Params::glideMode, 2.00f},    {Params::glideRate, 12.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 35.00f},  {Params::filterReso, 78.00f},    {Params::filterEnv, 0.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 30.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 35.00f},   {Params::envDecay, 50.00f},      {Params::envSustain, 80.00f},
                     {Params::envRelease, 70.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Pizzicato", {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},      {Params::oscFine, 0.00f},
                      {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                      {Params::filterFreq, 23.00f},  {Params::filterReso, 20.00f},    {Params::filterEnv, 50.00f},
                      {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                      {Params::filterDecay, 22.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                      {Params::envAttack, 0.00f},    {Params::envDecay, 47.00f},      {Params::envSustain, 0.00f},
                      {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                      {Params::noise, 80.00f},       {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                      {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Synth Strings",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, 0.00f},         {Params::oscFine, -7.10f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 0.00f},       {Params::glideBend, -0.97f},
                     {Params::filterFreq, 42.00f},  {Params::filterReso, 26.00f},     {Params::filterEnv, 50.00f},
                     {Params::filterLFO, 14.00f},   {Params::filterVelocity, 38.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 67.00f}, {Params::filterSustain, 55.00f},  {Params::filterRelease, 97.00f},
                     {Params::envAttack, 82.00f},   {Params::envDecay, 70.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 42.00f},  {Params::lfoRate, 0.84f},         {Params::vibrato, 34.00f},
                     {Params::noise, 30.00f},       {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Synth Strings 2",
                    {{Params::oscMix, 75.00f},      {Params::oscTune, 0.00f},          {Params::oscFine, -3.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 49.00f},       {Params::glideBend, 0.00f},
                     {Params::filterFreq, 55.00f},  {Params::filterReso, 16.00f},      {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 8.00f},    {Params::filterVelocity, -60.00f}, {Params::filterAttack, 76.00f},
                     {Params::filterDecay, 29.00f}, {Params::filterSustain, 76.00f},   {Params::filterRelease, 100.00f},
                     {Params::envAttack, 46.00f},   {Params::envDecay, 80.00f},        {Params::envSustain, 100.00f},
                     {Params::envRelease, 39.00f},  {Params::lfoRate, 0.79f},          {Params::vibrato, -46.00f},
                     {Params::noise, 0.00f},        {Params::octave, 1.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Leslie Organ",
                    {{Params::oscMix, 0.00f},       {Params::oscTune, 0.00f},           {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 13.00f},        {Params::glideBend, -0.38f},
                     {Params::filterFreq, 38.00f},  {Params::filterReso, 74.00f},       {Params::filterEnv, 8.00f},
                     {Params::filterLFO, 20.00f},   {Params::filterVelocity, -100.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 55.00f}, {Params::filterSustain, 52.00f},    {Params::filterRelease, 31.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 17.00f},         {Params::envSustain, 73.00f},
                     {Params::envRelease, 28.00f},  {Params::lfoRate, 0.87f},           {Params::vibrato, -52.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset ("Click Organ", {{Params::oscMix, 50.00f},
                                                   {Params::oscTune, 12.00f},
                                                   {Params::oscFine, 0.00f},
                                                   {Params::glideMode, 0.00f},
                                                   {Params::glideRate, 35.00f},
                                                   {Params::glideBend, 0.00f},
                                                   {Params::filterFreq, 44.00f},
                                                   {Params::filterReso, 50.00f},
                                                   {Params::filterEnv, 30.00f},
                                                   {Params::filterLFO, 16.00f},
                                                   {Params::filterVelocity, -100.00f},
                                                   {Params::filterAttack, 0.00f},
                                                   {Params::filterDecay, 0.00f},
                                                   {Params::filterSustain, 18.00f},
                                                   {Params::filterRelease, 0.00f},
                                                   {Params::envAttack, 0.00f},
                                                   {Params::envDecay, 75.00f},
                                                   {Params::envSustain, 80.00f},
                                                   {Params::envRelease, 0.00f},
                                                   {Params::lfoRate, 0.81f},
                                                   {Params::vibrato, -2.00f},
                                                   {Params::noise, 0.00f},
                                                   {Params::octave, 0.00f},
                                                   {Params::tuning, 0.00f},
                                                   {Params::outputLevel, 0.00f},
                                                   {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset ("Hard Organ", {{Params::oscMix, 89.00f},
                                                  {Params::oscTune, 19.00f},
                                                  {Params::oscFine, -0.90f},
                                                  {Params::glideMode, 0.00f},
                                                  {Params::glideRate, 35.00f},
                                                  {Params::glideBend, 0.00f},
                                                  {Params::filterFreq, 51.00f},
                                                  {Params::filterReso, 62.00f},
                                                  {Params::filterEnv, 8.00f},
                                                  {Params::filterLFO, 0.00f},
                                                  {Params::filterVelocity, -100.00f},
                                                  {Params::filterAttack, 0.00f},
                                                  {Params::filterDecay, 37.00f},
                                                  {Params::filterSustain, 0.00f},
                                                  {Params::filterRelease, 100.00f},
                                                  {Params::envAttack, 4.00f},
                                                  {Params::envDecay, 8.00f},
                                                  {Params::envSustain, 72.00f},
                                                  {Params::envRelease, 4.00f},
                                                  {Params::lfoRate, 0.77f},
                                                  {Params::vibrato, -2.00f},
                                                  {Params::noise, 0.00f},
                                                  {Params::octave, 0.00f},
                                                  {Params::tuning, 0.00f},
                                                  {Params::outputLevel, 0.00f},
                                                  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Bass Clarinet",
                    {{Params::oscMix, 100.00f},    {Params::oscTune, 0.00f},        {Params::oscFine, 0.00f},
                     {Params::glideMode, 1.00f},   {Params::glideRate, 0.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 51.00f}, {Params::filterReso, 10.00f},    {Params::filterEnv, 0.00f},
                     {Params::filterLFO, 11.00f},  {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 0.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 35.00f},  {Params::envDecay, 65.00f},      {Params::envSustain, 65.00f},
                     {Params::envRelease, 32.00f}, {Params::lfoRate, 0.79f},        {Params::vibrato, -2.00f},
                     {Params::noise, 20.00f},      {Params::octave, -1.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f}, {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Trumpet", {{Params::oscMix, 0.00f},       {Params::oscTune, 0.00f},        {Params::oscFine, 0.00f},
                    {Params::glideMode, 1.00f},    {Params::glideRate, 6.00f},      {Params::glideBend, 0.00f},
                    {Params::filterFreq, 57.00f},  {Params::filterReso, 0.00f},     {Params::filterEnv, -36.00f},
                    {Params::filterLFO, 15.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 21.00f},
                    {Params::filterDecay, 15.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                    {Params::envAttack, 24.00f},   {Params::envDecay, 60.00f},      {Params::envSustain, 80.00f},
                    {Params::envRelease, 10.00f},  {Params::lfoRate, 0.75f},        {Params::vibrato, 10.00f},
                    {Params::noise, 25.00f},       {Params::octave, 1.00f},         {Params::tuning, 0.00f},
                    {Params::outputLevel, 0.00f},  {Params::polyMode, 0.00f}}));
    presets.push_back (makePreset (
        "Soft Horn", {{Params::oscMix, 12.00f},      {Params::oscTune, 19.00f},        {Params::oscFine, 1.90f},
                      {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},      {Params::glideBend, 0.00f},
                      {Params::filterFreq, 50.00f},  {Params::filterReso, 21.00f},     {Params::filterEnv, -42.00f},
                      {Params::filterLFO, 12.00f},   {Params::filterVelocity, 20.00f}, {Params::filterAttack, 0.00f},
                      {Params::filterDecay, 35.00f}, {Params::filterSustain, 36.00f},  {Params::filterRelease, 25.00f},
                      {Params::envAttack, 8.00f},    {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                      {Params::envRelease, 27.00f},  {Params::lfoRate, 0.83f},         {Params::vibrato, 2.00f},
                      {Params::noise, 10.00f},       {Params::octave, -1.00f},         {Params::tuning, 0.00f},
                      {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Brass Section",
                    {{Params::oscMix, 43.00f},      {Params::oscTune, 12.00f},       {Params::oscFine, -7.90f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 28.00f},     {Params::glideBend, -0.79f},
                     {Params::filterFreq, 50.00f},  {Params::filterReso, 0.00f},     {Params::filterEnv, 18.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 24.00f},
                     {Params::filterDecay, 16.00f}, {Params::filterSustain, 91.00f}, {Params::filterRelease, 8.00f},
                     {Params::envAttack, 17.00f},   {Params::envDecay, 50.00f},      {Params::envSustain, 80.00f},
                     {Params::envRelease, 45.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Synth Brass", {{Params::oscMix, 40.00f},      {Params::oscTune, 0.00f},        {Params::oscFine, -6.30f},
                        {Params::glideMode, 0.00f},    {Params::glideRate, 30.00f},     {Params::glideBend, -3.07f},
                        {Params::filterFreq, 39.00f},  {Params::filterReso, 15.00f},    {Params::filterEnv, 50.00f},
                        {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 39.00f},
                        {Params::filterDecay, 30.00f}, {Params::filterSustain, 82.00f}, {Params::filterRelease, 25.00f},
                        {Params::envAttack, 33.00f},   {Params::envDecay, 74.00f},      {Params::envSustain, 76.00f},
                        {Params::envRelease, 41.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, -6.00f},
                        {Params::noise, 23.00f},       {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                        {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Detuned Syn Brass [ZF]",
                    {{Params::oscMix, 68.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, 31.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 31.00f},      {Params::glideBend, 0.50f},
                     {Params::filterFreq, 26.00f},  {Params::filterReso, 7.00f},      {Params::filterEnv, 70.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 32.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 83.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 5.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 75.00f},       {Params::envSustain, 54.00f},
                     {Params::envRelease, 32.00f},  {Params::lfoRate, 0.76f},         {Params::vibrato, -26.00f},
                     {Params::noise, 29.00f},       {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Power PWM",
                    {{Params::oscMix, 100.00f},     {Params::oscTune, -12.00f},         {Params::oscFine, -8.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},        {Params::glideBend, 0.00f},
                     {Params::filterFreq, 82.00f},  {Params::filterReso, 13.00f},       {Params::filterEnv, 50.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, -100.00f}, {Params::filterAttack, 24.00f},
                     {Params::filterDecay, 30.00f}, {Params::filterSustain, 88.00f},    {Params::filterRelease, 34.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 50.00f},         {Params::envSustain, 100.00f},
                     {Params::envRelease, 48.00f},  {Params::lfoRate, 0.71f},           {Params::vibrato, -26.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Water Velocity [SA]",
                    {{Params::oscMix, 76.00f},      {Params::oscTune, 0.00f},          {Params::oscFine, -1.40f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 49.00f},       {Params::glideBend, 0.00f},
                     {Params::filterFreq, 87.00f},  {Params::filterReso, 67.00f},      {Params::filterEnv, 100.00f},
                     {Params::filterLFO, 32.00f},   {Params::filterVelocity, -82.00f}, {Params::filterAttack, 95.00f},
                     {Params::filterDecay, 56.00f}, {Params::filterSustain, 72.00f},   {Params::filterRelease, 100.00f},
                     {Params::envAttack, 4.00f},    {Params::envDecay, 76.00f},        {Params::envSustain, 11.00f},
                     {Params::envRelease, 46.00f},  {Params::lfoRate, 0.88f},          {Params::vibrato, 44.00f},
                     {Params::noise, 0.00f},        {Params::octave, -1.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Ghost [SA]", {{Params::oscMix, 75.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, -7.10f},
                       {Params::glideMode, 2.00f},    {Params::glideRate, 16.00f},      {Params::glideBend, -0.00f},
                       {Params::filterFreq, 38.00f},  {Params::filterReso, 58.00f},     {Params::filterEnv, 50.00f},
                       {Params::filterLFO, 16.00f},   {Params::filterVelocity, 62.00f}, {Params::filterAttack, 0.00f},
                       {Params::filterDecay, 30.00f}, {Params::filterSustain, 40.00f},  {Params::filterRelease, 31.00f},
                       {Params::envAttack, 37.00f},   {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                       {Params::envRelease, 54.00f},  {Params::lfoRate, 0.85f},         {Params::vibrato, 66.00f},
                       {Params::noise, 43.00f},       {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                       {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Soft E.Piano",
                    {{Params::oscMix, 31.00f},      {Params::oscTune, 0.00f},         {Params::oscFine, -0.20f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 34.00f},  {Params::filterReso, 26.00f},     {Params::filterEnv, 6.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 26.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 22.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 39.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 80.00f},       {Params::envSustain, 0.00f},
                     {Params::envRelease, 44.00f},  {Params::lfoRate, 0.81f},         {Params::vibrato, 2.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Thumb Piano", {{Params::oscMix, 72.00f},      {Params::oscTune, 15.00f},       {Params::oscFine, 50.00f},
                        {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                        {Params::filterFreq, 37.00f},  {Params::filterReso, 47.00f},    {Params::filterEnv, 8.00f},
                        {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 0.00f},
                        {Params::filterDecay, 45.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 39.00f},
                        {Params::envAttack, 0.00f},    {Params::envDecay, 39.00f},      {Params::envSustain, 0.00f},
                        {Params::envRelease, 48.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 20.00f},
                        {Params::noise, 0.00f},        {Params::octave, 1.00f},         {Params::tuning, 0.00f},
                        {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Steel Drums [ZF]",
                    {{Params::oscMix, 81.00f},      {Params::oscTune, 12.00f},         {Params::oscFine, -12.00f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 18.00f},       {Params::glideBend, 2.30f},
                     {Params::filterFreq, 40.00f},  {Params::filterReso, 30.00f},      {Params::filterEnv, 8.00f},
                     {Params::filterLFO, 17.00f},   {Params::filterVelocity, -20.00f}, {Params::filterAttack, 0.00f},
                     {Params::filterDecay, 42.00f}, {Params::filterSustain, 23.00f},   {Params::filterRelease, 47.00f},
                     {Params::envAttack, 12.00f},   {Params::envDecay, 48.00f},        {Params::envSustain, 0.00f},
                     {Params::envRelease, 49.00f},  {Params::lfoRate, 0.53f},          {Params::vibrato, -28.00f},
                     {Params::noise, 34.00f},       {Params::octave, 0.00f},           {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Car Horn", {{Params::oscMix, 57.00f},      {Params::oscTune, -1.00f},        {Params::oscFine, -2.80f},
                     {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},      {Params::glideBend, 0.00f},
                     {Params::filterFreq, 46.00f},  {Params::filterReso, 0.00f},      {Params::filterEnv, 36.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f},  {Params::filterAttack, 46.00f},
                     {Params::filterDecay, 30.00f}, {Params::filterSustain, 100.00f}, {Params::filterRelease, 23.00f},
                     {Params::envAttack, 30.00f},   {Params::envDecay, 50.00f},       {Params::envSustain, 100.00f},
                     {Params::envRelease, 31.00f},  {Params::lfoRate, 1.00f},         {Params::vibrato, -24.00f},
                     {Params::noise, 0.00f},        {Params::octave, 0.00f},          {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Helicopter",
                    {{Params::oscMix, 0.00f},        {Params::oscTune, -12.00f},      {Params::oscFine, 0.00f},
                     {Params::glideMode, 0.00f},     {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                     {Params::filterFreq, 8.00f},    {Params::filterReso, 36.00f},    {Params::filterEnv, 38.00f},
                     {Params::filterLFO, 100.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 100.00f},
                     {Params::filterDecay, 100.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 100.00f},
                     {Params::envAttack, 96.00f},    {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                     {Params::envRelease, 92.00f},   {Params::lfoRate, 0.97f},        {Params::vibrato, 0.00f},
                     {Params::noise, 100.00f},       {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},   {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Arctic Wind", {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},      {Params::oscFine, 0.00f},
                        {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                        {Params::filterFreq, 16.00f},  {Params::filterReso, 85.00f},    {Params::filterEnv, 0.00f},
                        {Params::filterLFO, 28.00f},   {Params::filterVelocity, 0.00f}, {Params::filterAttack, 37.00f},
                        {Params::filterDecay, 30.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                        {Params::envAttack, 89.00f},   {Params::envDecay, 50.00f},      {Params::envSustain, 100.00f},
                        {Params::envRelease, 89.00f},  {Params::lfoRate, 0.24f},        {Params::vibrato, 0.00f},
                        {Params::noise, 100.00f},      {Params::octave, 2.00f},         {Params::tuning, 0.00f},
                        {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Thip", {{Params::oscMix, 100.00f},     {Params::oscTune, -7.00f},       {Params::oscFine, 0.00f},
                 {Params::glideMode, 0.00f},    {Params::glideRate, 35.00f},     {Params::glideBend, 0.00f},
                 {Params::filterFreq, 0.00f},   {Params::filterReso, 100.00f},   {Params::filterEnv, 94.00f},
                 {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 2.00f},
                 {Params::filterDecay, 20.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 20.00f},
                 {Params::envAttack, 0.00f},    {Params::envDecay, 46.00f},      {Params::envSustain, 0.00f},
                 {Params::envRelease, 30.00f},  {Params::lfoRate, 0.81f},        {Params::vibrato, 0.00f},
                 {Params::noise, 78.00f},       {Params::octave, 0.00f},         {Params::tuning, 0.00f},
                 {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (makePreset (
        "Synth Tom", {{Params::oscMix, 0.00f},       {Params::oscTune, -12.00f},       {Params::oscFine, 0.00f},
                      {Params::glideMode, 0.00f},    {Params::glideRate, 76.00f},      {Params::glideBend, 24.53f},
                      {Params::filterFreq, 30.00f},  {Params::filterReso, 33.00f},     {Params::filterEnv, 52.00f},
                      {Params::filterLFO, 0.00f},    {Params::filterVelocity, 36.00f}, {Params::filterAttack, 0.00f},
                      {Params::filterDecay, 59.00f}, {Params::filterSustain, 0.00f},   {Params::filterRelease, 59.00f},
                      {Params::envAttack, 10.00f},   {Params::envDecay, 50.00f},       {Params::envSustain, 0.00f},
                      {Params::envRelease, 50.00f},  {Params::lfoRate, 0.81f},         {Params::vibrato, 0.00f},
                      {Params::noise, 70.00f},       {Params::octave, -2.00f},         {Params::tuning, 0.00f},
                      {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));
    presets.push_back (
        makePreset ("Squelchy Frog",
                    {{Params::oscMix, 50.00f},      {Params::oscTune, -5.00f},       {Params::oscFine, -7.90f},
                     {Params::glideMode, 2.00f},    {Params::glideRate, 77.00f},     {Params::glideBend, -36.00f},
                     {Params::filterFreq, 40.00f},  {Params::filterReso, 65.00f},    {Params::filterEnv, 90.00f},
                     {Params::filterLFO, 0.00f},    {Params::filterVelocity, 0.00f}, {Params::filterAttack, 33.00f},
                     {Params::filterDecay, 50.00f}, {Params::filterSustain, 0.00f},  {Params::filterRelease, 25.00f},
                     {Params::envAttack, 0.00f},    {Params::envDecay, 70.00f},      {Params::envSustain, 65.00f},
                     {Params::envRelease, 18.00f},  {Params::lfoRate, 0.32f},        {Params::vibrato, 100.00f},
                     {Params::noise, 0.00f},        {Params::octave, -2.00f},        {Params::tuning, 0.00f},
                     {Params::outputLevel, 0.00f},  {Params::polyMode, 1.00f}}));

    return presets;
}
