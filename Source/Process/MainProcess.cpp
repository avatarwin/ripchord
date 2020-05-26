#include "MainProcess.h"

//==============================================================================
MainProcess::MainProcess()
{
}

MainProcess::~MainProcess()
{
}

//==============================================================================
void MainProcess::handleMidiBuffer (MidiBuffer& inMidiBuffer, int inNumSamples, double inSampleRate)
{
    mMidiState.handleBuffer (inNumSamples, inSampleRate);

    if (mMouseDownBuffer.getNumEvents() > 0)
    {
        transformMidiBuffer (mMouseDownBuffer);
        mMouseDownBuffer.clear();
    }
    else
    {
        transformMidiBuffer (inMidiBuffer);
    }

    if (mMidiState.hasStuckNotes())
    {
        juce::Array<int> stuckNotes = mMidiState.clearStuckNotes();
        for (int& note : stuckNotes) { sendStuckNoteOff (note); }
    }

    handleNoteEventQueue();

    inMidiBuffer.clear();
    inMidiBuffer.swapWith (mTransformedMidiBuffer);
}

//==============================================================================
void MainProcess::handlePlayModeMouseUpOnInput (int inInputNote)
{
    mMouseDownBuffer.addEvent (MidiMessage::noteOff (1, inInputNote), 0);
}

void MainProcess::handlePlayModeMouseDownOnInput (int inInputNote)
{
    mMouseDownBuffer.addEvent (MidiMessage::noteOn (1, inInputNote, 0.8f), 0);
}

//==============================================================================
void MainProcess::transformMidiBuffer (MidiBuffer& inMidiBuffer)
{
    int samplePosition;
    MidiMessage message;
    mTransformedMidiBuffer.clear();

    for (MidiBuffer::Iterator iterator (inMidiBuffer); iterator.getNextEvent (message, samplePosition);)
    {
        mMidiState.setCurrentChannel (message.getChannel());

        if (mGlobalState.isPlayMode() &&
            mControlsState.isTransposeOn() &&
            mControlsState.isTransposeNote (message.getNoteNumber()))
        {
            if (message.isNoteOn()) { handleActiveTransposeNote (message.getNoteNumber()); }
        }
        else
        {
            if (message.isNoteOn()) { handleNoteOn (message); }
            if (message.isNoteOff()) { handleNoteOff (message); }
            if (!message.isNoteOnOrOff()) { handleNonNote (message); }
        }
    }
}

void MainProcess::handleNoteOn (MidiMessage& inMessage)
{
    int inChannel = inMessage.getChannel();
    int inInputNote = inMessage.getNoteNumber();
    float inVelocity = inMessage.getFloatVelocity();
    int inSamplePosition = round (inMessage.getTimeStamp());
    if (inInputNote < 21 || inInputNote > 108) { return; }

    mMidiState.setInputNoteOn (inInputNote);

    if (mPresetState.containsChord (inInputNote))
    {
        juce::Array<int> chordNotes = mPresetState.getChordNotes (inInputNote);
        juce::Array<int> sortedChordNotes = mControlsState.getSortedChordNotes (inInputNote, chordNotes);

        for (int index = 0; index < sortedChordNotes.size(); index++)
        {
            float delayDepth = mControlsState.getDelayDepth();
            float delayVariance = mControlsState.getDelayVariance();
            int activeTransposeNote = mControlsState.getActiveTransposeNote();
            int transposedNote = mControlsState.getTransposedNote (sortedChordNotes[index], activeTransposeNote);
            int chordNote = mGlobalState.isPlayMode() ? transposedNote : sortedChordNotes[index];
            NoteEvent initial { inChannel, inSamplePosition, inVelocity, inInputNote, chordNote };
            NoteEvent noteEvent = mControlsState.setVelocity (initial, index, sortedChordNotes.size());

            if (index == 0 && (delayDepth >= MIN_DELAY_DEPTH || delayVariance >= MIN_DELAY_VARIANCE))
            {
                mMidiState.addSampleCounter (noteEvent.inputNote);
            }

            if (mGlobalState.isEditMode() || index == 0 ||
               (delayDepth < MIN_DELAY_DEPTH && delayVariance < MIN_DELAY_VARIANCE))
            {
                sendOutputNoteOn (noteEvent);
            }
            else
            {
                mMidiState.addNoteEventToQueue (noteEvent, index, delayDepth, delayVariance);
            }
        }
    }
    else
    {
        NoteEvent noteEvent { inChannel, inSamplePosition, inVelocity, inInputNote, inInputNote };
        sendOutputNoteOn (noteEvent);
    }
}

