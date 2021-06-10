#include <SPI.h>
#include <Ethernet.h>
#include "Ewma.h"

// #ifdef PRINT
// #error PRINT already defined
// #endif
//#define PRINT

// #ifndef SIMULATE
// #define SIMULATE
// uint8_t count = 0;
// #else
// #error SIMULATE already defined
// #endif

#ifndef FASTADC
#define FASTADC 1
#else
#error FASTADC already defined
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#else
#error cbi already defined
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#else
#error sbi already defined
#endif

#define IDNUM 18
byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x12};
IPAddress ip(172,16,10,118); // byte ip[] = {172, 16, 10, 200};
IPAddress gateway(0,0,0,0); // byte gateway[] = {0, 0, 0, 0};
IPAddress subnet(255,255,255,0); // byte subnet[] = {255, 255, 255, 0};
IPAddress PC(172,16,10,1);
// byte fwd_pc_ip[] = {172, 16, 10, 1};
// byte PC[] = {172,29,220,218};
// char *PC = "172.29.220.218";
// char *PC = "google.com";
/* EthernetServer server(4012); */ EthernetServer server = EthernetServer(4012);
// EthernetClient fwd_pc = EthernetClient(4013);
#define PORT 4012
// EthernetClient hostClient;
EthernetClient client;

#define LATCH_PIN 19
#define CLOCK_PIN 18
#define DATA_PIN 17
#define FWD_PORT 4013

#define SPI_CLK_PIN 7
#define SPI_DATA_PIN  6
#define ENABLE_MIN 5

#define JB2_V1 8
#define JB2_V2 15
#define JB2_V3 16

#define GB_V1 2
#define GB_V2 3
#define GB_V3 4

//array to store input and output ports of the element (user input)
char BYTES[4]; // extra byte for print-friendly null terminal
bool LISTENING = true;
bool got3 = false;
bool got5 = false;

#define MINPIN1 2
#define MAXPIN1 8
#define MINPIN2 15
#define MAXPIN2 19

Ewma fastFilter(0.1);
Ewma slowFilter(0.01); // lower than fastFilter, 0 to 1 (1 no filter)
// good values: .09 and .01

int ITER;
#define NSAMPLES 450
#define MIN_SEP  70
#define MAX_SEP  200
#define SEP_ERROR 15
#define MIN_VOLTAGE_INDEX 300

#define SENSORPIN A0

int SENSORVALS[NSAMPLES];
int WINDOW[5];
int wIndex = 0;
int val;
// char dirs[4] = {'w', 'e', 'n', 's'};

void set_io(char inAnt, char outAnt);
void set_phase(int phase);
int hex2int(char hex);
void set_JB2_pins(char outAnt);
void set_GB_pins(char inAnt);
void set_amp_pin(char outAnt);
void measure(void);
int analyze(void);
void waiting(int nsec);

struct Message {
  short idnum;
  short score;
  short antenna;
} msg;

// union Raw {
//   struct Message msg;
//   uint8_t src[sizeof(struct Message)];
// } raw;

// union Raw {
//   struct Message msg;
//   uint8_t src[sizeof(struct Message)];
// } raw;

// union Source {
//   struct Data data;
//   uint8_t srcBytes[sizeof(struct Data)];
// } EXTREME;

// union Raw {
//   int num;
//   uint8_t src[sizeof(int)];
// };

