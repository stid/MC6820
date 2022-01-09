#include <Arduino.h>


/*
                        W65C21S <--->  Arduino Uno
                 GND                         
                 |       +------\/------+      
                 +----  1| VSS      CA1 |40 ------ 13
                        2| PA0      CA2 |39 ------ A4
                        3| PA1    IRQAB |38 ------ A0
                        4| PA2    IRQBB |37 ------ NA
                        5| PA3      RS0 |36 ------ A1
                        6| PA4      RS1 |35 ------ A2
                        7| PA5     RESB |34 ------ 12
                        8| PA6       D0 |33 -------- 2
                        9| PA7       D1 |32 -------- 3
                       10| PB0       D2 |31 -------- 4
                       11| PB1       D3 |30 -------- 5
                       12| PB2       D4 |29 -------- 6
                       13| PB3       D5 |28 -------- 7
                       14| PB4       D6 |27 -------- 8
                       15| PB5       D7 |26 -------- 9
                       16| PB6     PHI2 |25 -------- 10
                       17| PB7      CS1 |24 -------- 5V
           5V -------- 18| CB1     CS2B |23 -------- A3
           5V -------- 19| CB2      CS0 |22 -------- 5V
           5V -------- 20| VDD      RWB |21 -------- 11 
                          +--------------+      
                                               
*/               
const int SERIAL_SPEED = 9600; // Arduino Serial Speed
const int CLOCK_DELAY = 5; // Arduino Serial Speed


#define PHI2 53
#define RESB 52
#define RWB 50
#define CA1 51
#define CA2 49
#define IRQAB 48
#define RS0 47
#define RS1 46
#define CS2B 45
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


const int DATA_PINS[]     = {22, 23, 24, 25, 26, 27, 28, 29}; // TO DATA BUS PIN 0-7 6502
int rw_state;             // Current R/W state (from 6502)
unsigned char bus_data;   // Data Bus value (from 6502)
unsigned char pre_bus_data;   // Previous Bus value (from 6502)
int pre_rw_state;             // Previous R/W state (from 6502)

int clock_state=0;             // Previous R/W state (from 6502)
long tick_count=0;             // Previous R/W state (from 6502)


// Set Arduino Bus conneced pins mode as IN or OUT
void setBusMode(int mode) {
  for (int i = 0; i < 8; ++i) {
    pinMode(DATA_PINS[i], mode);
  }
}

// Read RW_PIN state and set the busMode (aruduino related PINS) to OUTPUT or INPUT
void handleRWState() {
  int tmp_rw_state=digitalRead(RWB);

  if (rw_state != tmp_rw_state) {
    rw_state=tmp_rw_state;
    rw_state ? setBusMode(INPUT) : setBusMode(OUTPUT);
  }
}

// Read 6502 Data PINS and store the BYTE in our bus_data var
void readData() {
  bus_data = 0;
  for (int i = 0; i < 8; ++i)
  {
    bus_data = bus_data << 1;
    bus_data += (digitalRead(DATA_PINS[8-i-1]) == HIGH)?1:0;
  }
}

// Send a byte to the 6502 DATA BUS
void byteToDataBus(unsigned char data) {
  for (int i = 0; i < 6; i++) {
    digitalWrite(DATA_PINS[i], CHECK_BIT(data, i));
  }
}

void tick() {
  clock_state = !clock_state;
  tick_count+=1;
  digitalWrite(PHI2, clock_state);
  delay(CLOCK_DELAY);
  Serial.print("#");
  Serial.print(tick_count*CLOCK_DELAY, DEC);
  Serial.print(" CPH: ");
  Serial.println(clock_state, DEC);

}

void writeToCIA() {
  digitalWrite(RWB, LOW); // WRITE TO CIA
  handleRWState();
}

void readFromCIA() {
  digitalWrite(RWB, HIGH); // READ FROM CIA
  handleRWState();
}

