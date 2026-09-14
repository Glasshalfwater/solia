#include "/lib/TFT_eSPI/TFT_eSPI.h"
#include "lib\TFT_eSPI\User_Setup_Select.h" 

const int BTN_TR =  1;
const int BTN_TL = 2;
const int BTN_BL = 39;
const int BTN_BR = 3;

unsigned long last_button_press = 0;

const int TMP_SENSOR = 4;
 
const int BUZZER_PIN = 41;

TFT_eSPI tft = TFT_eSPI();

struct Pet 
{
  // Range from 0 to 100 with 0 being the lowest and 100 being the best
  int hunger;
  int happiness;
  int energy;
  unsigned long age;
};

Pet fox;

unsigned long last_update = 0;
unsigned long pet_starving = 0;
unsigned long pet_depressed = 0;
unsigned long pet_exhausted = 0;


enum Screen
{
  SCREEN_MAIN,
  SCREEN_FEED,
  SCREEN_PLAY,
  SCREEN_SLEEP,

};


Screen current_screen = SCREEN_MAIN;
Screen screen_array[] = {SCREEN_MAIN, SCREEN_FEED, SCREEN_PLAY, SCREEN_SLEEP};
int screen_index = 0;
// Example: happy face (all stats above 50)
const unsigned char PROGMEM petHappy[] = {
  0b00000000, 0b00000000,
  0b00011111, 0b11111000,
  0b00100000, 0b00000100,
  0b01000000, 0b00000010,
  0b01001100, 0b00110010,
  0b01001100, 0b00110010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000100, 0b00100010,
  0b01000011, 0b11000010,
  0b01000000, 0b00000010,
  0b00100000, 0b00000100,
  0b00011111, 0b11111000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

// Example: sad face (any stat below 30)
const unsigned char PROGMEM petSad[] = {
  0b00000000, 0b00000000,
  0b00011111, 0b11111000,
  0b00100000, 0b00000100,
  0b01000000, 0b00000010,
  0b01001100, 0b00110010,
  0b01001100, 0b00110010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000011, 0b11000010,
  0b01000100, 0b00100010,
  0b00100000, 0b00000100,
  0b00011111, 0b11111000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

// Example: neutral face (everything else)
const unsigned char PROGMEM petNeutral[] = {
  0b00000000, 0b00000000,
  0b00011111, 0b11111000,
  0b00100000, 0b00000100,
  0b01000000, 0b00000010,
  0b01001100, 0b00110010,
  0b01001100, 0b00110010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000111, 0b11100010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b00100000, 0b00000100,
  0b00011111, 0b11111000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

// Example: sleeping face (closed eyes)
const unsigned char PROGMEM petSleep[] = {
  0b00000000, 0b00000000,
  0b00011111, 0b11111000,
  0b00100000, 0b00000100,
  0b01000000, 0b00000010,
  0b01001111, 0b01110010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b01000011, 0b11000010,
  0b01000000, 0b00000010,
  0b01000000, 0b00000010,
  0b00100000, 0b00000100,
  0b00011111, 0b11111000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000
};

void setup() 
{
  // IO
  pinMode (BTN_TR, INPUT_PULLUP);
  pinMode (BTN_BL, INPUT_PULLUP);
  pinMode (BTN_BR, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  // TFT Display
  pinMode(TFT_CS, OUTPUT);
  SPI.begin(TFT_SCLK, TFT_MOSI, TFT_CS);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  fox.hunger = 80;
  fox.happiness = 80;
  fox.energy = 80;
  fox.age = 0;

  tft.setTextSize(1);
  tft.fillScreen(TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setCursor(0, 20);
  tft.print("Nota OS loading...");
  delay(2000);
  tft.print("Nota OS loading...");
  delay(2000);
  tft.print("Nota OS loading...");
  delay(2000);



}

void loop() 
{ 
  check_button_io();
  update_pet(fox); 
  render();
  delay(150);   
}

void update_pet(Pet pet)
{
  if ((millis() - last_update) > 60000) // Once per minute
  {
    pet.hunger--;
    pet.happiness--;
    pet.energy--;
    pet.age++;

    // Capping values out, if stay at zero for enough time, pet dies?
    if (pet.hunger < 0)
    {
      pet.hunger = 0;
      pet_starving = millis();
    }

    if (pet.happiness < 0) 
    {
      pet.happiness = 0;
      pet_depressed = millis();
    }

    if (pet.energy < 0)
    {
      pet.energy = 0;
      pet_exhausted = millis();
    }  
  }
}

void check_button_io() 
{
  if (millis() - last_button_press < 200) return;

  // "Interaction" button
  if (digitalRead(BTN_TR) == LOW) 
  {
    switch (current_screen) 
    {
      case(SCREEN_FEED):
        fox.hunger += 15;
        if (fox.hunger > 100) fox.hunger = 100;
        break;  

      case(SCREEN_PLAY):
        fox.happiness += 10;
        if (fox.happiness > 100) fox.happiness = 100;
        break;  

      case(SCREEN_SLEEP):
        fox.energy += 30;
        if (fox.energy > 100) fox.energy = 100;
        break;
      
      case(SCREEN_MAIN):
        break;
    }
  }

  // NOTE: still requires updating based on what features they want (still need to ask), differs from tamagotchi guided tutorial because I added an extra button for switching modes
  else if (digitalRead(BTN_TL) == LOW) 
  {
    current_screen = SCREEN_PLAY;
    tone(BUZZER_PIN, 1200, 50);
    last_button_press = millis();
  }

  else if (digitalRead(BTN_BL) == LOW) 
  {
    current_screen = SCREEN_SLEEP;
    tone(BUZZER_PIN, 800, 50);
    last_button_press = millis();
  }
  else if (digitalRead(BTN_BR) == LOW)
  {
    screen_index++;
    if (screen_index >= sizeof(screen_array))
      screen_index = 0;

    switch(screen_index)
    {
      case 0:
      current_screen = screen_array[screen_index];
      tone(BUZZER_PIN, 600, 50);
      last_button_press = millis();
      break;

      case 1:
      current_screen = screen_array[screen_index];
      tone(BUZZER_PIN, 1000, 50);
      last_button_press = millis();
      break;

      case 2:
      current_screen = screen_array[screen_index];
      tone(BUZZER_PIN, 1200, 50);
      last_button_press = millis();
      break;

      case 3:
      current_screen = screen_array[screen_index];
      tone(BUZZER_PIN, 800, 50);
      last_button_press = millis();
      break;

      default:
      current_screen = screen_array[screen_index];
      tone(BUZZER_PIN, 600, 50);
      last_button_press = millis();
      break;
    }
  } 
}


// Credit: basic tone ranges Gemini
void pet_chirp() { 
  for (int frequency = 1500; frequency < 3000; frequency += 150) {
    tone(BUZZER_PIN, frequency, 15); 
    delay(10);        
  }
  
  delay(50);  
   
  for (int frequency = 2000; frequency < 3500; frequency += 150) {
    tone(BUZZER_PIN, frequency, 15); 
    delay(10); 
  }
}

// NOTE: DO NOT USE, NEEDS TO BE EDITED TO FIT ESP32 S3
void render() {
  tft.clearDisplay();

  // Choose the right sprite based on pet stats
  const unsigned char* sprite;
  if (fox.hunger < 30 || fox.happiness < 30 || fox.energy < 30) {
    sprite = petSad;
  } else if (fox.hunger > 50 && fox.happiness > 50 && fox.energy > 50) {
    sprite = petHappy;
  } else {
    sprite = petNeutral;
  }

  // Draw the pet sprite (centered horizontally, near the top)
  tft.drawBitmap(56, 2, sprite, 16, 16, SSD1306_WHITE);

  // Draw stat bars below the pet
  tft.setTextSize(1);

  tft.setCursor(0, 24);
  tft.print("HUN ");
  drawBar(24, 24, fox.hunger);

  tft.setCursor(0, 34);
  tft.print("HAP ");
  drawBar(24, 34, fox.happiness);

  tft.setCursor(0, 44);
  tft.print("ENG ");
  drawBar(24, 44, fox.energy);

  // Button labels at the bottom
  tft.setCursor(0, 56);
  tft.println("[Feed] [Play] [Sleep]");

  tft.display();
}

// Draws a stat bar: empty rectangle with a filled portion based on value (0 to 100)
void drawBar(int x, int y, int value) {
  int barWidth = 100;
  int barHeight = 6;
  int fillWidth = map(value, 0, 100, 0, barWidth);

  display.drawRect(x, y, barWidth, barHeight, SSD1306_WHITE);       // outline
  display.fillRect(x, y, fillWidth, barHeight, SSD1306_WHITE);      // filled portion
}