void setup(void){

  msg.idnum = IDNUM;

  // Serial.begin(9600);
  // Serial.println("Boot process begins...");

#if FASTADC
  // set prescale to 16
  // Serial.println("Setting up FASTADC");
  sbi(ADCSRA, ADPS2) ;
  cbi(ADCSRA, ADPS1) ;
  cbi(ADCSRA, ADPS0) ;
#endif

  // Set all pins to output mode
  // Serial.println("Setting pinModes...");
  pinMode(SENSORPIN, INPUT_PULLUP);
  for (int i = MINPIN1; i <= MAXPIN1; i++) {
    pinMode(i, OUTPUT);
  }
  for (int i = MINPIN2; i <= MAXPIN2; i++) {
    pinMode(i, OUTPUT);
  }

  // Starting Ethernet server
  Ethernet.begin(mac, ip, gateway, subnet);
  // Serial.print("Waiting for Ethernet shield");
  waiting(5);
  // server.begin();
  // Serial.print("Server is at ");
  // Serial.println(Ethernet.localIP());
  // Checking short circuits
  // Serial.println("Testing for short circuits...");
  set_io('-', 'w');
  waiting(5);
  set_io('-', 'e');
  waiting(5);
  set_io('-', 'n');
  waiting(5);
  set_io('-', 's');
  waiting(5);
  // set_io('-', 'w');
  // waiting(1);
  // set_io('-', 'e');
  // waiting(1);
  // set_io('-', 'n');
  // waiting(1);
  // set_io('-', 's');
  // waiting(1);

  BYTES[3] = '\0'; // just for easy printing

  set_io('e', '-');
  set_phase(0);

  // client = server.accept();
  // Serial.println("Connecting to host...");
  // while(!hostClient.connect(host,4012));

  // Serial.println("Connecting to PC...");
   // while(!client.connect(PC,PORT));
   // delay(1000);
  while(client.connect(PC,PORT) != 1);
  // waiting(5);

  // Serial.println("Boot process ended!");
  // Serial.println("Ready for a command");

  // while(1){
  //   measure();
  //   if(!analyze()){
  //     break;
  //   }
  // }

  // int nfalse = 0;
  // while(nfalse != 5){
  //   measure();
  //   if(!analyze()){
  //     Serial.println("Above is false negative");
  //     nfalse++;
  //   }
  // }

  return;
}

void waiting(int nsec) {
  for (int i = 1; i <= nsec; i++) {
    delay(1000);
    // Serial.print("...");
    // Serial.print(i);
  }
  // Serial.println();
  return;
}

void loop(void) {
  // EthernetClient client = server.available();
  if(!client.connected()){
     // Serial.println("Disconnected, reconnecting...");
     client.stop();
     while(client.connect(PC,PORT) != 1);
  }
  if(client.available()){
    ITER = 0;
    while (client.connected() && ITER < 3) {
      BYTES[ITER++] = client.read();
    }
    if (ITER < 3) {
      // Serial.println("Failed to fill BYTES");
    }
    else {
      set_io(BYTES[0], BYTES[1]);
      //for (int i=0;i<50;i++){
        //unsigned long time = millis();
        // measure();
        // keepExtreme();
        //Serial.println(millis() - time);
        // Serial.println("Done!");
      //}
      //client.write(EXTREME.srcBytes, sizeof(EXTREME.srcBytes));
      return;
    }
  } else {
      measure();
      // keepExtreme();
      val = analyze();
      // Serial.print("Score: ");
      // Serial.println(val);
      WINDOW[wIndex] = val;
      if(wIndex == 2){
        got3 = true;
      } else if(wIndex == 4){
        got5 = true;
      }

      if(LISTENING){
        // Serial.print("wIndex is: ");
        // Serial.println(wIndex);
        // Serial.print("Plus 5 mod 5: ");
        // Serial.println((wIndex + 5) % 5);
        if(got3 && WINDOW[(wIndex + 5) % 5] > 0 && WINDOW[(wIndex - 1 + 5) % 5] > 0 && WINDOW[(wIndex - 2 + 5) % 5] > 0){
          // Serial.println("Transmitter nearby");
          LISTENING = false;
          msg.score = val;
          msg.antenna = getAntenna();
          client.write((uint8_t *)&msg,sizeof(msg));
        }
      } else { // not listening (reporting)
        if(!val){ // zero
          if(got5 && !WINDOW[0] && !WINDOW[1] && !WINDOW[2] && !WINDOW[3] && !WINDOW[4]){
            // Serial.println("Transmitter gone");
            LISTENING  = true;
            msg.score = 0;
            msg.antenna=0;
            client.write((uint8_t *)&msg,sizeof(msg));
          }
        } else { // not zero
          msg.score = val;
          client.write((uint8_t *)&msg,sizeof(msg));
        }
      }

      if(++wIndex == 5){
        wIndex = 0;
      }

      // #ifdef SIMULATE
      // if(!count++){
      //   msg.score = 7;
      // }
      // #endif

      // Serial.print("val.num is: ");
      // Serial.println(val.num);


      // if(msg.score){
        // sendReport();
        // int ret = client.write((uint8_t *)&msg,sizeof(msg));
        // Serial.print("Wrote ");
        // Serial.println(ret);
      // }


  }
  return;
}


