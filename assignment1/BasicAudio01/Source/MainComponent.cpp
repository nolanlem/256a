// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University

// Description: additive synthesis using individual freq/gain sliders 

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"

class MainContentComponent :
    public AudioAppComponent, // extends other classes here
    private Slider::Listener, // want to be able to do everything on this 1 c++ file
    private ToggleButton::Listener
{
public:
    // constructor here
    MainContentComponent() : gain {0.0,0.0}, onOff {0,0}, samplingRate(0.0), trig {0,0}
    {
        for (int n = 0; n < numsines; n++){
            // configuring frequency slider and adding it to the main window
            addAndMakeVisible (frequencySlider[n]);
            frequencySlider[n].setRange (50.0, 5000.82);
            frequencySlider[n].setSkewFactorFromMidPoint (500.0);
            frequencySlider[n].setValue(485); // will also set the default frequency of the sine osc
            frequencySlider[n].addListener (this); // this is why it's important to extend the above Slider::Listener class
        
            // configuring frequency label box and adding it to the main window
            addAndMakeVisible(frequencyLabel[n]);
            frequencyLabel[n].setText ("Frequency", dontSendNotification);
            frequencyLabel[n].attachToComponent (&frequencySlider[n], true);
        
            
            // configuring gain slider and adding it to the main window
            addAndMakeVisible (gainSlider[n]);
            gainSlider[n].setRange (0.0, 1.0);
            gainSlider[n].setValue(0.0); // will alsi set the default gain of the sine osc
            gainSlider[n].addListener (this);
        
        
            // configuring gain label and adding it to the main window
            addAndMakeVisible(gainLabel[n]);
            gainLabel[n].setText ("Gain", dontSendNotification);
            gainLabel[n].attachToComponent (&gainSlider[n], true);
        
        
            // configuring on/off button and adding it to the main window
            addAndMakeVisible(onOffButton[n]);
            onOffButton[n].addListener(this);
            
            
            //adding trigger button to trigger frequency motion (brownian mov't?) event
            addAndMakeVisible(trigButton[n]); 
            trigButton[n].addListener(this);
            
            addAndMakeVisible(brownianLabel[n]); 
            brownianLabel[n].setText("brownianish motion", dontSendNotification); 
            brownianLabel[n].attachToComponent(&trigButton[n], true); 
        
        
            // configuring on/off label and adding it to the main window
            addAndMakeVisible(onOffLabel[n]);
            onOffLabel[n].setText ("On/Off", dontSendNotification);
            onOffLabel[n].attachToComponent (&onOffButton[n], true);
        }
        
        gainSlider[0].setValue(0.5); 
         
        
        setSize (600, 400); // set default size of the window here
        nChans = 2; //one channel
        setAudioChannels (0, nChans); // no inputs, one output
    }
    
    // destructor 
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    // this method has to be declared because it's a virtual method, bc we extended audioappcomponent class
    // this is called with user changes size of window
    void resized() override
    {
        // placing the UI elements in the main window
        // getWidth has to be used in case the window is resized by the user
        const int sliderLeft = 80;
        for (int i = 0; i < numsines; i++){
            frequencySlider[i].setBounds (sliderLeft, 10 +110*i, getWidth() - sliderLeft - 20, 20);
            gainSlider[i].setBounds (sliderLeft, 40 + 110*i, getWidth() - sliderLeft - 20, 20);
            onOffButton[i].setBounds (sliderLeft, 70 + 110*i, getWidth() - sliderLeft - 20, 20);
            trigButton[i].setBounds (sliderLeft + 300, 70 + 110*i, getWidth(), 20);
        }
    }
    
    void reinitializeFreq(){ 
        for (int n = 0; n<numsines; n++){
            ax[n] = frequencySlider[n].getValue() ;
        }
        
    }
    
    
    // has to be declared too because we're extending slider:listener or else compiler will complain 
    void sliderValueChanged (Slider* slider) override
    {
        if (samplingRate > 0.0){
            for (int n = 0; n < numsines; n++){ 
                if (slider == &frequencySlider[n]){
                    sine[n].setFrequency(frequencySlider[n].getValue());
                }
                else if (slider == &gainSlider[n]){
                    gain[n] = gainSlider[n].getValue();
                }
            }
        }
    }
    
    // callback function for button 
    void buttonClicked (Button* button) override
    {
        // turns audio on or off
        for(int i = 0; i< numsines; i++){
            if(button == &onOffButton[i] && onOffButton[i].getToggleState()){
                onOff[i] = 1;
                std::cout << i; 
                std::cout << "\n"; 
            }
            // if button is trigbutton, and trigbutton pressed
            else if (button == &trigButton[i] && trigButton[i].getToggleState()){
                trig[i] = 1; 
                std::cout << i;
                reinitializeFreq(); 
                brownTrig = 1; 
            }
            // if button is trig button and it's currently ON (toggle Off); 
            else if (button == &trigButton[i] && trig[i] == 1){ 
                trig[i] = 0;
                brownTrig = 0; 
                //sine[i].setFrequency(500);
            }
            // if button is onOff button and it's 0
            else if (button == &onOffButton[i] && onOffButton[i].getToggleState() == 0){
                onOff[i] = 0;
            }
        }
        
        
    }
    

   
    
    // 
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = sampleRate;
        for (int i = 0; i<numsines; i++){
            sine[i].setSamplingRate(sampleRate); // configuring our sine to have this sampleRate
        }
    }
    
    // has to be declared
    void releaseResources() override
    {
    }
    
    // this is our AUDIO callback function 
    // getNextAudioBlock comes from having extended audioappcomponent, 
    // override adds any methods associated with getNextAudioBlock member class 
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // note: anything that happens before the sample level, these are running at control rate       
        
        // getting the audio output buffer to be filled
        // buffer is just array of samples 
        // args(channel number (0), 
        
        // the buffer is just an array (vector) 
        // if we were working with input audio (microphone), the buffer would already be filled but 
        // here it's empty 
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample); 
        
        // activate brownian motion if brownTrig flag is set
        if (brownTrig == 1){
            for (int n = 0; n < numsines; n++){
            // assign brownianish motion to freq param 
                float randf = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/4);      
                randf = randf - 2; 
                ax[n] = ax[n] + randf;
                sine[n].setFrequency(ax[n]); 
            }
        
        }
     
        // computing one block, this is at audio rate
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample){
            for(int i = 0; i<numsines; i++){               
                if(onOff[i] == 1){  // if individual button is on, then 
                    buffer[sample] = buffer[sample] + sine[i].tick() * gain[i];
                } 
                //else if (onOff[i] == 0) buffer[sample] = buffer[sample] + 0.0;
            }

        }
        
        

    }
    
    
private:
    // UI Elements
    int numsines = 5; 
    Slider frequencySlider [5];
    Slider gainSlider [5];
    ToggleButton onOffButton [5];
    ToggleButton trigButton [5]; 
    
    Label frequencyLabel [5]; 
    Label gainLabel [5];
    Label onOffLabel [5];
    Label brownianLabel [5]; 
    
    Sine sine [5]; // the sine wave oscillator
    
    // Global Variables
    float gain [5]; 
    int onOff [5]; // state variables
    int trig [5]; 
    float ax [5] = {500,500,500,500,500}; 
    int brownTrig; 
    
    int samplingRate, nChans;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