// the setup function runs once when you press reset or power the board
void setup() {
  clock_state = 0;
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(CA1, OUTPUT);
  pinMode(CA2, OUTPUT);
  pinMode(PHI2, OUTPUT);
  pinMode(RESB, OUTPUT);
  pinMode(RWB, OUTPUT);
  pinMode(CS2B, OUTPUT);
  pinMode(RS0, OUTPUT);
  pinMode(RS1, OUTPUT);
  pinMode(IRQAB, INPUT_PULLUP);


  digitalWrite(RESB, LOW); // RESET GOES LOW!
  digitalWrite(CA1, HIGH); // INTERRUPT VIA CA1
  digitalWrite(CA2, HIGH); // INTERRUPT VIA CA2
  readFromCIA();
  
  // SELECT CONTROL REG
  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW); // Control Register A

  Serial.begin(SERIAL_SPEED);
  tick();
  tick();
  tick(); // ALIGN TO POSITIVE CLOCK


  digitalWrite(RESB, HIGH); // STOP RESET
  tick();
  tick();


  digitalWrite(CS2B, LOW); // SELECT
  tick();
  tick();


  Serial.println("----------------------------");
  Serial.println("65C21 Test");
  Serial.println("----------------------------");

  tick();
  tick();


  readFromCIA();
  tick();
  tick();
  Serial.print("REGA AFTER RESET: ");
  readData();
  Serial.println(bus_data, BIN);


  // WRITE REG A (active int)
  writeToCIA();
  byteToDataBus(0b00000001);
  tick();
  tick();


  readFromCIA();
  tick();
  tick();

  // RESULT
  Serial.print("IRQAB NO IRQ: ");
  Serial.println(digitalRead(IRQAB));

  Serial.print("REGA AFTER CIA SET: ");
  readData();
  Serial.println(bus_data, BIN);

  digitalWrite(CA1, LOW); // TRIGGER INT
  tick();
  tick();


  readData();
  Serial.print("REGA AFTER INTERRUPT TRIGGERED on CA1: ");
  Serial.println(bus_data, BIN);
  tick();
  tick();

    // RESULT
  Serial.print("IRQAB AFTER INPUT TRIGGERED: ");
  Serial.println(digitalRead(IRQAB));


  digitalWrite(CA1, HIGH); // UNTRIGGER INT
  tick();
  tick();


    // RESULT
  Serial.print("IRQAB AFTER CA1 DETRIG: ");
  Serial.println(digitalRead(IRQAB));

  readData();
  Serial.print("REGA AFTER CA1 DETRIG: ");
  Serial.println(bus_data, BIN);
  tick();
  tick();

  // WRITE REG A (PERIPHERIAL A SELECT)
  writeToCIA();
  byteToDataBus(0b00000101); // Bit 2 at 1
  tick();
  tick();
  digitalWrite(RS0, LOW); // Per A
  digitalWrite(RS1, LOW); // Per A
  readFromCIA();
  tick();
  tick();
  readData();
  tick();
  tick();
  Serial.print("PERIPHERIAL DATA: ");
  Serial.println(bus_data, BIN);


  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW); // Control Register A
  tick();
  tick();
  readData();
  Serial.print("REGA AFTER PER A READ: ");
  Serial.println(bus_data, BIN);
  tick();
  tick();
  Serial.print("IRQAB AFTER A READ: ");
  Serial.println(digitalRead(IRQAB));


  // TRIGGER CA1 AGAIN
  Serial.print("TRIGGER CA1 AGAIN");
  digitalWrite(CA1, LOW); // UNTRIGGER INT
  tick();
  tick();
  Serial.print("IRQAB AFTER TRIGGER ");
  Serial.println(digitalRead(IRQAB));



  Serial.print("DESELECT & CA1 HIGH");
  digitalWrite(CA1, HIGH); // UNTRIGGER INT
  digitalWrite(CS2B, HIGH); // DESELECT
  tick();
  tick();


  // TRIGGER CA1 AGAIN
  Serial.print("TRIGGER CA1 AGAIN AFTER DESELECT");
  digitalWrite(CA1, LOW); // UNTRIGGER INT
  tick();
  tick();
  Serial.print("IRQAB AFTER TRIGGER ");
  Serial.println(digitalRead(IRQAB));


  Serial.print("SELECT");
  digitalWrite(CS2B, LOW); // SELECT

  /*
  digitalWrite(CS2B, HIGH); // DESELECT
  tick();
  tick();
  digitalWrite(CS2B, LOW); // SELECT
  tick();
  tick();
  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW); // Control Register A
  tick();
  tick();

 // WRITE REG A (PERIPHERIAL A SELECT)
  writeToCIA();
  byteToDataBus(0b00001000); // Bit 3 at 1 so CA2 now trigger too
  tick();
  tick();

  readFromCIA();
  tick();
  tick();
  readData();
  Serial.print("REGA AFTER BIT3 CA2 SET: ");
  Serial.println(bus_data, BIN);





  digitalWrite(CA2, LOW); // TRIGGER CA2
  tick();
  tick();
  readData();
  Serial.println("REGA & IRQA AFTER CA2 triggered");
  Serial.println(bus_data, BIN);
  Serial.println(digitalRead(IRQAB));
  Serial.println("-------------------------------");


  */


  digitalWrite(CS2B, HIGH); // DESELECT
  tick();
  tick();


  // READY

}

// the loop function runs over and over again forever
void loop() {
                     // wait for a second
}