void MainProcess::handleNoteOff (MidiMessage& inMessage)
{
    int inChannel = inMessage.getChannel();
    int inInputNote = inMessage.getNoteNumber();
    float inVelocity = inMessage.getFloatVelocity();
    int inSamplePosition = round (inMessage.getTimeStamp());
    if (inInputNote < 21 || inInputNote > 108) { return; }

    mMidiState.setInputNoteOff (inInputNote);

    if (mPresetState.containsChord (inInputNote))
    {
        mMidiState.removeSampleCounter (inInputNote);
        mMidiState.removeNoteEventsFromQueue (inInputNote);
        juce::Array<int> chordNotes = mPresetState.getChordNotes (inInputNote);

        for (int index = 0; index < chordNotes.size(); index++)
        {
            int activeTransposeNote = mControlsState.getActiveTransposeNote();
            int transposedNote = mControlsState.getTransposedNote (chordNotes[index], activeTransposeNote);
            int chordNote = mGlobalState.isPlayMode() ? transposedNote : chordNotes[index];

            NoteEvent noteEvent { inChannel, inSamplePosition, inVelocity, inInputNote, chordNote };
            sendOutputNoteOff (noteEvent);
        }
    }
    else
    {
        NoteEvent noteEvent { inChannel, inSamplePosition, inVelocity, inInputNote, inInputNote };
        sendOutputNoteOff (noteEvent);
    }
}

void MainProcess::handleNonNote (MidiMessage& inMessage)
{
    int inSamplePosition = round (inMessage.getTimeStamp());
    mTransformedMidiBuffer.addEvent (inMessage, inSamplePosition);
}

//==============================================================================
void MainProcess::sendOutputNoteOn (NoteEvent inNoteEvent)
{
    juce::Array<int> triggers = mMidiState.getOutputNoteTriggers (inNoteEvent.outputNote);

    if (triggers.size() == 1)
    {
        const auto& message = MidiMessage::noteOff (inNoteEvent.channel, inNoteEvent.outputNote);
        mTransformedMidiBuffer.addEvent (message, inNoteEvent.samplePosition);
    }

    if (triggers.size() < 2)
    {
        const auto& message = MidiMessage::noteOn (inNoteEvent.channel, inNoteEvent.outputNote, inNoteEvent.velocity);
        mTransformedMidiBuffer.addEvent (message, inNoteEvent.samplePosition);

        triggers.add (inNoteEvent.inputNote);
        mMidiState.setOutputNoteOn (inNoteEvent.outputNote, triggers);
    }
}

void MainProcess::sendOutputNoteOff (NoteEvent inNoteEvent)
{
    juce::Array<int> triggers = mMidiState.getOutputNoteTriggers (inNoteEvent.outputNote);
    if (triggers.indexOf (inNoteEvent.inputNote) < 0) { return; }

    if (triggers.size() == 1)
    {
        const auto& message = MidiMessage::noteOff (inNoteEvent.channel, inNoteEvent.outputNote);
        mTransformedMidiBuffer.addEvent (message, inNoteEvent.samplePosition);
    }

    if (triggers.size() <= 2)
    {
        triggers.removeFirstMatchingValue (inNoteEvent.inputNote);
        mMidiState.setOutputNoteOff (inNoteEvent.outputNote, triggers);
    }
}

void MainProcess::sendStuckNoteOff (int inOutputNote)
{
    int channel = mMidiState.getCurrentChannel();
    mTransformedMidiBuffer.addEvent (MidiMessage::noteOff (channel, inOutputNote), 0);
}

//==============================================================================
void MainProcess::handleNoteEventQueue()
{
    if (mGlobalState.isPlayMode() && mMidiState.timeToSendNextNoteEvent())
    {
        sendOutputNoteOn (mMidiState.getNextNoteEvent());
    }
}

//==============================================================================
void MainProcess::handleActiveTransposeNote (int inInputNote)
{
    if (inInputNote == mControlsState.getTransposeBase() + 12) { return; }
    if (mMidiState.getCurrentlyOnInputNotes().size() > 0) { return; }

    else if (mControlsState.getActiveTransposeNote() != inInputNote)
    {
        mMidiState.setActiveTransposeNoteIfAllowed (inInputNote);
    }

    else if (mControlsState.getActiveTransposeNote() == inInputNote)
    {
        mMidiState.setActiveTransposeNoteIfAllowed (-1);
    }
}

//==============================================================================
XmlElement* MainProcess::exportSessionXml()
{
    XmlElement* sessionXml = new XmlElement ("Session");
    sessionXml->addChildElement (mControlsState.exportControlsStateXml());
    sessionXml->addChildElement (mPresetState.exportPresetStateXml());
    return sessionXml;
}

void MainProcess::importSessionXml (XmlElement* inSessionXml)
{
    if (inSessionXml->getTagName() != "Session") { return; }

    XmlElement* controlsStateXml = inSessionXml->getChildByName("ControlsState");
    XmlElement* presetStateXml = inSessionXml->getChildByName("PresetState");

    mControlsState.importControlsStateXml (controlsStateXml);
    mPresetState.importPresetStateXml (presetStateXml);
}
