#include "Adafruit_TinyUSB.h"
#include <NintendoExtensionCtrl.h>


#define MOUSE_SENSITIVITY 4  //number of bits to shift to decrease joystick sensitivity
#define MOUSE_ZERO 3         // send 0 if x or y is less than MOUSE_ZERO

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};

// USB HID object
Adafruit_USBD_HID usb_hid;
Nunchuk nchuk;


// the setup function runs once when you press reset or power the board
void setup()
{
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();
  nchuk.begin();
  
  Serial.begin(115200);

  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);

  // wait until nunchuck mounted
  while (!nchuk.connect()) {
    Serial.println("Nunchuk not detected!");
    delay(1000);
  }
}

void loop()
{
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
    
    if ( USBDevice.suspended() )  // Remote wakeup
    {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    USBDevice.remoteWakeup();
    }

    if ( usb_hid.ready() )
    {
      usb_hid.mouseReport(0,buttons,deltax, deltay, 0,0);
    }
  }
}
