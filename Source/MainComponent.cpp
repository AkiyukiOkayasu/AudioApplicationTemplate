#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    setSize (800, 450);
    if(JUCE_MAC) setMacMainMenu(this);
    formatManager.registerBasicFormats();
    transportSource.setLooping(true);
    
    //GUI
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
        btn_inputSelector[i].setColour (TextButton::buttonOnColourId, Colours::limegreen);
        btn_inputSelector[i].addListener(this);
    }
    btn_inputSelector[0].setButtonText("Audio File");
    btn_inputSelector[1].setButtonText("Input Stream");
    btn_inputSelector[0].setToggleState(true, NotificationType::dontSendNotification);
    
    addAndMakeVisible(btn_play);
    btn_play.setButtonText("Play");
    btn_play.setColour(TextButton::buttonColourId, Colours::limegreen);
    btn_play.setEnabled(false);
    btn_play.addListener(this);
    
    addAndMakeVisible(btn_stop);
    btn_stop.setButtonText("Stop");
    btn_stop.setColour(TextButton::buttonColourId, Colours::darkgrey);
    btn_stop.setEnabled(false);
    btn_stop.addListener(this);
    
    addAndMakeVisible(btn_open);
    btn_open.setButtonText("Open");
    btn_open.addListener(this);
    
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
    if (JUCE_MAC) setMacMainMenu(nullptr);
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
    
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if  (isStreamingInput)
    {
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
        if (readerSource != nullptr)
        {
            transportSource.getNextAudioBlock(bufferToFill);
        }
    }
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
    auto inputSelectorBounds = r.removeFromTop(40);
    btn_inputSelector[0].setBounds(inputSelectorBounds.removeFromLeft(inputSelectorBounds.getWidth() / 2));
    btn_inputSelector[1].setBounds(inputSelectorBounds);
    auto transportBounds = r.removeFromTop(40);
    const int buttonWidth = transportBounds.getWidth() / 3;
    btn_open.setBounds(transportBounds.removeFromLeft(buttonWidth));
    btn_play.setBounds(transportBounds.removeFromLeft(buttonWidth));
    btn_stop.setBounds(transportBounds);
    
    auto sliderBounds = r.removeFromBottom(60);
    sl_outputVolume.setBounds(sliderBounds);
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &sl_outputVolume) outputVolume.setGainDecibels(slider->getValue());
}

void MainContentComponent::buttonClicked (Button* button)
{
    if (button == &btn_inputSelector[0])
    {
        //Audio file
        isStreamingInput = false;
    }
    else if(button == &btn_inputSelector[1])
    {
        //Input stream
        isStreamingInput = true;
    }
    else if(button == &btn_open)
    {
        if (transportSource.isPlaying()) transportSource.stop();
        openAudioFile();
    }
    else if(button == &btn_play)
    {
        if(!transportSource.isPlaying()) transportSource.start();
    }
    else if(button == &btn_stop)
    {
        if (transportSource.isPlaying()) transportSource.stop();
    }
}

void MainContentComponent::openAudioFile()
{
    FileChooser chooser("Select audio file to play", File(), formatManager.getWildcardForAllFormats());
    
    if (chooser.browseForFileToOpen())
    {
        File file(chooser.getResult());
        ScopedPointer<AudioFormatReader> reader = formatManager.createReaderFor(file);
        
        if (reader != nullptr)
        {
            if (readerSource != nullptr) readerSource.release();
            readerSource = std::make_unique<AudioFormatReaderSource>(reader, true);
            readerSource->setLooping(true);
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
            btn_play.setEnabled(true);
            btn_stop.setEnabled(true);
        }
        reader.release();
    }
    
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
