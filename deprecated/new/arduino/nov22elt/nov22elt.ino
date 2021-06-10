// nov22elt.ino -- created by Will Sussman on November 22, 2019

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0x00,0x00,0x00,0x00,0x00,0x0D};
byte ip[] = {172,16,10,113};
byte gateway[] = {0,0,0,0};
byte subnet[] = {255,255,255,0};
EthernetServer server = EthernetServer(4012);
EthernetClient client;
char BYTES[3];

int MINPIN1 = 2;
int MAXPIN1 = 7;

int GB_V1 = 2;
int GB_V2 = 3;
int GB_V3 = 4;

int SPI_CLK_PIN = 7;
int SPI_DATA_PIN = 6;
int ENABLE_MIN = 5;

// int JB1_V1 = 2;
// int JB1_V2 = 2;
// int JB1_V3 = 2;

int MINPIN2 = 14;
int MAXPIN2 = 19;

int JB2_V1 = 14;
int JB2_V2 = 15;
int JB2_V3 = 16;

int LATCH_PIN = 19;
int CLOCK_PIN = 18;
int DATA_PIN = 17;

// enum dirs{N,E,S,W,A,B,C,D,X} IN_ANT, OUT_ANT;
char IN_ANT, OUT_ANT;
enum states{IN,OUT,PHASE} STATE = IN;

void set_phase(int phase);
void set_io(char inAnt, char outAnt);
int hex2int(char hex);
void set_GB_pins(char inAnt);
void set_amp_pin(char outAnt);
void set_JB2_pins(char outAnt);
// void set_JB1_pins(char outAnt);
void printVs(void);

void setup(){
	//Serial.begin(9600);
	for(int i = MINPIN1; i <= MAXPIN1; i++){
		pinMode(i,OUTPUT);
	}
	for(int i = MINPIN2; i <= MAXPIN2; i++){
		pinMode(i,OUTPUT);
	}
	set_io('-','-');
	set_phase(0);
	return;
}

void printVs(void){
	//Serial.print("GB_Vs are: ");
	//Serial.print(GB_V1);
	//Serial.print(" ");
	//Serial.print(GB_V2);
	//Serial.print(" ");
	//Serial.println(GB_V3);

	// //Serial.print("JB1_Vs are: ");
	// //Serial.print(JB1_V1);
	// //Serial.print(" ");
	// //Serial.print(JB1_V2);
	// //Serial.print(" ");
	// //Serial.println(JB1_V3);

	//Serial.print("JB2_Vs are: ");
	//Serial.print(JB2_V1);
	//Serial.print(" ");
	//Serial.print(JB2_V2);
	//Serial.print(" ");
	//Serial.println(JB2_V3);
}

void loop(){ //reads one char at a time

	/* BASED ON GIVEN CODE */
	client = server.available();
	if(client){
		int i = 0;
		while(client.connected()){
			BYTES[i++] = client.read();
		}
	}
	set_io(BYTES[0],BYTES[1]);
	set_phase(BYTES[2]);
	set_amp_pin(BYTES[1]);
	return;

	// char gotByte;
	// if(//Serial.available()){ //there are chars to read
	// 	gotByte = //Serial.read();
	// 	if(gotByte == '\n'){
	// 		return; //skip
	// 	}

	// 	if(STATE == IN){
	// 		IN_ANT = gotByte;
	// 		STATE = OUT;
	// 	} else if(STATE == OUT){
	// 		OUT_ANT = gotByte;
	// 		STATE = PHASE;
	// 	} else if(STATE == PHASE){
	// 		set_io(IN_ANT,OUT_ANT);
	// 		set_phase(hex2int(gotByte));
	// 		STATE = IN;
	// 	} else {
	// 		//Serial.println("loop(): invalid STATE, returning");
	// 		return;
	// 	}
	// }
	// return;
}

