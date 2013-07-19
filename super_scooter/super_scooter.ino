#include <LiquidCrystal.h>
#include <LCDKeypad.h>

#define  STATE_GAME       1
#define  STATE_GAME_LOST  2
#define  STATE_GAME_WON   3
#define  STATE_MENU       0

#define  KEY_UP     1
#define  KEY_DOWN   2
#define  KEY_RIGHT  0
#define  KEY_LEFT   3
#define  KEY_SELECT 5

int state = STATE_MENU;
int lcd_width = 16;

// Detect physical key press. Very neat, this board
// maps the 5 physical keys to 1 analog input! The
// mapping below lets us determine which key was pressed
int adc_key_val[5] ={
  50, 200, 400, 600, 800 };
int adc_key_in;
int NUM_KEYS = 5;
int key =     -1;
int oldkey =  -1; 

// the default timing interval of the game
int   interval =       300;
long  previousMillis = 0;

// define the player and starting position
String player_ship =    "[=>";
int player_position_y = 0;
int player_position_x = 1;

int number_of_enemies = 20;
// This defines the enemies if you want to build your level by hand.
// Below, these values will be overwritten by random values. But I 
// wanted to leave this in so you can see how to define a level. 
// Simply comment out the code below that fills in random the values.
int enemies_x[20] = {
  16, 22, 27, 34, 38, 42, 47, 51, 54, 58, 65, 70, 74, 78, 83, 87, 91, 95, 99, 104};
int enemies_y[20] = {
  1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0};

// the current difficulty setting of the game
int difficulty = 3;

LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

void setup()
{
  // initialize the serial port
  Serial.begin(9600);

  // initialize the lcd screen
  lcd.begin(16,2);

  // clear the lcd screen and show the game startup text
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Jeff Cardillo's");
  lcd.setCursor(0,1);
  lcd.print("Super Scooter!!!");

  // seed the random number generator so we get different values every new run
  // analog pin 1 should be a "fairly" random value because it is "floating"
  randomSeed(analogRead(1));

  // randomly generate the placement of the enemies so each game is different.
  // first, make sure we start with at least the width of the lcd so no enemies
  // are on the screen when we first launch. Also need to be sure we have at 
  // least the ship length between the enemies to be sure the game is solvable!
  // COMMENT OUT THE BLOCK BELOW TO USE DEININED VALUES ABOVE
  int accumulation = lcd_width;
  for(int i=0; i<number_of_enemies; i++) {
    // randomly pick a number between the ship's length and 8
    accumulation += random((player_ship.length()+1), 8);
    enemies_x[i] = accumulation;
    enemies_y[i] = random(0,2);
  }

  // show the startup text for 2 seconds
  delay(2000);
}

void loop()
{
  unsigned long currentMillis = millis();  

  // only do this after the defined interval has passed since the last time
  if(currentMillis - previousMillis > interval) {

    // remember when we do this block
    previousMillis = currentMillis;
      
    if(state == STATE_MENU) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Right to start"); 
        showDifficulty();
    }
    else if(state == STATE_GAME) {  
        // clear the screen and draw the player
        lcd.clear();
        lcd.setCursor(player_position_x, player_position_y);
        lcd.print(player_ship); 
  
        // for each enemy
        for(int i=0; i<number_of_enemies; i++) {
          // move them to the left 1 space
          enemies_x[i] = enemies_x[i] - 1;
  
          // and draw them where they should appear
          if(enemies_x[i] >= 0 && enemies_x[i] <= lcd_width) { 
            lcd.setCursor(enemies_x[i], enemies_y[i]);
            lcd.print("*");
          }
  
          // check to see if any enemy touches the player, if so, GAME OVER!!!
          if((enemies_x[i] <= player_position_x+player_ship.length()) 
            && (enemies_x[i] >= player_position_x)
            && (enemies_y[i] == player_position_y))
            state = STATE_GAME_LOST;
        }
  
        // check to see if all enemies have been passed, if so, GAME WON!!!
        if(enemies_x[(number_of_enemies-1)] < 0)
          state = STATE_GAME_WON;
    } 
    else if(state == STATE_GAME_LOST) {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("GAME OVER!!!");
    } 
    else if(state == STATE_GAME_WON) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("You WIN!!!");
    }
  }
  
  // check for key presses on every cycle
  // read the value from the sensor 
  adc_key_in = analogRead(0); 
  // convert into key press  
  key = get_key(adc_key_in);  
  // if keypress is detected
  if (key != oldkey)   
  {
    // we want to "debounce" the switch so we don't get multiple presses
    delay(50);
    // read the value from the sensor 
    adc_key_in = analogRead(0);
 
    // convert into key press 
    key = get_key(adc_key_in);    
    if (key != oldkey) {   
      oldkey = key;
      if (key >= 0) {
        // we have a valid key
        if(key == KEY_DOWN) {
          // this key has different functions based on the state of our game!
          if(state == STATE_GAME) {
            player_position_y = 1;
          } 
          else if(state == STATE_MENU) {
            if(difficulty < 4) {
              difficulty = difficulty + 1;
              interval = difficulty * 100;
            }
          }             
        }
        else if(key == KEY_UP) {
          // this key has different functions based on the state of our game!
          if(state == STATE_GAME) {
            player_position_y = 0;
          } 
          else if (state == STATE_MENU) {
            if(difficulty >= 2) {
              difficulty = difficulty - 1;
              interval = difficulty * 100;
            }
          }
        }
        else if(key == KEY_RIGHT) { 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Starting game...");
          delay(1000);
          state = STATE_GAME;
        }
      }
    }
  }
} 

// convenience method for displaying the difficulty
void showDifficulty() {
  lcd.setCursor(0, 1);
  if(difficulty == 1)
    lcd.print("D: Hardest!");
  else if(difficulty == 2)
    lcd.print("D: Harder!");
  else if(difficulty == 3)
    lcd.print("D: Normal!"); 
  else if(difficulty == 4)
    lcd.print("D: Easy cheesy!");   
}

// Convert ADC value to key number
int get_key(unsigned int input) {
  int k;
  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return k;
    }
  }   
  if (k >= NUM_KEYS)k = -1;  // No valid key pressed
  return k;
}


