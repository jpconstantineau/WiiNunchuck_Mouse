#include <bluefruit.h>
#include "Adafruit_TinyUSB.h"
#include <NintendoExtensionCtrl.h>

BLEDis bledis;
BLEHidAdafruit blehid;
BLEBas blebas;

Adafruit_USBD_HID usb_hid;
extern Adafruit_USBD_Device USBDevice;

Nunchuk nchuk;


#define MOUSE_SENSITIVITY 4  //number of bits to shift to decrease joystick sensitivity
#define MOUSE_ZERO 2         // send 0 if x or y is less than MOUSE_ZERO

uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};



uint8_t mvToPercent( uint32_t mvolts)
{
  uint8_t vbat_per = 0;
  if (mvolts < 3600) {
    mvolts -= 3300;
    vbat_per = mvolts / 30;
  }
  mvolts -= 3600;
  vbat_per = (uint8_t) 10 + (uint8_t)((mvolts * 15) / 100); // thats mvolts /6.66666666
  if (vbat_per > 100) {
    vbat_per = 100; // checks if we are higher than 100%. when this is the case windows doesn't show anything...
  }
  return vbat_per;
}

uint32_t readVBAT(uint8_t pin) {
  #define VBAT_MV_PER_LSB   (0.73242188F) 
  #define VBAT_DIVIDER_COMP (1.403F)
  analogReference(AR_INTERNAL_3_0); // Set the analog reference to 3.0V (default = 3.6V)
  analogReadResolution(12);         // Set the resolution to 12-bit (0..4095) // Can be 8, 10, 12 or 14
  delay(1);                         // Let the ADC settle
  uint32_t vbat_mv = analogRead(pin) * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP;        // Get the raw 12-bit, 0..3000mV ADC value
  analogReference(AR_DEFAULT);      // Set the ADC back to the default settings - just in case we use it somewhere else
  analogReadResolution(10);         // Set the ADC back to the default settings - just in case we use it somewhere else
  return vbat_mv;
};

void setup() {

  // USB
  USBDevice.setManufacturerDescriptor("nrf52.jpconstantineau.com");
  USBDevice.setProductDescriptor("Nunchuck");
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  // BLE
  Bluefruit.begin();
  Bluefruit.Periph.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(0);
  Bluefruit.setName("Nunchuck_BLE");
  bledis.setManufacturer("nrf52.jpconstantineau.com");
  bledis.setModel("Nunchuck Mouse");
  bledis.begin();
  blehid.begin();
  blebas.begin();
  blebas.write(42);

  // Nunchuck
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
  // poll rate = 10ms 
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
    int buttons = ((cButton) ? MOUSE_BUTTON_LEFT : 0) | ((zButton) ? MOUSE_BUTTON_RIGHT : 0);
    //variable = (condition) ? expressionTrue : expressionFalse;

    int deltax = (nchuk.joyX() - 128) >> MOUSE_SENSITIVITY;
    int deltay = -1 * (nchuk.joyY() - 128) >> MOUSE_SENSITIVITY;
    if (abs(deltax) < MOUSE_ZERO) deltax = 0;
    if (abs(deltay) < MOUSE_ZERO) deltay = 0;


    if (usb_hid.ready() && !USBDevice.suspended())  // USB Connected
    {
      if (Bluefruit.connected())
      {
        //  Bluefruit.disconnect();
        Bluefruit.Advertising.stop();
      }
      usb_hid.mouseReport(0, buttons, deltax, deltay, 0, 0);
    }
    else if (Bluefruit.connected()) // BLE Connected
    {
      blehid.mouseReport(0, buttons, deltax, deltay, 0, 0);
      blebas.notify(mvToPercent(readVBAT(31)));  // 31 is the GPIO for the BlueMicro840 // not sure why this doesn't seem to update from initial value

    }
    else  // not connected...
    {
      if ( USBDevice.suspended() )  // Remote wakeup
      {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
      }
      if ( !USBDevice.mounted() )
      {
        //restart BLE
        startAdv();
      }
    }
  }







}
