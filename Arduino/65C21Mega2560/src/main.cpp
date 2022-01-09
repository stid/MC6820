#include <Arduino.h>

/*
                        W65C21S <--->  Arduino Uno
                 GND
                 |       +------\/------+
                 +----  1| VSS      CA1 |40 ------ 49
                        2| PA0      CA2 |39 ------ 48
                        3| PA1    IRQAB |38 ------ 47
                        4| PA2    IRQBB |37 ------ NA
                        5| PA3      RS0 |36 ------ 46
                        6| PA4      RS1 |35 ------ 45
                        7| PA5     RESB |34 ------ 51
                        8| PA6       D0 |33 -------- 22
                        9| PA7       D1 |32 -------- 23
                       10| PB0       D2 |31 -------- 24
                       11| PB1       D3 |30 -------- 25
                       12| PB2       D4 |29 -------- 26
                       13| PB3       D5 |28 -------- 27
                       14| PB4       D6 |27 -------- 28
                       15| PB5       D7 |26 -------- 29
                       16| PB6     PHI2 |25 -------- 52
                       17| PB7      CS1 |24 -------- 5V
           5V -------- 18| CB1     CS2B |23 -------- 44
           5V -------- 19| CB2      CS0 |22 -------- 5V
           5V -------- 20| VDD      RWB |21 -------- 50
                          +--------------+

*/
const int SERIAL_SPEED = 19200; // Arduino Serial Speed
const int CLOCK_DELAY = 5;      // Arduino Serial Speed

#define PHI2 52
#define RESB 51
#define RWB 50
#define CA1 49
#define CA2 48
#define IRQAB 47
#define RS0 46
#define RS1 45
#define CS2B 44
const int DATA_PINS[] = {22, 23, 24, 25, 26, 27, 28, 29}; // TO DATA BUS PIN 0-7 6502

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
int rw_state;               // Current R/W state (from 6502)
unsigned char bus_data;     // Data Bus value (from 6502)
unsigned char pre_bus_data; // Previous Bus value (from 6502)
int pre_rw_state;           // Previous R/W state (from 6502)

int clock_state = 0; // Previous R/W state (from 6502)
long tick_count = 0; // Previous R/W state (from 6502)

// Set Arduino Bus conneced pins mode as IN or OUT
void setBusMode(int mode)
{
  for (int i = 0; i < 8; ++i)
  {
    pinMode(DATA_PINS[i], mode);
  }
}

// Read RW_PIN state and set the busMode (aruduino related PINS) to OUTPUT or INPUT
void handleRWState()
{
  int tmp_rw_state = digitalRead(RWB);

  if (rw_state != tmp_rw_state)
  {
    rw_state = tmp_rw_state;
    rw_state ? setBusMode(INPUT) : setBusMode(OUTPUT);
  }
}

// Read 6502 Data PINS and store the BYTE in our bus_data var
void readData()
{
  bus_data = 0;
  for (int i = 0; i < 8; ++i)
  {
    bus_data = bus_data << 1;
    bus_data += (digitalRead(DATA_PINS[8 - i - 1]) == HIGH) ? 1 : 0;
  }
}

// Send a byte to the 6502 DATA BUS
void byteToDataBus(unsigned char data)
{
  for (int i = 0; i < 6; i++)
  {
    digitalWrite(DATA_PINS[i], CHECK_BIT(data, i));
  }
}

void tick()
{
  clock_state = !clock_state;
  tick_count += 1;
  digitalWrite(PHI2, clock_state);
  delay(CLOCK_DELAY);
  Serial.print("#");
  Serial.print(tick_count * CLOCK_DELAY, DEC);
  Serial.print(" CPH: ");
  Serial.println(clock_state, DEC);
}

void fullCycle()
{
  tick();
  tick();
}

void writeToCIA()
{
  digitalWrite(RWB, LOW); // WRITE TO CIA
  handleRWState();
}

void readFromCIA()
{
  digitalWrite(RWB, HIGH); // READ FROM CIA
  handleRWState();
}

void DeselectSelect()
{
  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();
  digitalWrite(CS2B, LOW); // SELECT
  fullCycle();
  Serial.println("\nDESELECTED & SELECTED!\n");
}

