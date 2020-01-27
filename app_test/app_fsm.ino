// --- declaração de estruturas
typedef enum{
  STATE_START,
  STATE_CALIB,
  STATE_WEIGHT,
  STATE_IDLE,
}app_fsm_state_t;

typedef enum{
  CALIB_ZERO_STATE,
  CALIB_FIRST_STEP_STATE,
}app_fsm_substate_calib_t;

typedef enum{
  WEIGHT_FIRST_STEP_STATE,
  WEIGHT_SECOND_STEP_STATE,
}app_fsm_substate_weight_t;

typedef union{
  app_fsm_substate_calib_t calib_substate;
  app_fsm_substate_weight_t weight_substate;
}app_fsm_substate_t;

typedef struct{
  app_fsm_state_t state;
  app_fsm_substate_t substate;
}app_fsm_data_t;

typedef app_fsm_state_t (*app_fsm_function)(app_fsm_data_t*);

typedef struct{
  app_fsm_state_t state;
  app_fsm_function function;
}app_fsm_struct_table_t;

void serial_print(char* format, ...);
//unsigned long hx711_get_weight(unsigned long analog);
bool is_calib_done();
unsigned long hx711_procedure(void);
unsigned long hx711_weight_procedure(unsigned long analog);
app_fsm_state_t fsm_run(app_fsm_state_t state);
app_fsm_state_t fsm_state_weight(app_fsm_data_t *data);
app_fsm_state_t fsm_state_calib(app_fsm_data_t *data);
app_fsm_state_t fsm_state_start(app_fsm_data_t *data);

static const app_fsm_struct_table_t fsm_struct[] = {
  {     .state = STATE_START,         .function = fsm_state_start       },
  {     .state = STATE_CALIB,         .function = fsm_state_calib       },
  {     .state = STATE_WEIGHT,        .function = fsm_state_weight      },
  {     .state = STATE_IDLE,          .function = fsm_state_start       },
};

app_fsm_state_t state = STATE_START;
static app_fsm_data_t data = {
  .state = STATE_START,
};

void fsm_procedure(void)
{
  state = fsm_run(state);
}

app_fsm_state_t fsm_state_weight(app_fsm_data_t *data)
{
  app_fsm_state_t next_state = STATE_WEIGHT;
  unsigned long weight;

  unsigned long convert = hx711_procedure();

  if (is_calib_done())
  {
    weight = hx711_weight_procedure(convert);
    serial_print("<Weight> : %lu g", weight);
  }
  else
  {
    serial_print("<Weight> : AD = %lu", convert);
  }

  return next_state;
}

app_fsm_state_t fsm_state_calib(app_fsm_data_t *data)
{
  app_fsm_state_t next_state = STATE_CALIB;

  unsigned long convert = hx711_procedure();

//  Serial.print("<Calib> : AD = ");
//  Serial.println(convert);
  serial_print("<Calib> : AD = %lu", convert);

  return next_state;
}

app_fsm_state_t fsm_state_start(app_fsm_data_t *data)
{
  app_fsm_state_t next_state = STATE_WEIGHT;

  Serial.println("Program Started!");
  
  return next_state;
}


app_fsm_state_t fsm_run(app_fsm_state_t state)
{
  app_fsm_state_t next_state = STATE_IDLE;
  for (int i = 0; i < sizeof(fsm_struct)/sizeof(app_fsm_struct_table_t)-1; i++)
  {
    if (state == fsm_struct[i].state)
    {
      if (fsm_struct[i].function)
      {
        next_state = fsm_struct[i].function(&data);
      }
      return next_state;
    }
  }
  return next_state;
}

void fsm_set_state(app_fsm_state_t next_state)
{
  state = next_state;
}

app_fsm_state_t fsm_get_state(void)
{
  return state;
}