int analyze(void) {

  float filteredFast;
  float filteredSlow;
  int crossings[6] = {0, 0, 0, 0, 0, 0};
  boolean fast_up = false;
  boolean first_valid = false;
  float maxData = 0;
  float minData = 100;
  int cnt = 0;
  int prev_crossing = 0;
  float max_slow_filter = 0;
  float min_slow_filter = 100;

  fastFilter.reset();
  slowFilter.reset();

  filteredFast = fastFilter.filter(SENSORVALS[0] * 5.0 / 1023.0);
  filteredSlow = slowFilter.filter(SENSORVALS[0] * 5.0 / 1023.0);

  if (filteredFast > filteredSlow) {
    fast_up =  true;
  }

  // Detecting edges
  for (int i = 1; i < NSAMPLES; i++) {

    filteredFast = fastFilter.filter(SENSORVALS[i] * 5.0 / 1023.0);
    filteredSlow = slowFilter.filter(SENSORVALS[i] * 5.0 / 1023.0);

    if (filteredFast > filteredSlow) {
      if (!fast_up) { // edge detected
        if (cnt + 1 > 6) {
          break;
        }
        if (i - prev_crossing > MIN_SEP && i - prev_crossing < MAX_SEP) {
          if (!first_valid) {
            if (prev_crossing == 0) {
              cnt++;
              crossings[cnt-1] = i;
              prev_crossing = i;
              first_valid = true;
            }
            else {
              crossings[0] = prev_crossing;
              cnt = cnt + 2;
              crossings[cnt-1] = i;
              prev_crossing = i;
              first_valid = true;
            }
          }
          else {
            cnt++;
            crossings[cnt-1] = i;
            prev_crossing = i;
          }
        }
        else {
          if (!first_valid) {
            prev_crossing = i;
          }
        }
      }
      fast_up =  true;
    }
    else if (filteredFast < filteredSlow) {
      if (fast_up) { // edge detected
        if (cnt + 1 > 6) {
          break;
        }
        if (i - prev_crossing > MIN_SEP && i - prev_crossing < MAX_SEP) {
          if (!first_valid) {
            if (prev_crossing == 0) {
              cnt++;
              crossings[cnt-1] = i;
              prev_crossing = i;
              first_valid = true;
            }
            else {
              crossings[0] = prev_crossing;
              cnt = cnt + 2;
              crossings[cnt-1] = i;
              prev_crossing = i;
              first_valid = true;
            }
          }
          else {
            cnt++;
            crossings[cnt-1] = i;
            prev_crossing = i;
          }
        }
        else {
          if (!first_valid) {
            prev_crossing = i;
          }
        }
      }
      fast_up =  false;
    }

    if (filteredFast > maxData) {
      maxData = filteredFast;
    }
    if (filteredFast < minData) {
      minData = filteredFast;
    }

    if (filteredSlow > max_slow_filter) {
      max_slow_filter = filteredSlow;
    }
    if (filteredSlow < min_slow_filter) {
      min_slow_filter = filteredSlow;
    }
  }

  // Determining packet detection


  if (cnt < 3) { // we expect at least 3 crossings
    return 0;
  }
  
  if (max_slow_filter - min_slow_filter < 0.1) {
    return 0;
  }

  if (cnt == 4) {
    if (abs((crossings[1] - crossings[0]) - (crossings[3] - crossings[2])) < SEP_ERROR) {
        int voltage_index = (int)((maxData - minData) * 1000.00);
        return (voltage_index > MIN_VOLTAGE_INDEX) ? voltage_index : 0;

    }
  }

  if (cnt == 5) {
    if (abs((crossings[1] - crossings[0]) - (crossings[3] - crossings[2])) < SEP_ERROR || abs((crossings[2] - crossings[1]) - (crossings[4] - crossings[3])) < SEP_ERROR) {
        int voltage_index = (int)((maxData - minData) * 1000.00);
        return (voltage_index > MIN_VOLTAGE_INDEX) ? voltage_index : 0;
    }
  }

  if (cnt == 6) {
    if (abs((crossings[1] - crossings[0]) - (crossings[3] - crossings[2])) < SEP_ERROR || abs((crossings[2] - crossings[1]) - (crossings[4] - crossings[3])) < SEP_ERROR || abs((crossings[3] - crossings[2]) - (crossings[5] - crossings[4])) < SEP_ERROR) {
        int voltage_index = (int)((maxData - minData) * 1000.00);
        return (voltage_index > MIN_VOLTAGE_INDEX) ? voltage_index : 0;

    }
  }
return 0;
}