void fetchPeripherialA()
{
  // Write to REGA
  digitalWrite(RS0, HIGH);
  digitalWrite(RS1, LOW);
  // WRITE REG A (PERIPHERIAL A SELECT)
  writeToCIA();
  byteToDataBus(0b00000101); // Bit 2 at 1
  fullCycle();

  readFromCIA();
  digitalWrite(RS0, LOW); // Per A
  digitalWrite(RS1, LOW); // Per A
  fullCycle();
  readData();
  Serial.print("PERIPHERIAL DATA: ");
  Serial.println(bus_data, BIN);
}

void testStrangeDeselect()
{
  Serial.println("\n-----------> TEST STRANGE DESELECT");

  // SEST REGA to react to Interrupts
  digitalWrite(RS0, HIGH);
  digitalWrite(RS1, LOW);
  writeToCIA();
  byteToDataBus(0b00000101);
  fullCycle();
  Serial.println("SET REG A TO REACT TO CA1");
  Serial.print("IRQA BEFORE CA1: ");
  Serial.println(digitalRead(IRQAB));

  // READ PERIPHERIAL A
  digitalWrite(RS0, LOW); // Per A
  digitalWrite(RS1, LOW); // Per A
  readFromCIA();
  fullCycle();
  readData();
  Serial.print("PERIPHERIAL READ: ");
  Serial.println(bus_data, BIN);

  digitalWrite(CS2B, HIGH); // DESELECT

  // SET REGA to react to Interrupts
  digitalWrite(RS0, HIGH);
  digitalWrite(RS1, LOW);
  // writeToCIA();
  // byteToDataBus(0b00000001);
  // fullCycle();
  // readData();
  // Serial.print("REGA: ");
  // Serial.println(bus_data);

  // digitalWrite(CS2B, LOW); // SELECT

  Serial.print("IRQA AFTER PERIPHERIAL READ: ");
  Serial.println(digitalRead(IRQAB));
    tick();

  digitalWrite(CS2B, LOW); // SELECT

  // TRIGGER CA1

  digitalWrite(CA1, LOW); // TRIGGER CA1
  readData();
  Serial.print("IRQA AFTER CA1 LOW AND BEFORE CYCLE: ");
  Serial.println(digitalRead(IRQAB));
  digitalWrite(CA1, HIGH); // DETRIGGER CA1
  Serial.print("REGA READ: ");
  Serial.println(bus_data, BIN);
  
  
  fullCycle();
  Serial.print("IRQA AFTER CA1 LOW AFTER CYCLE: ");
  Serial.println(digitalRead(IRQAB));


  digitalWrite(CA1, HIGH);
  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();
  readData();
  Serial.print("REGA READ: ");
  Serial.println(bus_data, BIN);
  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();
  fullCycle();

  // READ PERIPHERIAL A
  digitalWrite(CS2B, LOW); // SELECT

  digitalWrite(RS0, LOW); // Per A
  digitalWrite(RS1, LOW); // Per A
  readFromCIA();
  fullCycle();
  readData();
  Serial.print("PERIPHERIAL READ: ");
  Serial.println(bus_data, BIN);

  digitalWrite(CS2B, HIGH); // DESELECT

  digitalWrite(RS0, HIGH);
  digitalWrite(RS1, LOW);

  digitalWrite(CS2B, LOW); // SELECT
  fullCycle();
  readData();
  Serial.print("REGA READ: ");
  Serial.println(bus_data, BIN);
  Serial.print("IRQA AFTER CA1 LOW: ");
  Serial.println(digitalRead(IRQAB));

  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();
  fullCycle();
}

