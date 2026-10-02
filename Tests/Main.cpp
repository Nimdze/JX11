#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    return failures == 0 ? 0 : 1;
}