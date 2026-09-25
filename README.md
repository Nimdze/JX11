## JX11 SYNTH based on JX10

A subtractive synth built with JUCE
Formats - Standalone, AU, VST3.

## Prerequisites
- macOS
-CMake 3.22 or above
- A C++17 compiler: Xcode Command Line Tools('xcode-select --install') or Xcode
--JUCE (see below)

## One-time setup
This project expects JUCE to be available at `./juce`. It's not committed to the repo, so create the symlink to your JUCE checkout:

    git clone https://github.com/juce-framework/JUCE.git ~ JUCE                                      
    ln -s ~/JUCE juce  

  Verify: `ls -l juce` should show `juce -> /Users/<you>/JUCE`. 

## Configure   
Use a dedicated build directory. The generator is locked in on first configure,  so don't reuse a directory configured with a different generator.    

    cmake -B build-vscode -G "Unix Makefiles"

## Build        
cmake --build build-vscode --target JX11_All -j8

Targets: 
`JX11_All` - all formats + standalone
`JX11_Standalone`, `JX11_AU`, `JX11_VST3` 

## Outputs 
Built artefacts live under `build-vscode/Source/JX11_artefacts/Debug/`.
Because `COPY_PLUGIN_AFTER_BUILD` is enabled, AU/VST3 are also copied to:     

- `~/Library/Audio/Plug-Ins/Components/JX11.component` 
- `~/Library/Audio/Plug-Ins/VST3/JX11.vst3` 

Run the standalone from:     

     open build-vscode/Source/JX11_artefacts/Debug/Standalone/JX11.app                                    
                                                                              
## Troubleshooting   
- **"generator does not match the generator used previously"** — the build                               
     directory was configured with a different generator. Use a fresh directory                  (e.g. `build-xcode`) or delete `CMakeCache.txt` and `CMakeFiles/`.

    - **C++ IntelliSense can't find `JuceHeader.h`** — re-run configure after                  adding source files; `JuceHeader.h` is generated at build time.  