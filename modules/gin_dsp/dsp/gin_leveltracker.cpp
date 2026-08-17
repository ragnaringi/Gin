/*
 ==============================================================================

 This file is part of the GIN library.
 Copyright (c) 2018 - 2026 by Roland Rabien.

 ==============================================================================
 */

//==============================================================================
LevelTracker::LevelTracker (float decayPerSecond)
    : decayRate (decayPerSecond)
{
}

void LevelTracker::trackBuffer (juce::AudioSampleBuffer& buffer)
{
    for (int i = 0; i < buffer.getNumChannels(); i++)
        trackBuffer (buffer.getReadPointer (i), buffer.getNumSamples());
}

void LevelTracker::trackBuffer (const float* buffer, int numSamples)
{
    juce::Range<float> range = juce::FloatVectorOperations::findMinAndMax (buffer, numSamples);
    float v1 = std::fabs (range.getStart());
    float v2 = std::fabs (range.getEnd());

    float peakDB = juce::Decibels::gainToDecibels (juce::jmax (v1, v2));

    if (peakDB > 0)
        clip.store (true, std::memory_order_relaxed);

    if (decayRate < 0)
    {
        if (peakDB < getLevel())
        {
            const double time = juce::Time::getMillisecondCounterHiRes() / 1000.0;

            peakLevel.store (peakDB, std::memory_order_relaxed);
            peakTime.store (time, std::memory_order_relaxed);
        }
    }
    else
    {
        if (peakDB > getLevel())
        {
            const double time = juce::Time::getMillisecondCounterHiRes() / 1000.0;

            peakLevel.store (peakDB, std::memory_order_relaxed);
            peakTime.store (time, std::memory_order_relaxed);
        }
    }
}

void LevelTracker::trackSample (float f)
{
    float peakDB = juce::Decibels::gainToDecibels (std::abs (f));

    if (peakDB > 0)
        clip.store (true, std::memory_order_relaxed);

    if (decayRate < 0)
    {
        if (peakDB < getLevel())
        {
            const double time = juce::Time::getMillisecondCounterHiRes() / 1000.0;

            peakLevel.store (peakDB, std::memory_order_relaxed);
            peakTime.store (time, std::memory_order_relaxed);
        }
    }
    else
    {
        if (peakDB > getLevel())
        {
            const double time = juce::Time::getMillisecondCounterHiRes() / 1000.0;

            peakLevel.store (peakDB, std::memory_order_relaxed);
            peakTime.store (time, std::memory_order_relaxed);
        }
    }
}

float LevelTracker::getLevel() const
{
    const double hold = 0.05; // 50ms

    const double now     = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    const double elapsed = now - peakTime.load (std::memory_order_relaxed);

    if (elapsed < hold)
        return peakLevel.load (std::memory_order_relaxed);

    return peakLevel.load (std::memory_order_relaxed)
         - decayRate * float (elapsed - hold);
}