int hex2int(char hex){
	if(hex >= '0' && hex <= '9'){
		return hex - '0';
	} else if(hex >= 'a' && hex <= 'f'){
		return hex - 'a' + 10;
	} else {
		//Serial.println("hex2int(): invalid hex, returning 0");
		return 0;
	}
}

void set_io(char inAnt, char outAnt){
	//Serial.println("calling set_io() on inAnt and outAnt:");
	//Serial.println(inAnt);
	//Serial.println(outAnt);
	//Serial.println();
	/* BASED ON GIVEN CODE */
	set_GB_pins(inAnt);
	if(outAnt == 'a' || outAnt == 'b'|| outAnt == 'c'|| outAnt == 'd'|| outAnt == '-'){
    	// set_JB1_pins(outAnt);
    	set_JB2_pins('-');
    } else if(outAnt == 'n' || outAnt == 'e'|| outAnt == 's'|| outAnt == 'w'){
	    set_JB2_pins(outAnt);
	    // set_JB1_pins('-');
    } else {
    	//Serial.println("set_io(): invalid outAnt, returning");
    }
	return;
}

void set_GB_pins(char inAnt){ /* BASED ON GIVEN CODE */
	//Serial.println("calling set_GB_pins() on inAnt:");
	//Serial.println(inAnt);
	//Serial.println();
	switch(inAnt){
		case 'a':
			digitalWrite(GB_V1,0);
			digitalWrite(GB_V2,0);
			digitalWrite(GB_V3,0);
			break;
		case 'w':
			digitalWrite(GB_V1,1);
			digitalWrite(GB_V2,0);
			digitalWrite(GB_V3,0);
			break;
		case 'b':
			digitalWrite(GB_V1,0);
			digitalWrite(GB_V2,1);
			digitalWrite(GB_V3,0);
			break;
		case 'c':
			digitalWrite(GB_V1,1);
			digitalWrite(GB_V2,1);
			digitalWrite(GB_V3,0);
			break;
		case 's':
			digitalWrite(GB_V1,0);
			digitalWrite(GB_V2,0);
			digitalWrite(GB_V3,1);
			break;
		case 'd':
			digitalWrite(GB_V1,1);
			digitalWrite(GB_V2,0);
			digitalWrite(GB_V3,1);
			break;
		case 'e':
			digitalWrite(GB_V1,0);
			digitalWrite(GB_V2,1);
			digitalWrite(GB_V3,1);
			break;
		case 'n':
			digitalWrite(GB_V1,1);
			digitalWrite(GB_V2,1);
			digitalWrite(GB_V3,1);
			break;
		default:
			//Serial.println("set_GB_pins(): invalid inAnt, defaulting to A");
			digitalWrite(GB_V1,0);
			digitalWrite(GB_V2,0);
			digitalWrite(GB_V3,0);
			break;
	}
	printVs();
	return;
}

void set_phase(int phase){ /* BASED ON GIVEN CODE */
    //Serial.print("Setting phase to ");
    //Serial.println(phase);

    // need to send two dummy bits at the end even though it's a 4-bit phase shifter
    bool bits[6] = {0}; //sets each bit to 0
    for (int i = 0; i < 4; i++) {
        bits[i] = phase & (1 << (4 - i - 1));
        // //Serial.print("bits[");
        // //Serial.print(i);
        // //Serial.print("] is now: ");
        // //Serial.println(bits[i]);
    }

    digitalWrite(SPI_CLK_PIN, LOW);
    digitalWrite(ENABLE_MIN, LOW);

    for (int i = 0; i < sizeof(bits) / sizeof(bits[0]); i++) {
        // //Serial.print("Sending bits[");
        // //Serial.print(i);
        // //Serial.print("], which is: ");
        // //Serial.println(bits[i]);
        digitalWrite(SPI_DATA_PIN, bits[i]);
        digitalWrite(SPI_CLK_PIN, HIGH);
        digitalWrite(SPI_CLK_PIN, LOW);
    }
    // //Serial.println();

    digitalWrite(ENABLE_MIN, HIGH);
    return;
}

