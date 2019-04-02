#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class MainContentComponent   :
public AudioAppComponent,
public Slider::Listener,
public Button::Listener,
public MenuBarModel
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    void sliderValueChanged (Slider* slider) override;
    void buttonClicked (Button* button) override;
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
    void showAudioSettings()
    {
        AudioDeviceSelectorComponent audioSettingsComp (deviceManager,
                                                        2, 2,//InputChannels: min/max
                                                        2, 2,//OutputChannels: min/max
                                                        false,//Show MIDI input options
                                                        false,//Show MIDI output selector
                                                        true,//Stereo pair
                                                        false//Hide advanced option with button
                                                        );
        
        audioSettingsComp.setSize (450, 250);
        DialogWindow::LaunchOptions o;
        o.content.setNonOwned (&audioSettingsComp);
        o.dialogTitle                   = "Audio Settings";
        o.componentToCentreAround       = this;
        o.dialogBackgroundColour        = Colours::grey;
        o.escapeKeyTriggersCloseButton  = false;
        o.useNativeTitleBar             = true;
        o.resizable                     = false;
        o.runModal();
        
        //設定をXMLファイルで保存
        ScopedPointer<XmlElement> audioState (deviceManager.createStateXml());
        appProperties->getUserSettings()->setValue ("audioDeviceState", audioState);
        appProperties->getUserSettings()->saveIfNeeded();
    }
    
private:
    void openAudioFile();
    
    dsp::Gain<float> outputVolume;
    Slider sl_outputVolume;
    TextButton btn_play;
    TextButton btn_stop;
    TextButton btn_open;
    TextButton btn_inputSelector[2];//0:AudioFile, 1:InputStream
    bool isStreamingInput = false;
    
    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    
    //オーディオインターフェース,ノイズゲート設定の記録、呼び出し用
    ScopedPointer<ApplicationProperties> appProperties;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
