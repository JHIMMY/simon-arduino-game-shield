/**********************************************************************************
 * TITLE: Simon Says Game
 * DESCRIPTION: This is the classic memory game implemented in Arduino!
 * YOUTUBE CHANNEL: https://www.youtube.com/@jadsa
 * Jhimmy Astoraque D.
 * *******************************************************************************/

// libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "pitches.h" // make sure you have this file in the same folder of this skecth

// leds
const uint8_t yellowLed = 7, blueLed = 6, greenLed = 4, redLed = 5;
const uint8_t ledPins[] = {yellowLed, blueLed, greenLed, redLed};

// buttons
const uint8_t yellowBtn = 8, blueBtn = 9, greenBtn = 11, redBtn = 10;
const uint8_t buttonPins[] = {yellowBtn, blueBtn, greenBtn, redBtn};
const uint16_t buttonTones[] = {NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};

// oled
const uint8_t width = 128;
const uint8_t height = 32;
const uint8_t oledAddress = 0X3C;
const int8_t oledReset = -1;
Adafruit_SSD1306 oled(width, height, &Wire, oledReset);

// buzzer
const uint8_t buzzerPin = 12;

// game state and settings
const uint8_t maxGameLength = 100; // Guinness's world record 84
uint8_t gameSequence[maxGameLength] = {0};
uint8_t gameIndex = 0; // can be used for referencing score

// non volatile score
const uint8_t highScoreAddress = 0; // score won't be more than 255



void setup()
{
    Serial.begin(115200);

    // config pins
    pinMode(buzzerPin, OUTPUT);
    for (uint8_t i = 0; i < sizeof(ledPins); i++)
    {
        pinMode(ledPins[i], OUTPUT);
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    // oled config
    Wire.begin();
    oled.begin(SSD1306_SWITCHCAPVCC, oledAddress);
    oled.clearDisplay();
    oled.setTextColor(WHITE);
       

    // random generator
    randomSeed(analogRead(A0));

    // uncomment if you want to reset high score
    resetHighScore();

    // Initial msgs
    welcomeMessage(); 
    showCurrentRecord();
    delay(2000);
    showScore(gameIndex);

}

void loop()
{    
    gameSequence[gameIndex++] = random(0, 4); // Add a random color to the sequence

    if (gameIndex >= maxGameLength)
    {
        gameIndex = maxGameLength - 1; // keep it in bounds
    }

    autoPlaySequence();
    if (!checkUserValidSequence())
    {
        gameOver();
    }

    delay(300);

    showScore(gameIndex);
    if (gameIndex > 0)
    {
        playLevelUpSound();
        delay(300);
    }
    
}


/******* Helper Functions *******/

void welcomeMessage()
{
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    oled.print("Welcome To");
    oled.setCursor(0, 16);
    oled.print("Simon Game");
    oled.display();
    delay(1500);
    oled.clearDisplay();
    oled.display();
}

void autoPlaySequence()
{
    for (uint8_t i = 0; i < gameIndex; i++)
    {
        uint8_t currentLed = gameSequence[i];
        illuminateLedAndPlaySound(currentLed);
        delay(70);
    }
}

void illuminateLedAndPlaySound(uint8_t ledIndex)
{
    digitalWrite(ledPins[ledIndex], HIGH);
    tone(buzzerPin, buttonTones[ledIndex]);
    delay(300);
    digitalWrite(ledPins[ledIndex], LOW);
    noTone(buzzerPin);
}

bool checkUserValidSequence()
{
    for (uint8_t i = 0; i < gameIndex; i++)
    {
        uint8_t expectedButton = gameSequence[i];
        uint8_t actualButton = readGameButtons();
        illuminateLedAndPlaySound(actualButton);
        if (expectedButton != actualButton)
            return false;
    }

    return true;
}

uint8_t readGameButtons()
{
    while (true)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t buttonPin = buttonPins[i];
            if (digitalRead(buttonPin) == LOW)
                return i;
        }
        delay(5);
    }
}


void gameOver()
{
    uint8_t gameScore = gameIndex - 1;
    
    Serial.println("Game over!");
    Serial.println("obtained score: " + (String)gameScore + "\n");
    gameIndex = 0;
    // check High Score
    if (gameScore > readHighScore())
        newRecordAchievement(gameScore);
    else
        showScore(gameScore);
    delay(350);

    playGameOverSound();
    
    for (uint8_t i = 0; i < 10; i++)
    {
        for (int pitch = -10; pitch <= 10; pitch++)
        {
            tone(buzzerPin, NOTE_C5 + pitch);
            delay(5);
        }
    }
    noTone(buzzerPin);
    delay(500);

    oled.clearDisplay();
    oled.display();
}

void playLevelUpSound()
{
    tone(buzzerPin, NOTE_E4);
    delay(150);
    tone(buzzerPin, NOTE_G4);
    delay(150);
    tone(buzzerPin, NOTE_E5);
    delay(150);
    tone(buzzerPin, NOTE_C5);
    delay(150);
    tone(buzzerPin, NOTE_D5);
    delay(150);
    tone(buzzerPin, NOTE_G5);
    delay(150);
    noTone(buzzerPin);
}

void playGameOverSound()
{
    tone(buzzerPin, NOTE_DS5);
    delay(300);
    tone(buzzerPin, NOTE_D5);
    delay(300);
    tone(buzzerPin, NOTE_CS5);
    delay(300);
}

void newRecordAchievement(byte newScore)
{
    writeHighScore(newScore);
    showNewRecord();
}

void showScore(byte score)
{
    oled.clearDisplay();
    oled.setCursor(0, 8);               
    oled.setTextSize(2);
    oled.print(" Score: " + (String)score);             
    oled.display();
}

void showRecord(String msg)
{
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print(msg);
    oled.setTextSize(2);
    oled.setCursor(0, 12);
    String s = (String)readHighScore();
    oled.print("    " + s);
    oled.display();
}

void showCurrentRecord()
{
    showRecord("   Current Record:");
}

void showNewRecord()
{
    showRecord("     New Record!");
}

uint8_t readHighScore()
{
    return EEPROM.read(highScoreAddress);
}

void writeHighScore(byte score)
{
    EEPROM.write(highScoreAddress, score);
}

void resetHighScore()
{
    EEPROM.write(highScoreAddress, 0);
}
