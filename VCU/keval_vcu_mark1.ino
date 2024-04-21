// SCS Implementation Left - BPS, Buzzer
// Flags
int rtdmode = 0, brakefault = 0, appsfault = 0, apps_scs = 0;

// Pins utilized in DAQ - 2, 12, 13, 14, 15, 16, 17, 32(can change), 33(can change), 34
// Pins avaliable - 4, 5, 18, 19, 22, 23, 25, 26, 27, 35(input only), 36(input only), 39(input only)

// Pin Definitions
#define buzzer 4
#define RTD_LED 5
#define pwm 23
#define testled 40 // ?
#define apps_led 25 //?
#define bspd_led 21 // ?

#define apps1_pin 26
#define apps2_pin 27
#define BPS_pin 39
#define RTDB_pin 36 
#define bps_scs_pin 18
#define ECU_SCS_pin 19
#define Air_State_pin 22

// Thresholds 12-bit ADC
#define lt1 1450.0   //1.25V 
#define lt2 750.0    //0.75V
#define ht1 3102.27  	//2.5V
#define ht2 1861.36   //1.5V
#define bps_th 930.68 //0.75V

// DAQ Part
#include <Ch376msc.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <LoRa.h>
#include <CAN.h>

#define TX_GPIO_NUM 17
#define RX_GPIO_NUM 16

#define HSPI_MOSI 13
#define HSPI_MISO 12
#define HSPI_SCK 14
#define HSPI_CS 15
#define RST 34
#define DI0 2
float canData[8] = {0.0, 2.3, 2.9, 1.5, 1.6, 1.9, 1.8, 0.7}; // Initialize array to store CAN data
uint8_t byteValue[8] = {100, 200, 255, 230, 150, 75, 63, 20};

SoftwareSerial mySerial(32, 33); // RX, TX pins on ESP32
Ch376msc flashDrive(mySerial);

