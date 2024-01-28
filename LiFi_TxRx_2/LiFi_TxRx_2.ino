// +++++++++++++ LIFI TRANSCEIVER PROGRAM FOR KEYPAD-LCD CONFIGURATION ++++++++++++++ //
#include "Wire.h" // Library for I2C Communication
#include "LiquidCrystal_I2C.h" // Library for  I2C LCD
#include "Keypad.h" // Library for Keypad

#define default_delay 20
int TxPin = 12; // Pin for data transmission
int bit_delay = 50; // delay between bits in a packet in ms
int byte_delay = 100; // delay between data packets in ms
int luxPin = A0; // Pin for data reception
int luxValue = 0; // Value read by arduino from luxPin
int threshold = 750; // minumum luxValue for binary 1
// keypad variables
const byte ROWS = 4; // no of keypad rows
const byte COLS = 4; //no of keypad columns
int x = 0; // Holds the LCD x position
int y = 0; // Holds the LCD y position
int minValue = 0; // Lower character location for T9 text entry
int maxValue = 0; // Max character location for T9 text entry
int keyPressCount = 100; // Number of loops check of the key
String txt = ""; // Holds the created message
String alpha = "!@_$%?1 ABC2 DEF3 GHI4 JKL5 MNO6 PQRS7 TUV8 WXYZ9 *  0#"; // Characters for T9 texting

char keys[ROWS][COLS] = { // Character matrix for the keypad
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; // pin assignments for keypad rows
byte colPins[COLS] = {5, 4, 3, 2}; // pin assignments for keypad columns

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); // keypad object
LiquidCrystal_I2C lcd =  LiquidCrystal_I2C(0X27, 16, 2); // lcd object

// +++++++++++++ LI-FI RECEIVER FUNCTION ++++++++++++++++++//
String lifiReceiver() {
  String text = ""; // variable to store received messages
  bool Rx_array[100][8]; // array to store received binary data
  // Receive the data packets
  int k = 0; // counter to keep track of stop bits
  for (int i = 0; k < 16; i++) {
    int count_ones = 0; // to count no. of '1's in a byte
    int num = 1; // variable for the decimal number
    for (int j = 0; j < 8; j++ ) {
      luxValue = analogRead(luxPin);
      if (luxValue > threshold) {
        count_ones += 1;
        k = 0;
        Rx_array[i][j] = 1; // Store '1' if luxValue>Threshold
        if (j > 0 ) {
          // covert the binary number to decimal, dicarding the MSB
          num += pow(2, (7 - j)) * int(Rx_array[i][j]);
        }
      }
      else {
        Rx_array[i][j] = 0;
        // increments k only when there is no '1' in a byte
        if (count_ones == 0) {
          k += 1;
        }
        else {
          k = 0;
        }
      }
      delay(bit_delay); // delay between bits
    }
    count_ones = 0;
    delay(byte_delay); // delay between bytes
    if(((num >= 47) and (num <= 56)) or ((num >= 96) and (num <= 121))){
      num += 1;  // ascii codes for numbers and lower case lettes
    }
    text += char(num);// covert the decimal number to a char and add to text
  }
  return text;
}