// void set_JB1_pins(char outAnt){ /* BASED ON GIVEN CODE */
// 	//Serial.println("calling set_JB1_pins() on outAnt:");
// 	//Serial.println(outAnt);
// 	//Serial.println();
// 	switch(outAnt){
// 	  	case 'c':
// 			digitalWrite(JB1_V1,0);
// 			digitalWrite(JB1_V2,0);
// 			digitalWrite(JB1_V3,0);
// 		    break;
// 		case 'b':
// 		    digitalWrite(JB1_V1,0);
// 		    digitalWrite(JB1_V2,0);
// 		    digitalWrite(JB1_V3,1);
// 		    break;
// 		case 'd':
// 		    digitalWrite(JB1_V1,0);
// 		    digitalWrite(JB1_V2,1);
// 		    digitalWrite(JB1_V3,0);
// 		    break;
// 		case 'a':
// 		    digitalWrite(JB1_V1,0);
// 		    digitalWrite(JB1_V2,1);
// 		    digitalWrite(JB1_V3,1);
// 		    break;
// 		case '-': //turn OFF JB1
// 		    digitalWrite(JB1_V1,1);
// 		    digitalWrite(JB1_V2,1);
// 		    digitalWrite(JB1_V3,1);
// 		    break;
// 		default:
// 			//Serial.println("set_JB1_pins(): invalid outAnt, defaulting to -");
// 			digitalWrite(JB1_V1,1);
// 		    digitalWrite(JB1_V2,1);
// 		    digitalWrite(JB1_V3,1);
// 		    break;
// 	}
// 	printVs();
// 	return;
// }

void set_JB2_pins(char outAnt){ /* BASED ON GIVEN CODE */
	//Serial.println("calling set_JB2_pins() on outAnt:");
	//Serial.println(outAnt);
	//Serial.println();
	switch(outAnt){
		case 'n':
			digitalWrite(JB2_V1,0);
			digitalWrite(JB2_V2,0);
			digitalWrite(JB2_V3,0);
			break;
		case 'e':
			digitalWrite(JB2_V1,0);
			digitalWrite(JB2_V2,0);
			digitalWrite(JB2_V3,1);
			break;
		case 's':
			digitalWrite(JB2_V1,0);
			digitalWrite(JB2_V2,1);
			digitalWrite(JB2_V3,0);
			break;
		case 'w':
			digitalWrite(JB2_V1,0);
			digitalWrite(JB2_V2,1);
			digitalWrite(JB2_V3,1);
			break;
		case '-': //turn OFF JB2
			digitalWrite(JB2_V1,1);
			digitalWrite(JB2_V2,1);
			digitalWrite(JB2_V3,1);
			break;
		default:
			//Serial.println("set_JB2_pins(): invalid outAnt, defaulting to -");
			digitalWrite(JB2_V1,1);
			digitalWrite(JB2_V2,1);
			digitalWrite(JB2_V3,1);
			break;
	}
	printVs();
	return;
}

void set_amp_pin(char outAnt){ /* BASED ON GIVEN CODE */
	//Serial.println("calling set_amp_pin() on outAnt:");
	//Serial.println(outAnt);
	//Serial.println();
	switch(outAnt){
		case '-':
			//Serial.println("set_amp_pin(): WARNING -- may be inaccurate");
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'a':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 1);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'b':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 2);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'c':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 4);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'd':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 8);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'w':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 16);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'e':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 32);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 'n':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 64);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		case 's':
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 128);
			digitalWrite(LATCH_PIN, HIGH);
			break;
		default:
			//Serial.println("set_amp_pin(): invalid outAnt, defaulting to -");
			//Serial.println("set_amp_pin(): WARNING -- may be inaccurate");
			digitalWrite(LATCH_PIN, LOW);
			shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
			digitalWrite(LATCH_PIN, HIGH);
			break;
	}
	printVs();
	return;
}
