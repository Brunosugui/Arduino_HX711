
#define PULSES_GAIN_NONE            0
#define PULSES_GAIN128              1      // CHANNEL A
#define PULSES_GAIN32               2      // CHANNEL B
#define PULSES_GAIN64               3      // CHANNEL A

#define APP_GAIN                    PULSES_GAIN64

#define BUFFER_SIZE_0               64
#define BUFFER_SIZE_1               32
#define BUFFER_SIZE_2               16

#define STEP_FILTER_ADC_THRES       0x400
#define STEP_FILTER_COUNTER_THRES   16


// --- Declaração de variáveis globais ---
unsigned long buffer_0[BUFFER_SIZE_0] = {0};
unsigned char buffer_0_idx = 0;
unsigned char buffer_0_counter = 0; 
unsigned long buffer_1[BUFFER_SIZE_1] = {0};
unsigned int buffer_1_idx = 0;
unsigned char buffer_1_counter = 0;
unsigned long buffer_2[BUFFER_SIZE_2] = {0};
unsigned int buffer_2_idx = 0;
unsigned char buffer_2_counter = 0;

unsigned long buffer_avg = 0;
unsigned long last_buffer_avg = 0;

unsigned char counter_high_step = 0;
unsigned char counter_low_step = 0;

unsigned long zeroAD = 2299695;
unsigned long step1AD = 1214135;
unsigned long step1Weight = 2000;

unsigned long hx711_read()
{
  unsigned long Count = 0;
  unsigned char i;
  
  digitalWrite(ADSK, LOW);
  
  while(digitalRead(ADDO));
  
  for(i=24 + APP_GAIN ;i;i--)
  {
     digitalWrite(ADSK, HIGH);
     Count = Count << 1;
     digitalWrite(ADSK, LOW);
     if(digitalRead(ADDO)) Count++;
  
  } //end for

  return(Count);
} //end ReadCount

unsigned long get_average(unsigned long vector[], unsigned char count)
{
  unsigned long long sum = 0;
  unsigned char i = 0;
  for (i = 0; i < count; i++)
  {
    sum += vector[i];
  }
  
  if(sum >= 0) {
      sum += (count / 2);    // Soma meio dígito (arredondamento).
  }
  else {
      sum -= (count / 2);    // Soma meio dígito (arredondamento).
  }
  
  return (unsigned long) sum/i;
}

unsigned long hx711_first_filter(unsigned long adc)
{
  if (buffer_0_counter < BUFFER_SIZE_0)
  {
    buffer_0_counter++;
  }

  buffer_0[buffer_0_idx] = adc;
  
  if (++buffer_0_idx >= BUFFER_SIZE_0)
  {
    buffer_0_idx = 0;
  }
  return get_average(&buffer_0[0], buffer_0_counter);
}

unsigned long hx711_second_filter(unsigned long adc)
{
  if (buffer_1_counter < BUFFER_SIZE_1)
  {
    buffer_1_counter++;
  }

  buffer_1[buffer_1_idx] = adc;
  
  if (++buffer_1_idx >= BUFFER_SIZE_1)
  {
    buffer_1_idx = 0;
  }
  return get_average(&buffer_1[0], buffer_1_counter);
}

void hx711_third_filter(unsigned long adc)
{
  if (buffer_2_counter < BUFFER_SIZE_2)
  {
    buffer_2_counter++;
  }

  buffer_2[buffer_2_idx] = adc;
  
  if (++buffer_2_idx >= BUFFER_SIZE_2)
  {
    buffer_2_idx = 0;
  }
  buffer_avg = get_average(&buffer_2[0], buffer_2_counter);
}

void hx711_reset_weight_counters(void)
{
  buffer_0_idx = 0;
  buffer_0_counter = 0; 
  buffer_1_idx = 0;
  buffer_1_counter = 0;
  buffer_2_idx = 0;
  buffer_2_counter = 0;

  Serial.println("Counters Reset!");
}

void hx711_step_filter(unsigned long cur_adc, unsigned long last_adc)
{
  if(cur_adc > last_adc) {
        if((cur_adc - last_adc) > STEP_FILTER_ADC_THRES) {
            counter_low_step = 0;

            if(++counter_high_step >= STEP_FILTER_COUNTER_THRES) {
                counter_high_step = 0;
                counter_low_step = 0;
                hx711_reset_weight_counters();
            }
        }
        else {
            if(counter_high_step) {
                counter_high_step--;
            }

            if(counter_low_step) {
                counter_low_step--;
            }
        }
    }
    else if(cur_adc < last_adc) {
        if((last_adc - cur_adc) > STEP_FILTER_ADC_THRES) {
            counter_high_step = 0;

            if(++counter_low_step >= STEP_FILTER_COUNTER_THRES) {
                counter_high_step = 0;
                counter_low_step = 0;
                hx711_reset_weight_counters();
            }
        }
        else {
            if(counter_high_step) {
                counter_high_step--;
            }

            if(counter_low_step) {
                counter_low_step--;
            }
        }
    }
    else {
        counter_high_step = 0;
        counter_low_step = 0;
    }
}

unsigned long hx711_procedure(void)
{
  unsigned long adc_read = hx711_read();
  unsigned long aux = 0;

  hx711_step_filter(adc_read, buffer_avg);

  aux = hx711_first_filter(adc_read);
  aux = hx711_second_filter(aux);
  hx711_third_filter(aux);
  
//  return buffer_avg;
  return aux;
}


void hx711_setZeroAD()
{
  zeroAD = buffer_avg;
}

unsigned long hx711_getZeroAD()
{
  return zeroAD;
}

void hx711_setStep1AD(int weightKG)
{
  step1AD = buffer_avg - zeroAD;
  step1Weight = weightKG*1000;
}

unsigned long hx711_getStep1AD()
{
  return step1AD;
}

void hx711_clearCalib()
{
  zeroAD = 0;
  step1AD = 0;
}

bool is_calib_done()
{
  return (0 != zeroAD && 0 != step1AD && 0 != step1Weight);
}

unsigned long hx711_get_weight(unsigned long analog)
{
  float fweight = 0;
  if (!is_calib_done())
  {
    return 0;
  }
  else
  {
    fweight = analog - zeroAD;
    if (fweight < 0) return 0;
    fweight = fweight/step1AD;
    fweight = fweight*step1Weight;
    return round(fweight);
  }
}

unsigned long hx711_weight_procedure(unsigned long analog)
{
  unsigned long weight;

  weight = hx711_get_weight(analog);

  //todo weight step 2

  return weight;
}
