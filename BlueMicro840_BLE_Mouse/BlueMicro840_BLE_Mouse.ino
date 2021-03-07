#include <bluefruit.h>
#include <NintendoExtensionCtrl.h>

BLEDis bledis;
BLEHidAdafruit blehid;
Nunchuk nchuk;

#define MOUSE_SENSITIVITY 4  //number of bits to shift to decrease joystick sensitivity
#define MOUSE_ZERO 2         // send 0 if x or y is less than MOUSE_ZERO

void setup() {
 Bluefruit.begin();
  // HID Device can have a min connection interval of 9*1.25 = 11.25 ms
  Bluefruit.Periph.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(0);   
  Bluefruit.setName("Nunchuck");

  // Configure and Start Device Information Service
  bledis.setManufacturer("nrf52.jpconstantineau.com");
  bledis.setModel("Nunchuck Mouse");
  bledis.begin();

  // BLE HID
  blehid.begin();
  nchuk.begin();
  
  // wait until nunchuck mounted
  while (!nchuk.connect()) {
    Serial.println("Nunchuk not detected!");
    delay(1000);
  }

  // Set up and start advertising
  startAdv();

}


void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for 'Name' in the advertising packet
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop() {
  // poll
  delay(10);

   boolean success = nchuk.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Controller disconnected!");
    delay(1000);
  }
  else {
    // Read a button (on/off, C and Z)
    boolean zButton = nchuk.buttonZ();
    boolean cButton = nchuk.buttonC();
    int buttons = ((cButton)?MOUSE_BUTTON_LEFT:0)|((zButton)?MOUSE_BUTTON_RIGHT:0);
    //variable = (condition) ? expressionTrue : expressionFalse;
             
    int deltax = (nchuk.joyX()-128)>>MOUSE_SENSITIVITY;
    int deltay = -1*(nchuk.joyY()-128)>>MOUSE_SENSITIVITY;
    if (abs(deltax)<MOUSE_ZERO) deltax = 0;
    if (abs(deltay)<MOUSE_ZERO) deltay = 0;
    
      blehid.mouseReport(0,buttons,deltax, deltay, 0,0);

  }

}
