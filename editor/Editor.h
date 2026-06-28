#pragma once
#pragma clang diagnostic ignored "-Wshadow-field"
#include <JuceHeader.h>
#include "Processor.h"
#include "EventEmitter.hpp"

class Editor : public AudioProcessorEditor, public EventEmitter::Listener {
public:
    Editor(Processor& p);
    ~Editor() override;
    
    auto resized() -> void override;

    auto getResource(const String& url) -> std::optional<WebBrowserComponent::Resource>;
    auto webviewOptions() -> WebBrowserComponent::Options;
    auto getWebviewFileBytes(const String& resourceStr) -> std::vector<std::byte>;

    auto handleEvent(const String& name, const var& payload) -> void override;
    auto handleThemeChange(const String& theme) -> void;
        
private:
    Processor& processor;
    ComponentBoundsConstrainer constrainer;

    WebToggleButtonRelay triggerRelay {Parameters::paramIDs.trigger.getParamID()};
    WebToggleButtonParameterAttachment triggerAttachment {*this->processor.parameters.triggerParam, triggerRelay, nullptr};

    WebSliderRelay stopTimeRelay {Parameters::paramIDs.stopTime.getParamID()};
    WebSliderParameterAttachment stopTimeAttachment {*this->processor.parameters.stopTimeParam, stopTimeRelay, nullptr};

    WebSliderRelay startTimeRelay {Parameters::paramIDs.startTime.getParamID()};
    WebSliderParameterAttachment startTimeAttachment {*this->processor.parameters.startTimeParam, startTimeRelay, nullptr};

    WebComboBoxRelay curveRelay {Parameters::paramIDs.curve.getParamID()};
    WebComboBoxParameterAttachment curveAttachment {*this->processor.parameters.curveParam, curveRelay, nullptr};

    WebBrowserComponent webview;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};