void setup() {
  lcd.begin();
  lcd.backlight();
  pinMode(TxPin, OUTPUT);
  pinMode(luxPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  // Check for incoming data
  luxValue = analogRead(luxPin);
  if (luxValue > threshold) {
    //Serial.println("Receiving Data...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receiving Data..");
    String msg = lifiReceiver(); // process and store the data in msg variable
    unsigned int len = msg.length() - 2; // removes the stop byte(s) from the data
    msg.remove(len, 3);
    //Serial.println("Received Data:");
    //Serial.println(msg);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Received Data:");
    lcd.setCursor(0, 1);
    lcd.print(msg);
  }
  else if(luxValue < threshold and txt == ""){
    processkey(); // tranmitter function implemented in the processkey function
  }
}

// ++++++++++++++++ Parse Key Function +++++++++++++++++//
void parseKey(int minValue, int maxValue, char keyPress) {
  int pos = minValue;
  char key = keyPress;
  if (keyPress == '*') { // backspace
    if ( (x > 0) || (y > 0) ) { // prevent backspace when no character yet
      x = x - 1; // go back to previous character position
      lcd.setCursor(x, y); // set the new lcd position
      lcd.print('_'); // _ to show editing
      txt.remove(txt.length() - 1); // remove the last character from the string
    }
  }
  else {
    for (int i = 0; i < keyPressCount; i++) {
      if (key == keyPress) { // make sure that a key is pressed
        lcd.setCursor(x, y); // set the lcd position
        lcd.print(alpha[pos]); // print the character according to the character position
        pos++; // increment character position
        if (pos > maxValue) { // if the character counter reached the max value
          pos = minValue; // reset to min value
          i = 0; // reset the loop counter
        }
      }
      key = keypad.getKey(); // get pressed key
      delay(default_delay);
    }
    x++; // increment the x position
    txt += alpha[pos - 1]; // add the character to the variable msg
    if (x > 15) { // if the lcd reaches the rightmost position
      y = 1; // then wrap to the next line
      x = 0; // first character to the left
    }
  }
}

// +++++++++++++ Text Entry Function ++++++++++++++//
String enterText() {
  char key;
  x = 0; // init the x position to zero
  y = 0; // init the y position to zero
  txt = ""; // clear the msg variable

  do {
    key = keypad.getKey();
    // if a key is pressed
    if (key == '1') {
      parseKey(0, 7, key); // compare it to the alpha string array
    }
    else if (key == '2') {
      parseKey(8, 12, key);
    }
    else if (key == '3') {
      parseKey(13, 17, key);
    }
    else if (key == '4') {
      parseKey(18, 22, key);
    }
    else if (key == '5') {
      parseKey(23, 27, key);
    }
    else if (key == '6') {
      parseKey(28, 32, key);
    }
    else if (key == '7') {
      parseKey(33, 38, key);
    }
    else if (key == '8') {
      parseKey(39, 43, key);
    }
    else if (key == '9') {
      parseKey(44, 49, key);
    }
    else if (key == '0') {
      parseKey(52, 54, key);
    }
    else if (key == '*') {
      parseKey(50, 51, key);
    }
    else if (key == '#') {
      // do nothing here
    }
  }
  while (key != '#'); // exit the loop when # is pressed
  return txt;
}

// +++++++++++++ LI-FI TRANSMISSION FUNCTION ++++++++++++++++++//
void lifiTransmitter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER TEXT:");
  delay(1250);
  lcd.clear();
  lcd.setCursor(0, 1);
  String text = enterText(); // Get the text data from the user
  // Convert the text to binary
  int len = text.length() - 1; // length = no. of bytes in text
  int num;
  int Tx_array[len][7];
  for (int i = 0; i <= len; i++) {
    num = int(text[i]); // converts each character in to ascii code
    for (int j = 6; j >= 0; j--) {
      Tx_array[i][j] = num % 2; // gets binary equivalent of the ascii code
      num = num / 2;
    }
  }
  // SEND DATA. The binary data in Tx_array send to TxPin
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LIFI DT");
  lcd.setCursor(0, 1);
  lcd.print("Sending Data... ");
  for (int i = 0; i <= len; i++) {
    digitalWrite(TxPin, HIGH); // start of a byte
    delay(bit_delay);
    for (int j = 0; j <= 6; j++) {
      if (Tx_array[i][j] == 1) {
        digitalWrite(TxPin, HIGH);
      }
      else {
        digitalWrite(TxPin, LOW);
      }
      delay(bit_delay); // delay between the bits
    }
    delay(byte_delay); // delay between bytes
  }
  digitalWrite(TxPin, LOW); // Turn off the led at end of Tx
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DaTa Sent");
  lcd.setCursor(0, 1);
  lcd.print("Successfully");
  delay(1250);
}

// +++++++++++++++++++ KEYPAD FUNCTION ++++++++++++++++++++//
void processkey() {
  char key = keypad.getKey();
  if (isAlpha(key)) { // check if key pressed is a letter
    if (key == 'A') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WELCOME: LIFI DT");
      lcd.setCursor(0, 1);
      lcd.print("D -> TX");
    }
    else if (key == 'B') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("NO FUNCTION YET!");
    }
    }
  }
  else {
    // do nothing
    else if (key == 'C') {
      lcd.clear();
      lcd.print("NO FUNCTION YET!");
    }
    else if (key == 'D') {
      lifiTransmitter();
      lcd.clear();
  }
}