#pragma once
#include <JuceHeader.h>

class TapeStop {
public:
    TapeStop() = default;
    ~TapeStop() = default;

    auto prepareToPlay(double sampleRate, double maxLength = 10.0) -> void {
        this->sampleRate = sampleRate;
        this->tapeBufferSize = static_cast<int>(sampleRate * maxLength);

        this->tapeBuffer.setSize(2, tapeBufferSize);
        this->reset();
    }

    auto reset() -> void {
        this->tapeBuffer.clear();
        this->speed = 1.0f;

        this->writePos = 0;
        this->readPos = 0.0f;

        this->lastTrigger = false;

        this->stopping = false;
        this->starting = false;

        this->stopProgress = 0.0f;
        this->startProgress = 0.0f;

        this->lowpassL = 0.0f;
        this->lowpassR = 0.0f;
    }


    auto setCurve(const String& curve) -> void {
        this->curve = curve;
    }

    auto setStopTime(float stopTimeMS) -> void {
        this->stopTimeMS = stopTimeMS;
    }

    auto setStartTime(float startTimeMS) -> void {
        this->startTimeMS = startTimeMS;
    }

    auto setTrigger(bool trigger) -> void {
        if (trigger && !this->lastTrigger) {
            this->stopping = true;
            this->starting = false;
            this->stopProgress = 0.0f;
            this->readPos = static_cast<float>(this->writePos);
        }

        if (!trigger && this->lastTrigger) {
            this->starting = true;
            this->stopping = false;
            this->startProgress = 0.0f;
            this->readPos = static_cast<float>(this->writePos);
        }

        this->lastTrigger = trigger;
    }

    auto applyCurve(float t) const -> float {
        if (this->curve == "linear") {
            return t;
        } else if (this->curve == "exponential") {
            return 1.0f - std::exp(-6.0f * t);
        } else if (this->curve == "logarithmic") {
            return std::log10(1.0f + 9.0f * t);
        }

        return t;
    }

    auto calculateSpeed() -> void {
        if (this->stopping) {
            float durationSamples = this->stopTimeMS * 0.001f * static_cast<float>(this->sampleRate);
            durationSamples = std::max(1.0f, durationSamples);

            this->stopProgress += 1.0f / durationSamples;
            this->stopProgress = juce::jlimit(0.0f, 1.0f, this->stopProgress);

            this->speed = 1.0f - this->applyCurve(stopProgress);

             if (this->stopProgress >= 1.0f) {
                this->speed = 0.0f;
                this->stopping = false;
            }

        } else if (this->starting) {
            float durationSamples = this->startTimeMS * 0.001f * static_cast<float>(this->sampleRate);
            durationSamples = std::max(1.0f, durationSamples);

            startProgress += 1.0f / durationSamples;
            startProgress = juce::jlimit(0.0f, 1.0f, this->startProgress);

            this->speed = this->applyCurve(startProgress);

            if (this->startProgress >= 1.0f) {
                this->speed = 1.0f;
                this->starting = false;
            }
        }
    }

    auto lowpassSample(float sample, float& lowpass) -> float {
        float cutoff = juce::jmap(speed, 0.0f, 1.0f, 500.0f, 20000.0f);

        float alpha = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 
            cutoff / static_cast<float>(sampleRate));

        lowpass += alpha * (sample - lowpass);

        return lowpass;
    }

    auto processSample(float inL, float inR, float& outL, float& outR) -> void {
        this->calculateSpeed();

        if (!stopping && !starting && speed >= 0.9999f) {
            outL = inL;
            outR = inR;

            this->tapeBuffer.setSample(0, this->writePos, inL);
            this->tapeBuffer.setSample(1, this->writePos, inR);

            this->writePos = (this->writePos + 1) % tapeBufferSize;
            this->readPos = static_cast<float>(this->writePos);

            return;
        }

        this->tapeBuffer.setSample(0, this->writePos, inL);
        this->tapeBuffer.setSample(1, this->writePos, inR);

        int pos1 = static_cast<int>(this->readPos);
        int pos2 = (pos1 + 1) % this->tapeBufferSize;

        float frac = this->readPos - static_cast<float>(pos1);

        float left =
            this->tapeBuffer.getSample(0, pos1) * (1.0f - frac) +
            this->tapeBuffer.getSample(0, pos2) * frac;

        float right =
            this->tapeBuffer.getSample(1, pos1) * (1.0f - frac) +
            this->tapeBuffer.getSample(1, pos2) * frac;

        outL = this->lowpassSample(left, lowpassL);
        outR = this->lowpassSample(right, lowpassR);

        this->writePos++;
        if (this->writePos >= this->tapeBufferSize) {
            this->writePos = 0;
        }

        this->readPos += this->speed;
        while (this->readPos >= static_cast<float>(this->tapeBufferSize)) {
            this->readPos -= static_cast<float>(this->tapeBufferSize);
        }
    }

private:
    double sampleRate = 44100.0;
    String curve = "exponential";
    float stopTimeMS = 500.0f;
    float startTimeMS = 500.0f;

    AudioBuffer<float> tapeBuffer;
    int tapeBufferSize = 0;

    float speed = 1.0f;
    bool lastTrigger = false;

    int writePos = 0;
    float readPos = 0.0f;

    bool stopping = false;
    bool starting = false;

    float stopProgress = 0.0f;
    float startProgress = 0.0f;

    float lowpassL = 0.0f;
    float lowpassR = 0.0f;
};