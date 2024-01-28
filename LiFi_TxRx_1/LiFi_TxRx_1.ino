/*+++++++++++++++++++++ LIFI TRANSCEIVER PROGRAM FOR PC ++++++++++++++++++++++//
// BY.....>>>>>> Ochieng Dickson <<<<<<<
//  ...........University of Nairobi.......................
//           BSc. Electrical and Electrical Engineering........... 
//  ........Final Year Project..............................................*/

int TxPin = 12; // Pin for data transmission 
int bit_delay = 50; // delay between bits in a packet
int byte_delay = 100; // delay between data packets
//int relaxtime = 200; // time allowed to reduce LED diminishing effect
int luxPin = A0; // Pin for data reception
int luxValue = 0; // Value read by arduino from luxPin 
unsigned int threshold = 750; // minumum luxValue for binary 1

void setup() {
  // put your setup code here, to run once
  pinMode(TxPin, OUTPUT); 
  pinMode(luxPin, INPUT);  
  Serial.begin(115200);
  unsigned int luxSum = 0;
  /*for(int i = 0; i < 20; i++){
    luxValue = analogRead(luxPin);
    luxSum += luxValue; 
    delay(250);
  } 
  threshold = luxSum/20 + 100;*/ 
} 

// +++++++++++++++++++++ DATA TRANSMISSION FUNCTION +++++++++++++++++++++ //
void lifiTransmitter(){
  while(Serial.available() == 0){
   //Checks if there is some text on the serial monitor     
  }
  // Grab the text on serial monitor and convert to binary
  String text = Serial.readString(); 
  //String text = "HELLO";
  int len = text.length()-1; // length = no. of bytes in text
  int num;
  int Tx_array[len][7];
  for(int i = 0; i < len; i++){
    num = int(text[i]); // converts each character in text to ascii code  
    for(int j = 6; j >= 0; j--){
      Tx_array[i][j] = num % 2; // gets binary equivalent of the ascii code
      num = num / 2;
      //Serial.print(Tx_array[i][j]);
    }
    //Serial.println("");
  }
  // The binary data in Tx_array send to TxPin
  for(int i = 0; i <= len; i++){
    digitalWrite(TxPin, HIGH); // start of a byte
    delay(bit_delay);
    for(int j = 0; j <= 6; j++){
      if(Tx_array[i][j] == 1){
        digitalWrite(TxPin, HIGH);
      }  
      else{
        digitalWrite(TxPin, LOW);        
      }    
      delay(bit_delay); // delay between the bits
    }
    delay(byte_delay); // delay between bytes
  }
  Serial.println("Data Sent Successfully"); // End of transmission
}

// +++++++++++++++++ DATA RECEIVER FUNCTION ++++++++++++++++++ //
String lifiReceiver(){
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

void loop() {
  // put your main code here, to run repeatedly:
  // Check for incoming data
  luxValue = analogRead(luxPin);
  if (luxValue > threshold) {
    Serial.println("Receiving Data...");
    String msg = lifiReceiver(); // process and store the data in msg variable
    unsigned int len = msg.length() - 2; // length of data bytes is 2 the less stop bytes
    msg.remove(len, 2); // removes the stop byte(s) from the data
    Serial.println("Received Data:");
    Serial.println(msg);
    }
  // transmit when there is data in the serial monitor and luxValue < threshold
  else if(luxValue < threshold and Serial.available() > 0){
    Serial.println("Sending Data...");
    lifiTransmitter();
  }  
  //else{lifiTransmitter(); }
}