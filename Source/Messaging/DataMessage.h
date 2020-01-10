#pragma once

#include "JuceHeader.h"

class DataMessageBroadcaster;

enum ListenerType
{
    kSync = 0,
    kAsync
};

enum MessageCode
{
    kGeneric = 0,

    // Messages from GlobalState
    kModeUpdated,
    kViewUpdated,
    kMenuUpdated,

    // Messages from PresetState
    kChordNameUpdated,
    kPresetNameUpdated,
    kEditModeInputNote,
    kEditModeOutputNotes,

    // Messages from MidiState
    kCurrentlyOnInputNotes,
    kCurrentlyOnOutputNotes
};

//==============================================================================
class DataMessage : public Message
{
public:
    //==============================================================================
    DataMessage();
    ~DataMessage();

    //==============================================================================
    DataMessageBroadcaster* broadcaster;
    MessageCode messageCode;

    int listenerType;
    var messageVar1;
    var messageVar2;
    var messageVar3;
    var messageVar4;
    juce::Array<int> messageArray1;
    juce::Array<int> messageArray2;

private:
    //==============================================================================

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataMessage);
};