void measure(void) {
  for (int i = 0; i < NSAMPLES; i++) {
    SENSORVALS[i] = analogRead(SENSORPIN);
  }
}


int hex2int(char hex) {
  if (hex >= '0' && hex <= '9') {
    return hex - '0';
  }
  else if (hex >= 'a' && hex <= 'f') {
    return hex - 'a' + 10;
  }
  else {
    return 0;
  }
}

int getAntenna(void){
  
  int max_index=0;
  int curr_val;
  int antenna;

  set_io('w', '-');
  delay(1);
  measure();
  curr_val = analyze();
  if (curr_val>max_index){
    max_index=curr_val;
    antenna=(int)('w');
  }

  set_io('e', '-');
  delay(1);
  measure();
  curr_val = analyze();
  if (curr_val>max_index){
    max_index=curr_val;
    antenna=(int)('e');
  }  

  set_io('n', '-');
  delay(1);
  measure();
  curr_val = analyze();
  if (curr_val>max_index){
    max_index=curr_val;
    antenna=(int)('n');
  } 

  set_io('s', '-');
  delay(1);
  measure();
  curr_val = analyze();
  if (curr_val>max_index){
    max_index=curr_val;
    antenna=(int)('s');
  } 
  return max_index>0 ? antenna : (int)('e');
}

void set_io(char inAnt, char outAnt) {
  /* BASED ON GIVEN CODE */
  // Serial.print("Setting ");
  // Serial.print(inAnt);
  // Serial.println(outAnt);
  set_GB_pins(inAnt);
  if (outAnt == 'a' || outAnt == 'b' || outAnt == 'c' || outAnt == 'd' || outAnt == '-') {
    set_JB2_pins('-');
  }
  else if (outAnt == 'n' || outAnt == 'e' || outAnt == 's' || outAnt == 'w') {
    set_JB2_pins(outAnt);
  }
  else {
    // Serial.println("set_io(): invalid outAnt, returning");
  }
  set_amp_pin(outAnt);
  return;
}


