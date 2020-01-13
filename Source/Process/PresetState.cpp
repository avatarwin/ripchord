#include "PresetState.h"

//==============================================================================
PresetState::PresetState()
:   mUserDataPath (System::getUserDataPath (ProjectInfo::companyName, ProjectInfo::projectName)),
    mPresetFolder (mUserDataPath.getChildFile ("Presets"))
{
}

PresetState::~PresetState()
{
}

//==============================================================================
bool PresetState::isPresetValid()
{
    if (mName.isEmpty()) { return false; }

    juce::Array<int> chordNotes;

    for (int inputNote : getPresetInputNotes())
    {
        for (int chordNote : getChordNotes (inputNote))
        {
            chordNotes.add (chordNote);
        }
    }

    return chordNotes.size() > 0;
}

void PresetState::resetEditModeInputNote()
{
    mEditModeInputNote = 0;
}

const int PresetState::getEditModeInputNote()
{
    return mEditModeInputNote;
}

juce::Array<int> PresetState::getPresetInputNotes()
{
    juce::Array<int> presetInputNotes;
    std::map<int, Chord>::iterator pair;

    for (pair = mChords.begin(); pair != mChords.end(); ++pair)
    {
      presetInputNotes.add (pair->first);
    }

    return presetInputNotes;
}

//==============================================================================
bool PresetState::containsChord (const int inInputNote)
{
    return mChords.count (inInputNote) > 0;
}

String PresetState::getChordName (const int inInputNote)
{
    return getChord (inInputNote).name;
}

juce::Array<int> PresetState::getChordNotes (const int inInputNote)
{
    return getChord (inInputNote).notes;
}

//==============================================================================
void PresetState::handleEditModeMouseDownOnInput (const int inInputNote)
{
    const int prevEditModeInputNote = mEditModeInputNote;
    const int nextEditModeInputNote = inInputNote == mEditModeInputNote ? 0 : inInputNote;
    bool prevEditModeInputNoteContainsChord = containsChord (prevEditModeInputNote);
    juce::Array<int> prevEditModeOutputNotes = getChordNotes (prevEditModeInputNote);
    juce::Array<int> nextEditModeOutputNotes = getChordNotes (nextEditModeInputNote);

    mEditModeInputNote = nextEditModeInputNote;

    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kEditModeInputNote;
    message->messageVar1 = prevEditModeInputNote;
    message->messageVar2 = nextEditModeInputNote;
    message->messageVar3 = prevEditModeInputNoteContainsChord;
    message->messageArray1 = prevEditModeOutputNotes;
    message->messageArray2 = nextEditModeOutputNotes;
    sendMessage (message, ListenerType::kSync);
}

void PresetState::handleEditModeMouseDownOnOutput (const int inOutputNote)
{
    if (mEditModeInputNote == 0) { return; }

    juce::Array<int> prevEditModeOutputNotes = getChordNotes (mEditModeInputNote);
    bool shouldAddNote = !prevEditModeOutputNotes.contains (inOutputNote);

    if (shouldAddNote)
    {
        Chord presetChord = getChord (mEditModeInputNote);
        presetChord.notes.add (inOutputNote);
        setChord (mEditModeInputNote, presetChord);
    }
    else
    {
        if (prevEditModeOutputNotes.size() > 1)
        {
            Chord presetChord = getChord (mEditModeInputNote);
            presetChord.notes.removeFirstMatchingValue (inOutputNote);
            setChord (mEditModeInputNote, presetChord);
        }
        else
        {
            mChords.erase (mEditModeInputNote);
        }
    }

    juce::Array<int> nextEditModeOutputNotes = getChordNotes (mEditModeInputNote);

    mIsPresetDataModified = true;
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kEditModeOutputNotes;
    message->messageArray1 = prevEditModeOutputNotes;
    message->messageArray2 = nextEditModeOutputNotes;
    sendMessage (message, ListenerType::kSync);
}

//==============================================================================
void PresetState::handleChordNameTextChanged (String inChordName)
{
    Chord presetChord = getChord (mEditModeInputNote);
    if (mEditModeInputNote == 0 || presetChord.name == inChordName) { return; }

    presetChord.name = inChordName;
    setChord (mEditModeInputNote, presetChord);

    mIsPresetDataModified = true;
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kChordNameUpdated;
    message->messageVar1 = inChordName;
    sendMessage (message, ListenerType::kSync);
}

void PresetState::handlePresetNameTextChanged (String inPresetName)
{
    if (mName == inPresetName) { return; }

    mName = inPresetName;

    mIsPresetNameModified = true;
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kPresetNameUpdated;
    message->messageVar1 = inPresetName;
    sendMessage (message, ListenerType::kSync);
}

//==============================================================================
void PresetState::handleMouseClickOnSave()
{
    if (!isPresetValid()) { return; }
    else if (!isPresetCreated()) { createPresetFile(); }
    else if (isPresetNameModified()) { renamePresetFile(); }
    else if (isPresetDataModified()) { updatePresetFile(); }
}

//==============================================================================
Chord PresetState::getChord (const int inInputNote)
{
    auto pair = mChords.find (inInputNote);
    if (pair == mChords.end()) { return mEmptyChord; }
    return pair->second;
}

void PresetState::setChord (const int inInputNote, Chord inChord)
{
    mChords[inInputNote] = inChord;
}

//==============================================================================
bool PresetState::isPresetCreated()
{
    return mIsPresetCreated;
}

bool PresetState::isPresetNameModified()
{
    return mIsPresetNameModified;
}

bool PresetState::isPresetDataModified()
{
    return mIsPresetDataModified;
}

void PresetState::setPresetFileSaved()
{
    mIsPresetCreated = true;
    mIsPresetNameModified = false;
    mIsPresetDataModified = false;
}

//==============================================================================
void PresetState::createPresetFile()
{
    DBG ("CREATE PRESET FILE");

    setPresetFileSaved();
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kPresetFileSaved;
    sendMessage (message, ListenerType::kSync);
}

void PresetState::renamePresetFile()
{
    DBG ("RENAME PRESET FILE");

    setPresetFileSaved();
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kPresetFileSaved;
    sendMessage (message, ListenerType::kSync);
}

void PresetState::updatePresetFile()
{
    DBG ("UPDATE PRESET FILE");

    setPresetFileSaved();
    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kPresetFileSaved;
    sendMessage (message, ListenerType::kSync);
}

void PresetState::deletePresetFile()
{
    DBG ("DELETE PRESET FILE");
}
