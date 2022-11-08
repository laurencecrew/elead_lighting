
#include <Arduino.h>
#include "pins.h"
#include "debug.h"
#include "can.h"
#include "can_lights.h"

// function prototypes
void update_lights (LIGHTS_Data_t*);

void setup ()
{

  // Initialise all pins
  pinMode (TAIL_OUT, OUTPUT);
  pinMode (BRAKE_OUT, OUTPUT);
  pinMode (TURN_L_OUT, OUTPUT);
  pinMode (TURN_R_OUT, OUTPUT);
 
  digitalWrite (TAIL_OUT, LOW);
  digitalWrite (BRAKE_OUT, LOW);
  digitalWrite (TURN_L_OUT, LOW);
  digitalWrite (TURN_R_OUT, LOW);
  
  #ifdef DEBUG
    DebugSerial.begin (115200);
    DebugSerial.println ("Init CAN ..");
  #endif

  // initialise CAN
  bool ret = CANInit(CAN_250KBPS, CAN_REMAP);  // CAN_RX mapped to PB8, CAN_TX mapped to PB9
  if (!ret) while(true);

  #ifdef DEBUG
    DebugSerial.println ("Done!");
  #endif

  // Configure CAN filters
  CAN1->FMR  |=   0x1UL;                // Set to filter initialization mode
 
 /*
 CANSetFilter:
 * @preconditions   - This register can be written only when the filter initialization mode is set (FINIT=1) in the CAN_FMR register.
 * @params: index   - Specified filter index. index 27:14 are available in connectivity line devices only.
 * @params: scale   - Select filter scale.
 *                    0: Dual 16-bit scale configuration
 *                    1: Single 32-bit scale configuration
 * @params: mode    - Select filter mode.
 *                    0: Two 32-bit registers of filter bank x are in Identifier Mask mode
 *                    1: Two 32-bit registers of filter bank x are in Identifier List mode
 * @params: fifo    - Select filter assigned.
 *                    0: Filter assigned to FIFO 0
 *                    1: Filter assigned to FIFO 1
 * @params: bank1   - Filter bank register 1
 * @params: bank2   - Filter bank register 2
 */
  u_int32_t CAN_Filter_ID = LIGHTS_CAN_ID << 21;
  u_int32_t CAN_Filter_Mask = 0xFFE00006; // For STD ID Data frame, Must be IDE=0 RTR=0

  CANSetFilter(0, 1, 0, 0, CAN_Filter_ID, CAN_Filter_Mask); 
  
  CAN1->FMR   &= ~(0x1UL);              // Deactivate initialization mode

}


void loop()
{

  CAN_msg_t CAN_RX_msg;
  LIGHTS_Data_t LIGHTS_Data; // CAN data packet

  // Check for CAN data
  if(CANMsgAvail())
  {
    CANReceive(&CAN_RX_msg);

    if (CAN_RX_msg.id == LIGHTS_CAN_ID)
    {
      // get data from the CAN packet into the report structure
      memcpy(&LIGHTS_Data.data, CAN_RX_msg.data, sizeof LIGHTS_Data.data);

      #ifdef DEBUG
        DebugSerial.print ("Lights data: ");
        DebugSerial.println (LIGHTS_Data.data, HEX);
      #endif

      update_lights (&LIGHTS_Data);

    }

  
  #ifdef DEBUG
    else
    {

      if (CAN_RX_msg.format == EXTENDED_FORMAT) {
        DebugSerial.print("Extended ID: 0x");
        if (CAN_RX_msg.id < 0x10000000) DebugSerial.print("0");
        if (CAN_RX_msg.id < 0x1000000) DebugSerial.print("00");
        if (CAN_RX_msg.id < 0x100000) DebugSerial.print("000");
        if (CAN_RX_msg.id < 0x10000) DebugSerial.print("0000");
        DebugSerial.print(CAN_RX_msg.id, HEX);
      } else {
        DebugSerial.print("Standard ID: 0x");
        if (CAN_RX_msg.id < 0x100) DebugSerial.print("0");
        if (CAN_RX_msg.id < 0x10) DebugSerial.print("00");
        DebugSerial.print(CAN_RX_msg.id, HEX);
        DebugSerial.print("     ");
      }

      DebugSerial.print(" DLC: ");
      DebugSerial.print(CAN_RX_msg.len);
      if (CAN_RX_msg.type == DATA_FRAME) {
        DebugSerial.print(" Data: ");
        for(int i=0; i<CAN_RX_msg.len; i++) {
          DebugSerial.print("0x"); 
          DebugSerial.print(CAN_RX_msg.data[i], HEX); 
          if (i != (CAN_RX_msg.len-1))  DebugSerial.print(" ");
        }
        DebugSerial.println();
      } else {
        DebugSerial.println(" Data: REMOTE REQUEST FRAME");
      }
    }
  #endif
  }
}

void update_lights (LIGHTS_Data_t *ldata)
{
  digitalWrite (BRAKE_OUT, ldata->bits.brake);
  digitalWrite (TAIL_OUT, ldata->bits.tail);
  digitalWrite (TURN_L_OUT, ldata->bits.turn_l);
  digitalWrite (TURN_R_OUT, ldata->bits.turn_r);
}


/* Spare stuff */


  /*
    switch (CAN_RX_msg.id)
    {
      case CAN_ID_INDICATOR:

        switch (CAN_RX_msg.data[0])
        {
          case CAN_DATA_OFF:

            if (state == STATE_BLINK)
            {
              // stop blinking
              digitalWrite (LED_PIN, LED_OFF);
              state = STATE_OFF;
              TMR_ClearInterval (); // cancel the timer
            }
            break;

          case CAN_DATA_BLINK:

            if (state == STATE_OFF)
            {
              // start blinking
              digitalWrite (LED_PIN, LED_ON);
              state = STATE_BLINK;
              TMR_SetInterval (blink, T_BLINK);
            }
            break;
        }

        break;

      default:
        break;

    }
  }
  */
    // CAN_RX_msg.len
    // (CAN_RX_msg.type == DATA_FRAME