void set_GB_pins(char inAnt) { /* BASED ON GIVEN CODE */
  switch (inAnt) {
  case 'a':
    digitalWrite(GB_V1, 0);
    digitalWrite(GB_V2, 0);
    digitalWrite(GB_V3, 0);
    break;
  case 'w':
    digitalWrite(GB_V1, 1);
    digitalWrite(GB_V2, 0);
    digitalWrite(GB_V3, 0);
    break;
  case 'b':
    digitalWrite(GB_V1, 0);
    digitalWrite(GB_V2, 1);
    digitalWrite(GB_V3, 0);
    break;
  case 'c':
    digitalWrite(GB_V1, 1);
    digitalWrite(GB_V2, 1);
    digitalWrite(GB_V3, 0);
    break;
  case 's':
    digitalWrite(GB_V1, 0);
    digitalWrite(GB_V2, 0);
    digitalWrite(GB_V3, 1);
    break;
  case 'd':
    digitalWrite(GB_V1, 1);
    digitalWrite(GB_V2, 0);
    digitalWrite(GB_V3, 1);
    break;
  case 'e':
    digitalWrite(GB_V1, 0);
    digitalWrite(GB_V2, 1);
    digitalWrite(GB_V3, 1);
    break;
  case 'n':
    digitalWrite(GB_V1, 1);
    digitalWrite(GB_V2, 1);
    digitalWrite(GB_V3, 1);
    break;
  default:
    //Serial.println("set_GB_pins(): invalid inAnt, defaulting to A");
    digitalWrite(GB_V1, 0);
    digitalWrite(GB_V2, 0);
    digitalWrite(GB_V3, 0);
    break;
  }
}


void set_phase(int phase) { /* BASED ON GIVEN CODE */
  // need to send two dummy bits at the end even though it's a 4-bit phase shifter
  bool bits[6] = {
    0
  }; //sets each bit to 0
  for (int i = 0; i < 4; i++) {
    bits[i] = phase & (1 << (4 - i - 1));
  }

  digitalWrite(SPI_CLK_PIN, LOW);
  digitalWrite(ENABLE_MIN, LOW);

  for (unsigned int i = 0; i < sizeof(bits) / sizeof(bits[0]); i++) {
    digitalWrite(SPI_DATA_PIN, bits[i]);
    digitalWrite(SPI_CLK_PIN, HIGH);
    digitalWrite(SPI_CLK_PIN, LOW);
  }
  // //Serial.println();

  digitalWrite(ENABLE_MIN, HIGH);
}


void set_JB2_pins(char outAnt) {
  switch (outAnt) {
  case 'n':
    digitalWrite(JB2_V1, 0);
    digitalWrite(JB2_V2, 0);
    digitalWrite(JB2_V3, 0);
    break;
  case 'e':
    digitalWrite(JB2_V1, 0);
    digitalWrite(JB2_V2, 0);
    digitalWrite(JB2_V3, 1);
    break;
  case 's':
    digitalWrite(JB2_V1, 0);
    digitalWrite(JB2_V2, 1);
    digitalWrite(JB2_V3, 0);
    break;
  case 'w':
    digitalWrite(JB2_V1, 0);
    digitalWrite(JB2_V2, 1);
    digitalWrite(JB2_V3, 1);
    break;
  case '-': //turn OFF JB2
    digitalWrite(JB2_V1, 1);
    digitalWrite(JB2_V2, 1);
    digitalWrite(JB2_V3, 1);
    break;
  default:
    digitalWrite(JB2_V1, 1);
    digitalWrite(JB2_V2, 1);
    digitalWrite(JB2_V3, 1);
    break;
  }
}


void set_amp_pin(char outAnt) { /* BASED ON GIVEN CODE */
  switch (outAnt) {
  case '-':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'a':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 1); //1<<0
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'b':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 2); //1<<1
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'c':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 4); //1<<2
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'd':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 8); //1<<3
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'w':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 16); //1<<4
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'e':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 32); //1<<5
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 'n':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 64); //1<<6
    digitalWrite(LATCH_PIN, HIGH);
    break;
  case 's':
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 128); //1<<7
    digitalWrite(LATCH_PIN, HIGH);
    break;
  default:
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
    digitalWrite(LATCH_PIN, HIGH);
    break;
  }
}


