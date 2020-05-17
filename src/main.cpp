#include "I2Cdev.h"
#include "MPU6050.h"
#include "common.h"
#include <avr/pgmspace.h>

#define N 15
#define THRESHOLD 10000

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

bool recordFlag = false;
bool unlockFlag = false;



short ax, ay, az;
short gx, gy, gz;

short axRecord[N] = {0};
short ayRecord[N] = {0};
short azRecord[N] = {0};

short gxRecord[N] = {0};
short gyRecord[N] = {0};
short gzRecord[N] = {0};

short recordCounter = 0;

short axUnlock[N] = {0};
short ayUnlock[N] = {0};
short azUnlock[N] = {0};

short gxUnlock[N] = {0};
short gyUnlock[N] = {0};
short gzUnlock[N] = {0};

short unlockCounter = 0;

extern short DTW[N+1][N+1] = {32767};


// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


#define LED_PIN 13

short DTWDistance(const short* a, const short* b) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            short cost = absDiff(*(a+i), *(b+j));
			//Serial.println(cost);
            DTW[i+1][j+1] = cost + min3(DTW[i][j+1], DTW[i+1][j], DTW[i][j]);
        }
    }
	short res = DTW[N][N];
	//Serial.println(res);
    DTW[N+1][N+1] = {32767};
    DTW[0][0] = 0;
    return res;
}

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(19200);

    // initialize devicer
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // use the code below to change accel/gyro offset values
    
    Serial.println("Updating internal sensor offsets...");
    // -76	-2359	1688	0	0	0
    accelgyro.setXAccelOffset(-2400);
	accelgyro.setYAccelOffset(700);
	accelgyro.setZAccelOffset(4120);
    accelgyro.setXGyroOffset(0);
    accelgyro.setYGyroOffset(0);
    accelgyro.setZGyroOffset(0);

    // configure Arduino LED pin for output

    pinMode(LED_PIN, OUTPUT);

	Serial.println("Allocating memory...");
	DTW[0][0] = 0;
	Serial.println("Setup Complete");
}

void loop() {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	
	if (!recordFlag && !unlockFlag) {
		if(Serial.available() > 0) {
			char operation = Serial.read();
			Serial.print("\n");
			Serial.print("Operation ");
			if(operation == 'r') {
				Serial.print(operation);
				Serial.println(": Record Gesture");
				recordFlag = true;
			} else if(operation == 'u') {
				Serial.print(operation);
				Serial.println(": Unlock Gesture");
				unlockFlag = true;
			} else {
				Serial.print(operation);
				Serial.println(": Not Defined");
			}
		}
	} else if (recordFlag && !unlockFlag) {
		if(recordCounter < N) {
			axRecord[recordCounter] = ax;
			ayRecord[recordCounter] = ay;
			azRecord[recordCounter] = az;
			gxRecord[recordCounter] = gx;
			gyRecord[recordCounter] = gy;
			gzRecord[recordCounter] = gz;
			Serial.print("ax: "); Serial.print(axRecord[recordCounter]); Serial.print(" | ");
    		Serial.print("ay: "); Serial.print(ayRecord[recordCounter]); Serial.print(" | ");
    		Serial.print("az: "); Serial.print(azRecord[recordCounter]); Serial.print(" | ");
    		Serial.print("gx: "); Serial.print(gxRecord[recordCounter]); Serial.print(" | ");
    		Serial.print("gy: "); Serial.print(gyRecord[recordCounter]); Serial.print(" | ");
    		Serial.print("gz: "); Serial.println(gzRecord[recordCounter]);
			++recordCounter;
		} else {
			const short *axRecordSave PROGMEM = axRecord;
			const short *ayRecordSave PROGMEM = ayRecord;
			const short *azRecordSave PROGMEM = azRecord;
			const short *gxRecordSave PROGMEM = gxRecord;
			const short *gyRecordSave PROGMEM = gyRecord;
			const short *gzRecordSave PROGMEM = gzRecord;

			recordCounter = 0;
			recordFlag = false;
		}
	} else if (!recordFlag && unlockFlag) {
		if(unlockCounter < N) {
			axUnlock[unlockCounter] = ax;
			ayUnlock[unlockCounter] = ay;
			azUnlock[unlockCounter] = az;
			gxUnlock[unlockCounter] = gx;
			gyUnlock[unlockCounter] = gy;
			gzUnlock[unlockCounter] = gz;
			Serial.print("ax: "); Serial.print(axUnlock[unlockCounter]); Serial.print(" | ");
    		Serial.print("ay: "); Serial.print(ayUnlock[unlockCounter]); Serial.print(" | ");
    		Serial.print("az: "); Serial.print(azUnlock[unlockCounter]); Serial.print(" | ");
    		Serial.print("gx: "); Serial.print(gxUnlock[unlockCounter]); Serial.print(" | ");
    		Serial.print("gy: "); Serial.print(gyUnlock[unlockCounter]); Serial.print(" | ");
    		Serial.print("gz: "); Serial.println(gzUnlock[unlockCounter]);
			++unlockCounter;
		} else {
			unlockCounter = 0;
			//
			short axDistance = abs(DTWDistance(axRecord, axUnlock)) / 100;
			short ayDistance = abs(DTWDistance(ayRecord, ayUnlock)) / 100;
			short azDistance = abs(DTWDistance(azRecord, azUnlock)) / 100;
			short gxDistance = abs(DTWDistance(gxRecord, gxUnlock)) / 100;
			short gyDistance = abs(DTWDistance(gyRecord, gyUnlock)) / 100;
			short gzDistance = abs(DTWDistance(gzRecord, gzUnlock)) / 100;

			// Serial.print("ax distance: "); Serial.print(axDistance); Serial.print(" ");
    		// Serial.print("ay distance: "); Serial.print(ayDistance); Serial.print(" ");
    		// Serial.print("az distance: "); Serial.print(azDistance); Serial.print(" ");
    		// Serial.print("gx distance: "); Serial.print(gxDistance); Serial.print(" ");
    		// Serial.print("gy distance: "); Serial.print(gyDistance); Serial.print(" ");
    		// Serial.print("gz distance: "); Serial.println(gzDistance);

			
			short accDistance = (axDistance + ayDistance + azDistance) / 3;
			short roDistance = (gxDistance + gyDistance + gzDistance) / 3;
			accDistance = accDistance == 0 ? 1 : accDistance;
			roDistance = roDistance == 0 ? 1 : roDistance;
			short distance = accDistance * roDistance;
			Serial.print("\n");
			Serial.print("Acceleration Distance: "); Serial.println(accDistance);
			Serial.print("Rotation Distance: "); Serial.println(roDistance);
			Serial.print("Total Distance: "); Serial.println(distance);
			if(distance > THRESHOLD || distance < 0) {
				Serial.println("Device Unlock Failed");
			} else {
				Serial.println("Device Unlocked");
				digitalWrite(LED_PIN, HIGH);
				delay(1000);
				digitalWrite(LED_PIN, LOW);
			}
			unlockFlag = false;
		}
	}
}