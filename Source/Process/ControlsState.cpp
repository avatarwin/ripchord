#include "ControlsState.h"

//==============================================================================
ControlsState::ControlsState()
{
}

ControlsState::~ControlsState()
{
}

//==============================================================================
bool ControlsState::isTransposeOff()
{
    return mTranspose == Transpose::Off;
}

bool ControlsState::isTransposeEnabled()
{
    return mTranspose == Transpose::Enabled;
}

bool ControlsState::isTransposeLatched()
{
    return mTranspose == Transpose::Latched;
}

//==============================================================================
int ControlsState::getTransposeBase()
{
    return mTransposeBase;
}

bool ControlsState::isTransposeNote (const int inInputNote)
{
    return mTransposeBase <= inInputNote && inInputNote < (mTransposeBase + 25);
}

//==============================================================================
void ControlsState::toggleTranspose()
{
    switch (mTranspose)
    {
        case (Transpose::Off): { mTranspose = Transpose::Enabled; } break;
        case (Transpose::Enabled): { mTranspose = Transpose::Latched; } break;
        case (Transpose::Latched): { mTranspose = Transpose::Off; } break;
    };

    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kToggleTranspose;
    message->messageVar1 = mTransposeBase;
    sendMessage (message, ListenerType::kSync);
}

void ControlsState::handleMouseClickOnShiftLeft()
{
    if (mTransposeBase == 21 || isTransposeOff()) { return; }

    int prevTransposeBase = mTransposeBase;
    int nextTransposeBase = mTransposeBase - 1;

    mTransposeBase = nextTransposeBase;

    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kTransposeBaseChanged;
    message->messageVar1 = prevTransposeBase;
    message->messageVar2 = nextTransposeBase;
    sendMessage (message, ListenerType::kSync);
}

void ControlsState::handleMouseClickOnShiftRight()
{
    if (mTransposeBase == 84 || isTransposeOff()) { return; }

    int prevTransposeBase = mTransposeBase;
    int nextTransposeBase = mTransposeBase + 1;

    mTransposeBase = nextTransposeBase;

    DataMessage* message = new DataMessage();
    message->messageCode = MessageCode::kTransposeBaseChanged;
    message->messageVar1 = prevTransposeBase;
    message->messageVar2 = nextTransposeBase;
    sendMessage (message, ListenerType::kSync);
}