TaskHandle_t Task1;
TaskHandle_t Task2;
void PWM_init() {
  // for motor controller
  ledcSetup(0, 5000, 8); // Use channel 0, 5000 Hz PWM, 8-bit resolution
  ledcAttachPin(pwm, 0);   
}

    
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    appendDataToFile();
    //LoRasend();
    delay(100);
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    ECU();
    delay(100);
  }
}
void ECU(){
float a, b, apps1, apps2, apps1_analog, apps2_analog,bps;
  int rtdb, bps_scs, ECU_SCS, Air_State;

  rtdb = analogRead(RTDB_pin);
  bps = analogRead(BPS_pin);
  bps_scs = digitalRead(bps_scs_pin);
  ECU_SCS = digitalRead(ECU_SCS_pin);
  Air_State = digitalRead(Air_State_pin);



  //Print the states of each pin
  Serial.print("RTDB Pin State: ");
  Serial.println(rtdb);
  Serial.print("BPS Pin State: ");
  Serial.println(bps);
  Serial.print("ECU_SCS Pin State: ");
  Serial.println(ECU_SCS);
  Serial.print("Air_State Pin State: ");
  Serial.println(Air_State);

  Serial.print("Value of BPS=");
  Serial.println(bps*3.3/4095);
  Serial.println(bps_th*3.3/4095);
  if (bps >= bps_th && rtdb > 4000 && ECU_SCS == 1 && Air_State == 1 && rtdmode == 0 && brakefault == 0 && appsfault == 0) { // pending - bps scs check
    unsigned long startTime = millis(); 

    while (millis() - startTime <= 2000) { 
      digitalWrite(buzzer, HIGH);
    }

    digitalWrite(buzzer, LOW);
    Serial.println("Buzz... Buzz... Buzz...");
    rtdmode = 1;
  }

  if (rtdmode) { 
    digitalWrite(RTD_LED, HIGH);
    Serial.println("RTD Mode Entered");
    bps = analogRead(BPS_pin);
    apps1_analog = analogRead(apps1_pin);
    apps2_analog = analogRead(apps2_pin);
    Serial.print("Apps1: ");
    apps1 = apps1_analog * 3.6 / 4095;
    Serial.println(apps1);
    apps2 = apps2_analog * 3.6 / 4095;
    Serial.print("Apps2: ");
    Serial.println(apps2);
    a = ((float)(apps1_analog - lt1) / (float)(ht1 - lt1));
    b = ((float)(apps2_analog - lt2) / (float)(ht2 - lt2));

    Serial.print("a: ");
    Serial.println(a);
    Serial.print("b: ");
    Serial.println(b);

    analogWrite(pwm, map(apps1_analog, lt1, ht1, 0, 255));

    while (ECU_SCS == 0) {
      ECU_SCS = digitalRead(ECU_SCS_pin);
      analogWrite(pwm, 0);
      digitalWrite(RTD_LED, LOW);
      rtdmode = 0;
      Serial.println("ECU SCS Check Failed... Exiting RTD Mode");
    }

    

    while (Air_State == 0) {
      Air_State = digitalRead(Air_State_pin);
      analogWrite(pwm, 0);
      digitalWrite(RTD_LED, LOW);
      rtdmode = 0;
      Serial.println("Air State Low... Exiting RTD Mode");
    }

    // Check
    // while (bps_scs == 0) {
    //   bps_scs = digitalRead(bps_scs_pin);
    //   analogWrite(pwm, 0);
    //   digitalWrite(RTD_LED, LOW);
    //   rtdmode = 0;
    //   Serial.println("BPS SCS Check Failed... Exiting RTD Mode");
    // }
    while((apps1_analog > 3500 || apps2_analog > 3500) && rtdmode == 1 && apps_scs==0)
    {
        Serial.println("APPS SCS Check Fault...");
        analogWrite(pwm, 0);
        digitalWrite(RTD_LED, LOW);
        Serial.println("Exiting RTD Mode");
    }

    if (analogRead(apps1_analog) < 3500 && analogRead(apps2_analog) < 3500)
    {
        digitalWrite(RTD_LED, HIGH);
        Serial.println("RTD Mode Entered...");
        apps_scs=0;
        analogWrite(pwm, map(apps1_analog, lt1, ht1, 0, 255));
    }
    bps = analogRead(BPS_pin);
    // Brake fault condition
    if (a > 0.25 && bps >= bps_th && brakefault == 0 && rtdmode == 1) {
      digitalWrite(bspd_led, HIGH);
      unsigned long startTime = millis(); 
      Serial.println("BPS fault... BSPD LED On");
      brakefault = 1;
      while (brakefault == 1) {
        bps = analogRead(BPS_pin);
        apps1_analog = analogRead(apps1_pin);
        a = ((float)(apps1_analog - lt1) / (float)(ht1 - lt1));
        b = ((float)(apps2_analog - lt2) / (float)(ht2 - lt2));

        if (millis() - startTime < 2000 && ((a < 0.25) || bps < bps_th)) { // 500ms plausibility check
          digitalWrite(RTD_LED, HIGH);
          digitalWrite(bspd_led, LOW);
          brakefault = 0;
          Serial.println("RTD Mode Re-entered");
        }
        if (millis() - startTime > 2000) {
          analogWrite(pwm, 0);
          digitalWrite(RTD_LED, LOW);
          Serial.println("Exiting RTD Mode");
          brakefault = 1;
        }
        
        apps1_analog = analogRead(apps1_pin);
        apps2_analog = analogRead(apps2_pin);
        a = ((float)(apps1_analog - lt1) / (float)(ht1 - lt1));
        b = ((float)(apps2_analog - lt2) / (float)(ht2 - lt2));
        if (a > 0.05 || b > 0.05) {
          analogWrite(pwm, 0);
        }
        else if (a < 0.05 || b < 0.05) {
          analogWrite(pwm, map(apps1_analog, lt1, ht1, 0, 255));
          digitalWrite(RTD_LED, HIGH);
          digitalWrite(bspd_led, LOW);
          brakefault = 0;
          Serial.println("BSPD LED Off");
          Serial.println("RTD Mode Re-entered");
        }
      }
      }
    // APPS Fault condition
    if (abs(a - b) > 0.3 && rtdmode == 1) { // 10% implausibility
      unsigned long startTime = millis(); 
      appsfault = 1;
      while (appsfault == 1) {
        apps1_analog = analogRead(apps1_pin);
        apps2_analog = analogRead(apps2_pin);
        digitalWrite(apps_led, HIGH);
        a = ((float)(apps1_analog - lt1) / (float)(ht1 - lt1));
        b = ((float)(apps2_analog - lt2) / (float)(ht2 - lt2));
        if (millis() - startTime <= 200 && (abs(a - b) < 0.3)) { // 100ms condition
          digitalWrite(apps_led, LOW);
          Serial.println("APPS LED Off");
          appsfault = 0;
        }
        if (millis() - startTime > 2000) { // 500ms plausibility check
          analogWrite(pwm, 0);
          digitalWrite(RTD_LED, LOW);
          digitalWrite(apps_led, HIGH);
          appsfault = 1;
          rtdmode = 0;
          Serial.println("Apps fault occurred. RTD LED turned LOW. APPS LED turned HIGH");
          delay(1000);
        }
      }
    }
  }
}

