# STM32 F103C8T6 Blue Pill as a HID Keyboard
This page contains the code and supporting libraries for the below video. 
It is a video demonstrating how to use the blue pill as a custom HID keyboard device and demonstrates its uses.

If you use this code, please link back to my YouTube channel.

**Video link:**

[![Youtube Video](https://img.youtube.com/vi/8i2fl8YVwuI/0.jpg)](https://www.youtube.com/watch?v=8i2fl8YVwuI)

# Code changes:
Below changes to be made in usbd_hid.c file.

* Location: Middlewares > ST > STM32_USB_Device_Library > Class > HID > Src
* Open the file and find the array of USB HID device FS Configuration Descriptor, in that you will find one parameter that is **nInterfaceProtocol**.
* Default value of the parameter is 0x02 which is for mouse, you have to replace it with 0x01 for keyboard.
* Find the array HID_MOUSE_ReportDesc. This array contains the descriptor for a mouse, therefore we have to change its value with a Keyboard Report Descriptor as shown below.
```
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION

};
```
**Below changes to be made in usbd_hid.h file.**

* Location: Middlewares > ST > STM32_USB_Device_Library > Class > HID > Inc
* Find HID_MOUSE_REPORT_DESC_SIZE. It will be 74U. Change it to 63U.
```
#define HID_MOUSE_REPORT_DESC_SIZE    74U
```

Below are the code custom variables and functions I have made for the project. The main code is available in the project.
```
extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct
{
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
	uint8_t KEYCODE6;
}subKeyBoard;

subKeyBoard keyBoardHIDsub = {0,0,0,0,0,0,0,0};
```

Function to send a series of keystrokes as a string or char array easily. Add keys codes if missing.
```
void typeOnKeyboard(char cmd[]){

	for(int i = 0; i < strlen(cmd); i++) {
		char charToType = cmd[i];
		uint8_t KEYCODE = 0x00;
		uint8_t MODIFIER = 0x00;

		switch(charToType) {
			case 'A': KEYCODE = 0x04; MODIFIER=0x02; break;
		    case 'B': KEYCODE = 0x05; MODIFIER=0x02; break;
		    case 'C': KEYCODE = 0x06; MODIFIER=0x02; break;
		    case 'D': KEYCODE = 0x07; MODIFIER=0x02; break;
		    case 'E': KEYCODE = 0x08; MODIFIER=0x02; break;
		    case 'F': KEYCODE = 0x09; MODIFIER=0x02; break;
		    case 'G': KEYCODE = 0x0A; MODIFIER=0x02; break;
		    case 'H': KEYCODE = 0x0B; MODIFIER=0x02; break;
		    case 'I': KEYCODE = 0x0C; MODIFIER=0x02; break;
		    case 'J': KEYCODE = 0x0D; MODIFIER=0x02; break;
		    case 'K': KEYCODE = 0x0E; MODIFIER=0x02; break;
		    case 'L': KEYCODE = 0x0F; MODIFIER=0x02; break;
		    case 'M': KEYCODE = 0x10; MODIFIER=0x02; break;
		    case 'N': KEYCODE = 0x11; MODIFIER=0x02; break;
		    case 'O': KEYCODE = 0x12; MODIFIER=0x02; break;
		    case 'P': KEYCODE = 0x13; MODIFIER=0x02; break;
		    case 'Q': KEYCODE = 0x14; MODIFIER=0x02; break;
		    case 'R': KEYCODE = 0x15; MODIFIER=0x02; break;
		    case 'S': KEYCODE = 0x16; MODIFIER=0x02; break;
		    case 'T': KEYCODE = 0x17; MODIFIER=0x02; break;
		    case 'U': KEYCODE = 0x18; MODIFIER=0x02; break;
		    case 'V': KEYCODE = 0x19; MODIFIER=0x02; break;
		    case 'W': KEYCODE = 0x1A; MODIFIER=0x02; break;
		    case 'X': KEYCODE = 0x1B; MODIFIER=0x02; break;
		    case 'Y': KEYCODE = 0x1C; MODIFIER=0x02; break;
		    case 'Z': KEYCODE = 0x1D; MODIFIER=0x02; break;
		    case 'a': KEYCODE = 0x04; break;
		    case 'b': KEYCODE = 0x05; break;
		    case 'c': KEYCODE = 0x06; break;
		    case 'd': KEYCODE = 0x07; break;
		    case 'e': KEYCODE = 0x08; break;
		    case 'f': KEYCODE = 0x09; break;
		    case 'g': KEYCODE = 0x0A; break;
		    case 'h': KEYCODE = 0x0B; break;
		    case 'i': KEYCODE = 0x0C; break;
		    case 'j': KEYCODE = 0x0D; break;
		    case 'k': KEYCODE = 0x0E; break;
		    case 'l': KEYCODE = 0x0F; break;
		    case 'm': KEYCODE = 0x10; break;
		    case 'n': KEYCODE = 0x11; break;
		    case 'o': KEYCODE = 0x12; break;
		    case 'p': KEYCODE = 0x13; break;
		    case 'q': KEYCODE = 0x14; break;
		    case 'r': KEYCODE = 0x15; break;
		    case 's': KEYCODE = 0x16; break;
		    case 't': KEYCODE = 0x17; break;
		    case 'u': KEYCODE = 0x18; break;
		    case 'v': KEYCODE = 0x19; break;
		    case 'w': KEYCODE = 0x1A; break;
		    case 'x': KEYCODE = 0x1B; break;
		    case 'y': KEYCODE = 0x1C; break;
		    case 'z': KEYCODE = 0x1D; break;
		    case '1': KEYCODE = 0x1E; break;
		    case '2': KEYCODE = 0x1F; break;
		    case '3': KEYCODE = 0x20; break;
		    case '4': KEYCODE = 0x21; break;
		    case '5': KEYCODE = 0x22; break;
		    case '6': KEYCODE = 0x23; break;
		    case '7': KEYCODE = 0x24; break;
		    case '8': KEYCODE = 0x25; break;
		    case '9': KEYCODE = 0x26; break;
		    case '0': KEYCODE = 0x27; break;
		    case ' ': KEYCODE = 0x2C; break;
		    case '-': KEYCODE = 0x2D; break;
		    case ';': KEYCODE = 0x33; break; // semi-colon
		    case ':': KEYCODE = 0x33; MODIFIER=0x02; break; // colon
		    case '\'': KEYCODE = 0x34; break; // single quote
		    case '\"': KEYCODE = 0x34; MODIFIER=0x02; break; // double quote
		    case '/': KEYCODE = 0x38; break; // slash
		    case '.': KEYCODE = 0x37; break; // slash
		    default: KEYCODE = 0x00; break; // Default case for undefined characters
		}


		 keyBoardHIDsub.MODIFIER=MODIFIER;  // To press shift key
		 keyBoardHIDsub.KEYCODE1=KEYCODE;

		 USBD_HID_SendReport(&hUsbDeviceFS,&keyBoardHIDsub,sizeof(keyBoardHIDsub));
		 HAL_Delay(25);

		 keyBoardHIDsub.MODIFIER=0x00;
		 keyBoardHIDsub.KEYCODE1=0x00;

		 USBD_HID_SendReport(&hUsbDeviceFS,&keyBoardHIDsub,sizeof(keyBoardHIDsub));
		 HAL_Delay(25);
	}

}
```

Function to press and relase a key, for shortcuts.
```
void keyPressRelease(uint8_t MODIFIER, uint8_t KEYCODE){
     keyBoardHIDsub.MODIFIER=MODIFIER;  // To press shift key
	 keyBoardHIDsub.KEYCODE1=KEYCODE;

	 USBD_HID_SendReport(&hUsbDeviceFS,&keyBoardHIDsub,sizeof(keyBoardHIDsub));
	 HAL_Delay(25);

	 keyBoardHIDsub.MODIFIER=0x00;
	 keyBoardHIDsub.KEYCODE1=0x00;

	 USBD_HID_SendReport(&hUsbDeviceFS,&keyBoardHIDsub,sizeof(keyBoardHIDsub));
	 HAL_Delay(25);
}
```
