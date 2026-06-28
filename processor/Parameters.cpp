#include "Parameters.h"
#include "Functions.hpp"

template<typename T>
static auto castParameter(const AudioProcessorValueTreeState& tree, 
    const ParameterID* id, T*& dest) -> void {
    dest = dynamic_cast<T*>(tree.getParameter(id->getParamID()));
    jassert(dest != nullptr);
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterFloat* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = paramObj->getDefaultValue();
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterBool* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = paramObj->getDefaultValue();
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterChoice* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = static_cast<T>(paramObj->getDefaultValue());
}

ParameterIDs Parameters::paramIDs = ParameterIDs::loadFromJSON();

Parameters::Parameters(AudioProcessorValueTreeState& tree) : tree(tree) {
    using FloatPair = std::pair<AudioParameterFloat*&, const ParameterID*>;
    using BoolPair = std::pair<AudioParameterBool*&, const ParameterID*>;
    using ChoicePair = std::pair<AudioParameterChoice*&, const ParameterID*>;

    auto floatParameters = std::vector<FloatPair>{
        {stopTimeParam, &paramIDs.stopTime},
        {startTimeParam, &paramIDs.startTime}
    };

    auto boolParameters = std::vector<BoolPair>{
        {triggerParam, &paramIDs.trigger}
    };

    auto choiceParameters = std::vector<ChoicePair>{
        {curveParam, &paramIDs.curve}
    };

    for (const auto& [param, paramID] : floatParameters) {
        castParameter(tree, paramID, param);
    }

    for (auto& [param, paramID] : boolParameters) {
        castParameter(tree, paramID, param);
    }

    for (auto& [param, paramID] : choiceParameters) {
        castParameter(tree, paramID, param);
    }
}

auto Parameters::createParameterLayout() -> AudioProcessorValueTreeState::ParameterLayout {
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterBool>(
        paramIDs.trigger, "Trigger", false
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.stopTime, "Stop Time", NormalisableRange<float>{0.0f, 2000.0f, 1.0f}, 1000.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayMilliseconds)
        .withValueFromStringFunction(Functions::parseMilliseconds)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.startTime, "Start Time", NormalisableRange<float>{0.0f, 2000.0f, 1.0f}, 1000.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayMilliseconds)
        .withValueFromStringFunction(Functions::parseMilliseconds)
    ));

    layout.add(std::make_unique<AudioParameterChoice>(
        paramIDs.curve, "Curve", StringArray{"exponential", "linear", "logarithmic"}, 0
    ));

    return layout;
}

auto Parameters::getDefaultParameter(const Array<var>& args,
    WebBrowserComponent::NativeFunctionCompletion completion) -> void {

    auto paramID = args[0].toString();
    auto* param = this->tree.getParameter(paramID);
    float defaultValue = param->convertFrom0to1(param->getDefaultValue());

    completion(defaultValue);
}

auto Parameters::prepareToPlay(double sampleRate, int blockSize) noexcept -> void {
    this->sampleRate = sampleRate;
    this->blockSize = blockSize;

    double duration = 0.001;

    auto smoothers = std::vector{
        &stopTimeSmoother,
        &startTimeSmoother
    };

    for (const auto& smoother : smoothers) {
        smoother->reset(this->sampleRate, duration);
    }
}

auto Parameters::reset() noexcept -> void {
    auto paramFloats = std::vector{
        std::pair{stopTimeParam, &stopTime},
        std::pair{startTimeParam, &startTime}
    };

    for (auto& [param, value] : paramFloats) {
        resetParameter(tree, param, value);
    }
    
    auto smoothers = std::vector{
        std::pair{stopTimeParam, &stopTimeSmoother},
        std::pair{startTimeParam, &startTimeSmoother}
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setCurrentAndTargetValue(param->get());
    }
}

auto Parameters::setHostInfo(double bpm, double ppq, const AudioPlayHead::TimeSignature& timeSignature) noexcept -> void {
    this->bpm = bpm;
    this->ppq = ppq;
    this->timeSignature = timeSignature;

    if (ppq > 0.0) {
        this->ppq = ppq;
        this->internalPPQ = ppq;
    } else {
        double ppqPerSample = (this->bpm / 60.0) / this->sampleRate;
        this->internalPPQ += ppqPerSample * this->blockSize; 
        this->ppq = this->internalPPQ;
    }
}

auto Parameters::blockUpdate() noexcept -> void {
    auto smoothers = std::vector{
        std::pair{stopTimeParam, &stopTimeSmoother},
        std::pair{startTimeParam, &startTimeSmoother}
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setTargetValue(param->get());
    }

    this->trigger = this->triggerParam->get();
    this->curve = this->curveParam->getCurrentChoiceName();
}

auto Parameters::update() noexcept -> void {
    this->stopTime = stopTimeSmoother.getNextValue();
    this->startTime = startTimeSmoother.getNextValue();
}