void createAndWriteFile() {
    Serial.print("Creating and writing to file: TEST1.CSV");
    flashDrive.setFileName("TEST1.CSV");
    flashDrive.openFile();

    char header[] = "sensor 1, sensor 2, sensor 3, sensor 4, sensor 5, sensor 6, sensor 7, sensor 8\n";
    flashDrive.writeFile(header, strlen(header));

    flashDrive.closeFile();
    Serial.println("Done!");
}

void appendDataToFile() {
    Serial.print("Appending data to file: TEST1.CSV");
    flashDrive.setFileName("TEST1.CSV");
    if (flashDrive.openFile() == ANSW_USB_INT_SUCCESS) {
        flashDrive.moveCursor(CURSOREND);

        char buffer[10];  // Adjust the size as needed

        for (int i = 0; i < 8; i++) {
            dtostrf(canData[i], 4, 2, buffer);  // Convert float to string with 2 decimal places
            flashDrive.writeFile(buffer, strlen(buffer));
            if (i < 7) {
                flashDrive.writeFile(",", 1);
            }
        }

        flashDrive.writeFile("\n", 1);  // Add a newline at the end of the row

        flashDrive.closeFile();
        Serial.println("Done Appending!");
    }
}

void LoRasend() {
  Serial.println("Sending packet");

  // Send the byte array via LoRa
  LoRa.beginPacket();
  LoRa.write(byteValue, sizeof(byteValue));
  LoRa.endPacket();

  if (LoRa.endPacket()) {
    Serial.println("Packet sent successfully");
  } else {
    Serial.println("Error sending packet");
  }

  delay(100);
}

void filter() {
    unsigned long packetId = CAN.packetId();
    unsigned long targetId = 0x12;

    if (packetId == targetId) {
        Serial.println("Data from Master 0x");
        Serial.println(packetId, HEX);
    } else {
        Serial.print("Data from CAN Node 0x");
        Serial.println(packetId, HEX);
    }
}

void canReceiver() {
    int packetSize = CAN.parsePacket();

    if (packetSize) {
        Serial.print("Received ");

        if (CAN.packetExtended()) {
            Serial.print("extended ");
        }

        if (CAN.packetRtr()) {
            Serial.print("RTR ");
        }

        unsigned long packetId = CAN.packetId();

        if (packetId) {
            if (!CAN.packetRtr()) {
                Serial.print(" and length ");
                Serial.println(packetSize);
                filter();

                for (int i = 0; i < 8; i++) {
                    byteValue[i] = CAN.read();
                    canData[i] = (float)byteValue[i] / 76;
                    Serial.println(canData[i], 2);
                }
                
            }
        } else {
            Serial.print(" and requested length ");
            Serial.println(CAN.packetDlc());
        }

        Serial.println();
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  pinMode(RTD_LED, OUTPUT);
  pinMode(pwm, OUTPUT);
  pinMode(testled, OUTPUT);
  pinMode(apps_led, OUTPUT);
  pinMode(bspd_led, OUTPUT);
  pinMode(apps1_pin, INPUT);
  pinMode(apps2_pin, INPUT);
  analogReadResolution(12); // Set ADC resolution to 12-bit
  PWM_init();
  mySerial.begin(9600);
    flashDrive.init();
    Serial.print("Initialized CH376");
    delay(1000);
    if (flashDrive.checkIntMessage()) {
        if (flashDrive.getDeviceStatus()) {
            Serial.println(F("Flash drive attached!"));
        } else {
            Serial.println(F("Flash drive detached!"));
        }
        delay(1000);
    }
    createAndWriteFile();

    Serial.println("LoRa Sender");
    SPI.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
    LoRa.setPins(HSPI_CS, RST, DI0);

    while (!LoRa.begin(433E6)) {
        Serial.println("LoRa initialization failed. Retrying...");
        delay(1000);
    }
    LoRa.setSyncWord(0xF1);
    Serial.println("LoRa Initializing Successful!");

    Serial.println("CAN Receiver/Receiver");
    CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);
    if (!CAN.begin(500E3)) {
        Serial.println("Starting CAN failed!");
        while (1);
    } else {
        Serial.println("CAN Initialized");
    }
    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
    delay(500); 

    //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
    xTaskCreatePinnedToCore(
                      Task2code,   /* Task function. */
                      "Task2",     /* name of task. */
                      10000,       /* Stack size of task */
                      NULL,        /* parameter of the task */
                      1,           /* priority of the task */
                      &Task2,      /* Task handle to keep track of created task */
                      1);          /* pin task to core 1 */
    delay(500); 
  
}
void loop() {
}

