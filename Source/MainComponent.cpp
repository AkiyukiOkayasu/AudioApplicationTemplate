#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    setSize (800, 450);
    setMacMainMenu(this);
    
    //出力ボリュームを調整するスライダーの設定
    addAndMakeVisible(sl_outputVolume);
    sl_outputVolume.setRange(-72.0, 0.0, 0.1);
    sl_outputVolume.setTextValueSuffix("dB");
    sl_outputVolume.setSkewFactor(1.5);
    sl_outputVolume.setValue(-12.0);
    sl_outputVolume.addListener(this);
    
    for(int i = 0; i < 2; ++i)
    {
        addAndMakeVisible(btn_inputSelector[i]);
        btn_inputSelector[i].setClickingTogglesState(true);
        btn_inputSelector[i].setRadioGroupId(34567);
        btn_inputSelector[i].setColour (TextButton::buttonOnColourId, Colours::grey);
        btn_inputSelector[i].addListener(this);
    }
    btn_inputSelector[0].setButtonText("Audio File");
    btn_inputSelector[1].setButtonText("Input Stream");
    btn_inputSelector[0].setToggleState(true, NotificationType::dontSendNotification);
    
    addAndMakeVisible(btn_play);
    btn_play.setButtonText("Play");
    btn_play.addListener(this);
    
    addAndMakeVisible(btn_stop);
    btn_stop.setButtonText("Stop");
    btn_stop.addListener(this);
    
    addAndMakeVisible(btn_open);
    btn_open.setButtonText("Open");
    btn_open.addListener(this);
    
    formatManager.registerBasicFormats();
    
    //保存したパラメータをXMLファイルから呼び出し
    PropertiesFile::Options options;
    options.applicationName     = ProjectInfo::projectName;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Preferences";
    appProperties = new ApplicationProperties();
    appProperties->setStorageParameters (options);
    ScopedPointer<XmlElement> savedAudioState (appProperties->getUserSettings()->getXmlValue ("audioDeviceState"));//オーディオインターフェースの設定
    
    setAudioChannels (2, 2, savedAudioState);
}

MainContentComponent::~MainContentComponent()
{
    setMacMainMenu(nullptr);
    shutdownAudio();
}

//==============================================================================
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    auto channels = static_cast<uint32>(2);//2ch
    dsp::ProcessSpec spec {sampleRate, static_cast<uint32>(samplesPerBlockExpected), channels};
    outputVolume.prepare(spec);
    outputVolume.reset();
    outputVolume.setRampDurationSeconds(0.05);//ボリュームが変更時は50msかけてスムージングをかける
    outputVolume.setGainDecibels(sl_outputVolume.getValue());
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    dsp::AudioBlock<float> block(*bufferToFill.buffer);
    dsp::ProcessContextReplacing<float> context(block);
    outputVolume.process(context);
}

void MainContentComponent::releaseResources()
{
    
}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainContentComponent::resized()
{
    auto r = getLocalBounds();
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &sl_outputVolume) outputVolume.setGainDecibels(slider->getValue());
}

void MainContentComponent::buttonClicked (Button* button)
{
    
}

StringArray MainContentComponent::getMenuBarNames()
{
    const char* const names[] = { "Options", nullptr };
    return StringArray (names);
}

PopupMenu MainContentComponent::getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/)
{
    PopupMenu menu;
    if (topLevelMenuIndex == 0) menu.addItem(1, "Audio Pref.");
    return menu;
}

void MainContentComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == 1) showAudioSettings();
}
