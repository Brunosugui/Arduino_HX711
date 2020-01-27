#define MAX_SERIAL_STRING_SIZE          16
#define STR_CALIB                        "c"
#define STR_WEIGHT                       "p"
#define STR_QUIT                         "q"
#define STR_ZERO                         "z"
#define STR_CLEAR                        "c"
#define STR_CALIBZERO                    "c0"
#define STR_CALIBSTP1                    "c1"

// --- Declaração de funções externas ---
void fsm_set_state(app_fsm_state_t next_state);
app_fsm_state_t fsm_get_state(void);
void serial_print(char* format, ...);
void hx711_clearCalib();
void hx711_setZeroAD();
unsigned long hx711_getZeroAD();
void hx711_setStep1AD(int weightKG);
unsigned long hx711_getStep1AD();
void keyboard_serial_rcv();
void keyboard_serial_parse_received(char receive[], int len);

boolean is_valid_number(String str){
   
   for(byte i=0;i<str.length();i++)
   {
      if(!isDigit(str.charAt(i))) return false;
   }
   return true;
} 

// parse de strings 
void keyboard_serial_parse_received(String receive)
{
  if (STATE_CALIB == fsm_get_state())
  {
     if (receive.equals(STR_QUIT))
     {
        serial_print("Switching to Weight State.");
        fsm_set_state(STATE_WEIGHT);
     }
     else if (receive.equals(STR_CLEAR))
     {
      serial_print("Clear Calib data.");
        hx711_clearCalib();
     }
     else if (receive.equals(STR_ZERO))
     {
        hx711_setZeroAD();
        serial_print("Analog Value set as Zero Kg");
     }
     else if (receive.equals(STR_CALIBZERO))
     {
        serial_print("<Calib> : ZERO = %lu", hx711_getZeroAD());
     }
     else if (receive.equals(STR_CALIBSTP1))
     {
      serial_print("<Calib> : STEP1 = %lu", hx711_getStep1AD());
     }
     else
     {
        if (is_valid_number(receive))
        {
          serial_print("Calibrating with %d kg", receive.toInt());
          hx711_setStep1AD(receive.toInt());
        }
        else
        {
          serial_print("Invalid Number %s", receive);
        }
     }
  }
  else
  {
    if (receive.equals(STR_CALIB))
    {
      fsm_set_state(STATE_CALIB);
    }
    else if (receive.equals(STR_WEIGHT))
    {
      fsm_set_state(STATE_WEIGHT);
    }    
  }
}

void keyboard_serial_rcv(void)
{
  String receive;
  int i = 0;
  
  if (Serial.available() > 0)
  {
    receive = Serial.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(&receive[0]);
    keyboard_serial_parse_received(&receive[0]);
  }
}

void serial_print(char* format, ...)
{
  va_list args;
  char s_buffer[32];
  
  va_start(args, format);
  vsprintf(&s_buffer[0], format, args);

  Serial.println(&s_buffer[0]);

  va_end(args);
}

void keyboard_procedure(void)
{
  keyboard_serial_rcv();
}
