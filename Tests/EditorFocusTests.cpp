#include <juce_core/juce_core.h>
#include "plugin/PluginProcessor.h"
#include "plugin/PluginEditor.h"

// Regression test: the editor must not take keyboard focus, otherwise opening
// it (or clicking in it) routes the host's key events to the plug-in.
class EditorFocusTests : public juce::UnitTest
{
public:
    EditorFocusTests()
        : juce::UnitTest ("EditorFocus", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("the editor does not take keyboard focus");
        {
            JX11AudioProcessor processor;
            JX11AudioProcessorEditor editor (processor);

            expect (!editor.getWantsKeyboardFocus());
            expect (!editor.getMouseClickGrabsKeyboardFocus());
        }
    }
};

static EditorFocusTests editorFocusTests;