void test_CA1()
{
  Serial.println("\n-----------> TEST CA1");

  // WRITE REG A (active int)
  writeToCIA();
  byteToDataBus(0b00000001);
  fullCycle();

  readFromCIA();
  fullCycle();

  // RESULT
  Serial.print("IRQAB NO IRQ: ");
  Serial.println(digitalRead(IRQAB));

  Serial.print("REGA AFTER CIA SET: ");
  readData();
  Serial.println(bus_data, BIN);

  digitalWrite(CA1, LOW); // TRIGGER INT
  fullCycle();

  readData();
  Serial.print("REGA AFTER INTERRUPT TRIGGERED on CA1: ");
  Serial.println(bus_data, BIN);
  fullCycle();

  // RESULT
  Serial.print("IRQAB AFTER INPUT TRIGGERED: ");
  Serial.println(digitalRead(IRQAB));

  digitalWrite(CA1, HIGH); // UNTRIGGER INT
  fullCycle();

  // RESULT
  Serial.print("IRQAB AFTER CA1 DETRIG: ");
  Serial.println(digitalRead(IRQAB));

  readData();
  Serial.print("REGA AFTER CA1 DETRIG: ");
  Serial.println(bus_data, BIN);
  fullCycle();

  fetchPeripherialA();

  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW);  // Control Register A
  fullCycle();
  readData();
  Serial.print("REGA AFTER PER A READ: ");
  Serial.println(bus_data, BIN);
  fullCycle();
  Serial.print("IRQAB AFTER A READ: ");
  Serial.println(digitalRead(IRQAB));

  
  Serial.println("TRIGGER CA1 AGAIN AFTER P READ");
  digitalWrite(CA1, LOW); // UNTRIGGER INT
  Serial.print("IRQAB AFTER P READ: ");
  Serial.println(digitalRead(IRQAB));
  fullCycle();
  digitalWrite(CA1, HIGH); // UNTRIGGER INT

  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();


  Serial.println("TRIGGER CA1 AGAIN AFTER DESELECT");
  digitalWrite(CA1, LOW); // UNTRIGGER INT
  fullCycle();
  Serial.print("IRQAB AFTER AFTER DESELECT: ");
  Serial.println(digitalRead(IRQAB));
  fullCycle();

  digitalWrite(CS2B, LOW); // SELECT
  fullCycle();


 readData();
  Serial.print("REGA AFTER DESELECT");
  Serial.println(bus_data, BIN);
    Serial.print("IRQAB AFTER AFTER DESELECT: ");
  Serial.println(digitalRead(IRQAB));
  fullCycle();


}

void test_CA1CA2()
{
  Serial.println("\n-----------> TEST CA1CA2");
  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW);  // Control Register A
  writeToCIA();
  byteToDataBus(0b00001001); // Bit 3 at 1 so CA1 & CA2 now trigger
  fullCycle();

  readFromCIA();
  fullCycle();
  readData();
  Serial.print("REGA AFTER BIT3 CA2 SET: ");
  Serial.println(bus_data, BIN);

  digitalWrite(CA2, LOW); // TRIGGER CA2
  fullCycle();
  readData();
  Serial.print("REGA AFTER CA2 triggered: ");
  Serial.println(bus_data, BIN);
  Serial.print("IRQAB AFTER CA2 triggered: ");
  Serial.println(digitalRead(IRQAB));
  Serial.println("-------------------------------");

  digitalWrite(CA1, LOW); // TRIGGER CA1
  fullCycle();
  readData();

  Serial.print("REGA AFTER CA1 triggered: ");
  Serial.println(bus_data, BIN);
  Serial.print("IRQAB AFTER CA1 triggered: ");
  Serial.println(digitalRead(IRQAB));
  Serial.println("-------------------------------");

  fetchPeripherialA();
  digitalWrite(RS0, HIGH); // Control Register A
  digitalWrite(RS1, LOW);  // Control Register A
  fullCycle();
  readData();
  Serial.print("REGA AFTER PER A READ: ");
  Serial.println(bus_data, BIN);
  fullCycle();
  Serial.print("IRQAB AFTER A READ: ");
  Serial.println(digitalRead(IRQAB));
}

void initializePia()
{
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
  digitalWrite(RS1, LOW);  // Control Register A

  Serial.begin(SERIAL_SPEED);
  fullCycle();
  tick(); // ALIGN TO POSITIVE CLOCK

  digitalWrite(RESB, HIGH); // STOP RESET
  fullCycle();

  digitalWrite(CS2B, LOW); // SELECT
  fullCycle();

  Serial.println("----------------------------");
  Serial.println("65C21 Test");
  Serial.println("----------------------------");
  Serial.print("REGA AFTER RESET: ");
  readData();
  Serial.println(bus_data, BIN);
}

// the setup function runs once when you press reset or power the board
void setup()
{
  initializePia();

  //testStrangeDeselect();

  // TEST CA1 Interrupts
  test_CA1();

  // TEST CA1 + CA2 Interrupts
  // test_CA1CA2();

  digitalWrite(CS2B, HIGH); // DESELECT
  fullCycle();

  // READY
}

// the loop function runs over and over again forever
void loop()
{
  // wait for a second
}