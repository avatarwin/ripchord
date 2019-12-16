#pragma once

#include "JuceHeader.h"

class DataMessage;

//==============================================================================
class DataMessageListener
{
public:
    //==============================================================================
    DataMessageListener();

    virtual ~DataMessageListener();

    virtual void messageReceived (const DataMessage* inMessage) = 0;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataMessageListener);
};