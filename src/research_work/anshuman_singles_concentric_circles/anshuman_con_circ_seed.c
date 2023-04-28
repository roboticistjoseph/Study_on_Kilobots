#define NUMBER_OF_ROBOTS 20


struct GLOBALS
{
  // global variables 
  // NOTE: the use of a GLOBALS struct will also work on a normal kilobot, 
  //       but is not required on a normal kilobot.
  //       It is, however, required by the simulator.

  message_t message;

  // Flag to keep track of message transmission.
  int message_sent;
  int stop;
  int data_from;

  int stopped_robots_array[NUMBER_OF_ROBOTS];

  int time_array[NUMBER_OF_ROBOTS];
}* g; // there should only be one GLOBAL, this is it, remember to register it in main()



void setup()
{
  // Initialize message 
  // (note that because we are never changing the data in the message in this example
  // then we only need to do this once, otherwise we'd need to do it every time we 
  // wanted to send a new message)

  // The type is always NORMAL.
  g->message.type = NORMAL;

  // indicate we are a star
  g->message.data[0] = 0;
  g->message.data[1]=0; // this will send message to planet robots that star has 12 neighbors

  // It's important that the CRC is computed after the data has been set;
  // otherwise it would be wrong.
  g->message.crc = message_crc(&g->message);

  g->stop=0;
  g->data_from=-1;

  int i; 
  for(i = 0; i <=NUMBER_OF_ROBOTS-1; i++)
  {
    g->stopped_robots_array[i]=0;  // init to a large number
    g->time_array[i]=0;
  }
}

void loop()
{
  set_color(RGB(1, 0, 1));

  // Blink LED magenta whenever a message is sent.
  if (g->message_sent == 1)
  {
      // Reset flag so LED is only blinked once per message.
      g->message_sent = 0;
      
      set_color(RGB(1, 0, 1));
      delay(1000);
      set_color(RGB(0, 0, 0));
  }

  int i;
  int n_counter=0;
  for (i=0;i<=NUMBER_OF_ROBOTS-1;i++)
  {
    // here we are checking if the we heard from a robot in the last 40 ticks 
    // if not heard then that robot must not be in the stopped radius
    // hence set its value in the array = 0 so we can only count the number of 
    // robots which are stopped.
    if (kilo_ticks-g->time_array[i]>40) 
    {
      g->stopped_robots_array[i]=0;
    }

    if (g->stopped_robots_array[i]==1)
    {
      n_counter++;
    }
  }

  // if (g->stop==1)
  // {
  //   log_message("Robot %d says Im a stopped",g->data_from);
  // }

  //log_message("Stopped : %d ",n_counter);

  // if (n_counter>=12)
  // {
  //   g->message.data[1]=1; // desirable number of neighbors reached; tell this to other planet robots
  // }
  // else
  // {
  //   g->message.data[1]=0;
  // }
  //log_message("Im a star");
  g->message.data[1]=n_counter;
  delay(100);
}


void message_rx(message_t *m, distance_measurement_t *d)
{
  if(m->data[0] == 1) // gettting msg from planet robot 
  {
    if (m->data[1] == 1) // if the planet robot is stopped
    {
      g->time_array[m->data[2]]=kilo_ticks; // set the time when the planet robot stopped

      g->stop=1;                 // flag to be to tell star that robot stopped with having      
      g->data_from=m->data[2];  //kilo uid equal to this 

      g->stopped_robots_array[m->data[2]]=1; // setting value of corresponging uid robot in array to 1 
    }
    else if (m->data[1] == 0) 
    {
      g->stopped_robots_array[m->data[2]]=0;
    } 
  }
}

message_t *message_tx()
{
    //log_message_from_sim("|| S  data[0]: %u", g->message.data[0]);

    return &(g->message);
}


void message_tx_success()
{
    // Set flag on message transmission.
    g->message_sent = 1;
}


int main()
{
    GLOBALS* g_safe =  (GLOBALS*)malloc(sizeof(GLOBALS));

    #ifdef USING_SIMULATION
      // register the global variables (only necessary on simulation)
      //g is a pointer
      //so the address of g is a pointer to a pointer,the address of it is getting casted to a pointer to a void pointer
   
      kilo_globals = (void**)&g;
    #endif

    kilo_init();


    // Register the message_rx callback function.
    kilo_message_rx = message_rx;

    // Register the message_tx callback function.
    kilo_message_tx = message_tx;

    // Register the message_tx_success callback function.
    kilo_message_tx_success = message_tx_success;

    g = g_safe;

    #ifdef USING_SIMULATION
      kilo_start(setup, loop, message_rx, message_tx, message_tx_success);
    #else
      kilo_start(setup, loop);
      // free user defined globals
      free(g_safe);
    #endif
  
  
    return 0;
}

