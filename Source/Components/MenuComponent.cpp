#include "MenuComponent.h"

//==============================================================================
MenuComponent::MenuComponent (MainProcess& inMainProcess)
:   mMainProcess (inMainProcess),
    mGlobalState (mMainProcess.getGlobalState()),
    mPresetState (mMainProcess.getPresetState())
{
    mGlobalState.DataMessageBroadcaster::addListener (this, ListenerType::kSync);

    mTitleLabel.setFont (Font().boldened());
    mTitleLabel.setColour (Label::textColourId, COLOR_PURPLE);

    mImages.setDrawableButtonImages (mBackground, "MenuBgLight.svg");
    mImages.setDrawableButtonImages (mNewPresetButton, "MenuNewPreset.svg");
    mImages.setDrawableButtonImages (mDuplicateButton, "MenuDuplicate.svg");
    mImages.setDrawableButtonImages (mImportMidiButton, "MenuImportMidi.svg");
    mImages.setDrawableButtonImages (mExportMidiButton, "MenuExportMidi.svg");
    mImages.setDrawableButtonImages (mImportPresetButton, "MenuImportPreset.svg");
    mImages.setDrawableButtonImages (mExportPresetButton, "MenuExportPreset.svg");
    mImages.setDrawableButtonImages (mCommunityButton, "MenuCommunity.svg");

    mNewPresetButton.setTriggeredOnMouseDown (true);
    mDuplicateButton.setTriggeredOnMouseDown (true);
    mImportMidiButton.setTriggeredOnMouseDown (true);
    mExportMidiButton.setTriggeredOnMouseDown (true);
    mImportPresetButton.setTriggeredOnMouseDown (true);
    mExportPresetButton.setTriggeredOnMouseDown (true);
    mCommunityButton.setTriggeredOnMouseDown (true);

    mNewPresetButton.onClick = [this]()
    {
        mPresetState.handleMouseDownOnNew();
        mGlobalState.toggleMenu();
    };

    mDuplicateButton.onClick = [this]()
    {
        if (!mPresetState.isPresetValid()) { return; }
        mPresetState.handleMouseDownOnDuplicate();
        mGlobalState.toggleMenu();
    };

    mImportMidiButton.onClick = [this]()
    {
        mPresetState.handleMouseDownOnImportMidi();
        mGlobalState.toggleMenu();
    };

    mExportMidiButton.onClick = [this]()
    {
        if (!mPresetState.isPresetValid()) { return; }
        mPresetState.handleMouseDownOnExportMidi();
        mGlobalState.toggleMenu();
    };

    mImportPresetButton.onClick = [this]()
    {
        mPresetState.handleMouseDownOnImportPreset();
        mGlobalState.toggleMenu();
    };

    mExportPresetButton.onClick = [this]()
    {
        if (!mPresetState.isPresetValid()) { return; }
        mPresetState.handleMouseDownOnExportPreset();
        mGlobalState.toggleMenu();
    };

    mCommunityButton.onClick = [this]()
    {
        URL url { "https://trackbout.com/presets" };
        url.launchInDefaultBrowser();
        mGlobalState.toggleMenu();
    };

    addAndMakeVisible (mBackground);
    addAndMakeVisible (mTitleLabel);

    addAndMakeVisible (mNewPresetButton);
    addAndMakeVisible (mDuplicateButton);
    addAndMakeVisible (mImportMidiButton);
    addAndMakeVisible (mExportMidiButton);
    addAndMakeVisible (mImportPresetButton);
    addAndMakeVisible (mExportPresetButton);
    addAndMakeVisible (mCommunityButton);
}

MenuComponent::~MenuComponent()
{
}

//==============================================================================
void MenuComponent::paint (Graphics& inGraphics)
{
    inGraphics.setColour (COLOR_OVERLAY);
    inGraphics.fillRect (getLocalBounds());
}

void MenuComponent::resized()
{
    auto mainArea = getLocalBounds();

    auto titleArea = Styles::getRelativeBounds (mainArea, MENU_X + 44, MENU_ACTION_Y_01 + 2, MENU_ACTION_WIDTH - 4, MENU_ITEM_HEIGHT - 4);
    mTitleLabel.setFont (Font ((float) titleArea.getHeight()).boldened());
    mTitleLabel.setBounds (titleArea);

    mBackground.setBounds (Styles::getRelativeBounds (mainArea, MENU_X, MENU_Y, MENU_WIDTH, MENU_HEIGHT));
    mNewPresetButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_01, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mDuplicateButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_02, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mImportMidiButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_03, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mExportMidiButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_04, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mImportPresetButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_05, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mExportPresetButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_06, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
    mCommunityButton.setBounds (Styles::getRelativeBounds (mainArea, MENU_ACTION_X, MENU_ACTION_Y_07, MENU_ACTION_WIDTH, MENU_ITEM_HEIGHT));
}

//==============================================================================
void MenuComponent::mouseDown (const MouseEvent& inEvent)
{
    mGlobalState.toggleMenu();
}

//==============================================================================
void MenuComponent::handleNewMessage (const DataMessage* inMessage)
{
    switch (inMessage->messageCode)
    {
        case (MessageCode::kToggleMenu): { handleToggleMenu (inMessage); } break;
        default: { } break;
    };
}

void MenuComponent::handleToggleMenu (const DataMessage* inMessage)
{
    if (mGlobalState.isMenuHidden()) { return; }
    bool hasValidPreset = mPresetState.isPresetValid();
    mImages.setDrawableButtonImages (mDuplicateButton, hasValidPreset ? "MenuDuplicate.svg" : "MenuDuplicateOFF.svg");
    mImages.setDrawableButtonImages (mExportMidiButton, hasValidPreset ? "MenuExportMidi.svg" : "MenuExportMidiOFF.svg");
    mImages.setDrawableButtonImages (mExportPresetButton, hasValidPreset ? "MenuExportPreset.svg" : "MenuExportPresetOFF.svg");
}
