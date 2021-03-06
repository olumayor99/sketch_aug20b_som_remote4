
//LG Remote (NEC)
enum LgRemote {
    LG_UP = 0x20DF02FD,
    LG_DW = 0x20DF827D,
    LG_INFO = 0x20DF55AA,
    LG_SMPLINK = 0x20DF7E81,
    
    //enter some kind of menu
    LG_MENU = 0x20DFC23D,
    LG_QMENU = 0x20DFA25D,
    LG_GUIDE = 0x20DFD52A,
    LG_FAV = 0x20DF7887,
    LG_ENTER = 0x20DF22DD,
  
    LG_RETURN = 0x20DF14EB,
    LG_POWER = 0x20DF10EF,
   
    LG_LEFT = 0x20DFE01F,
    LG_RIGHT = 0x20DF609F,
};


//AIWA RC-EX08 (NEC)
enum AiwaRemote {
    AIWA_1 = 0xE078FC8,
    AIWA_2 = 0xE078FE8,
    AIWA_3 = 0xE078FF8,
    AIWA_4 = 0xE078FD8,
    AIWA_5 = 0xE078FD2,
    AIWA_6 = 0xE078FE2,
};

const int RepeatTimeout = 300; //ms

/** The UP and DOWN arrows are used to control the volume, but when the menu key is pressed,
    these keys are used to navigate the menu, this namespace is used to handle when a menu key is pressed
    and momentarly stops responding to the UP/DW keys to avoid conflict when navigating on menu.
*/
namespace lgMenu{
  
  const unsigned int timeoutLimit = 40000;
  
  static bool enabled = false;
  static long timeout;
  
  void onModeLocked(){
      viewModeLocked();
  }
  void onModeLockedOff(){
      viewModeNormal();
  }
  
  /** called on loop */
  void routine(){
    
      unsigned long currentMillis = millis();
    
      if(enabled){
          if(currentMillis - timeout > timeoutLimit){ //after this amount of time, the menu should have vanished.
                enabled = false; //return to respond to UP/DW key presses
                onModeLockedOff();
          }
      }
  }

  void processKey(unsigned long value, long currentMillis){
        
        //when these keys are pressed, they enter in some menu
        if(value == LG_MENU || value == LG_QMENU || value == LG_GUIDE || value == LG_FAV ){
            enabled = !enabled; //toggle
            timeout = currentMillis; //restart timeout countdown
            
            if(enabled){
                onModeLocked();
            }
            else{
                onModeLockedOff();
            }
        }
        
        //the LG_RETURN quit the menu so return to normal mode
        if(value == LG_RETURN){
            enabled = false;
            onModeLockedOff();
        }
        
        if(enabled){
            //when these keys are pressed (navigating on menu), they keep the keep menu opened
            if(value == LG_UP || value == LG_DW || value == LG_LEFT || value == LG_RIGHT || value == LG_ENTER ){ 
                timeout = currentMillis; //restart timeout countdown
            }
        }
    }
}




/** This function is called on every loop and handles and dispatch everything related to IR events */
byte processRemote(){

    static byte lastCommand;
    static long previousMillisR;
    
    byte command = CMD_NONE;
    
    //if(irrecv.decode(&results)) {
    if(irrecv.decodeOnlyNEC(&results)) {

        unsigned long currentMillisR = millis();

        #if USESERIAL==true
            Serial.print(F("\nCODE:"));
            Serial.print(results.value, HEX);
        #endif

        unsigned long value = results.value;

        lgMenu::processKey(value, currentMillisR);

        switch(value){
            case AIWA_3:
                command = CMD_VOLUP;
            break;

            case AIWA_5:
                command = CMD_VOLDW;
            break;

            case AIWA_6:
                command = CMD_MUTE;
            break;

            case LG_UP:
                command = lgMenu::enabled ? CMD_NONE : CMD_VOLUP;
            break;

            case LG_DW:
                command = lgMenu::enabled ? CMD_NONE : CMD_VOLDW;
            break;

            case LG_INFO:
                command = CMD_MUTE;
            break;

            case REPEAT:
                if(currentMillisR - previousMillisR < RepeatTimeout){
                    if(lastCommand == CMD_VOLUP || lastCommand == CMD_VOLDW){
                        command = lastCommand;
                    }
                }
            break;

            default:
                command = CMD_NONE;
            break;
        }

        irrecv.resume();

        previousMillisR = currentMillisR;
        lastCommand = command;
        
        if(command!=CMD_NONE) ledBlink::on();
    }

    lgMenu::routine();

    return command;
}

