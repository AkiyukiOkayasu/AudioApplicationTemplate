#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    setSize (800, 450);
    setMacMainMenu(this);
    
    //出力ボリュームを調整するスライダーの設定
    addAndMakeVisible(slider_outputVolume);
    slider_outputVolume.setRange(-72.0, 0.0, 0.1);
    slider_outputVolume.setTextValueSuffix("dB");
    slider_outputVolume.setSkewFactor(1.5);
    slider_outputVolume.setValue(-12.0);
    slider_outputVolume.addListener(this);
    
    
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
    outputVolume.setGainDecibels(slider_outputVolume.getValue());
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
    slider_outputVolume.setBounds(r);
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &slider_outputVolume) outputVolume.setGainDecibels(slider->getValue());
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
