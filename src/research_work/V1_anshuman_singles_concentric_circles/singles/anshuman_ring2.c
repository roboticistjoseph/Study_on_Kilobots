struct GLOBALS
{
  // global variables 
  // NOTE: the use of a GLOBALS struct will also work on a normal kilobot, 
  //       but is not required on a normal kilobot.
  //       It is, however, required by the simulator.

  uint16_t wait_time;
  int state;

  int distance_array[2];
  int new_message;
  int avg_distance;
  int previous_avg_distance;
  message_t outgoing_message;
  int array_counter;
  int steady_counter;
  int last_changed;
  int distance;
  int previous_dist;
  int last_changed_2;
  int timer;
  int new_message_from_same = 0;
  int distance_from_same = 0;
  int stopped;

  int message_delay;
  int message_delay_timer;

  int distance_from_layer_1;
  int turn_away_flag;

  message_t steady;

}* g;



void setup()
{
  g->wait_time = 2000;
  g->state = 0;
  g->distance_array[0]=0;
  g->distance_array[1]=0;
  g->new_message=0;
  g->avg_distance=0;
  g->previous_avg_distance=0;

  g->outgoing_message.type = NORMAL;
  g->outgoing_message.data[0] = 2; //This is our ring number 
  g->outgoing_message.data[1] = 0; //stopping flag
  g->outgoing_message.crc = message_crc(&g->outgoing_message);
  
  g->array_counter=0;
  g->steady_counter=0;
  g->last_changed = kilo_ticks;
  g->distance=10000;
  g->previous_dist=0;
  g->last_changed_2=0;
  g->timer=0;
  g->new_message_from_same = 0;
  g->distance_from_same = 0;
  g->stopped=0;
  g->message_delay=0;
  g->message_delay_timer=0;
  g->distance_from_layer_1=0;
  g->turn_away_flag=0;

}



void loop()
{
   set_color(RGB(1, 0, 1));
   if (g->new_message==1)
   {

      g->new_message=0;
       

      if (g->turn_away_flag==1)
      {
          g->turn_away_flag=0;
          set_motors(0, 0);
          g->message_delay=1;
          set_motors( kilo_straight_left, 0);
          delay(4400); 
          set_motors(0, 0);
          set_motors( kilo_straight_left, kilo_straight_right);
          delay(3000);
      }


       if (g->distance_from_layer_1==27||g->distance_from_layer_1==28||g->distance_from_layer_1==29||g->distance_from_layer_1==30)
       {
           set_motors(0,0);
           set_color(RGB(0, 0, 1));
           g->outgoing_message.data[1]=2; // layer 2 robot has stopped 
           g->steady_counter=1;
       }

       else if(g->distance_from_layer_1<27)
       {
          //g->turn_away_flag=0;
          set_motors(0, 0);
          //g->message_delay=1;
          set_motors( kilo_straight_left, 0);
          delay(300); 
          set_motors(0, 0);
          set_motors( kilo_straight_left, kilo_straight_right);
          delay(300);
       }

    //    else if(g->distance>30 && g->distance_from_layer_1<g->previous_dist)
    //    {

    //        g->steady_counter=0;
    //        g->outgoing_message.data[1]=0;
    //        set_color(RGB(1, 0, 0));
    //        set_motors(kilo_straight_left, kilo_straight_right);
    //        delay(500);
    //        set_motors(0,0);
    //        delay(500);
    //    }

    //    else if (g->distance_from_layer_1>g->previous_dist) //moving far 
    //    {
    //        g->steady_counter=0;
    //        g->outgoing_message.data[1]=0;
    //        set_color(RGB(0, 1, 0));
    //        set_motors(0, kilo_straight_right);
    //        delay(500);
    //        set_motors(0,0);
    //        delay(1000);
    //    }

       g->previous_dist=g->distance_from_layer_1;
   }

   else if(kilo_ticks-g->timer>32*10 || kilo_ticks==32*30)  // if the robot hasnt recieved message in  while
   {
       g->steady_counter=0;
       g->outgoing_message.data[1]=0;
        set_motors(kilo_straight_left, kilo_straight_right);
        delay(500);
        set_motors(0,0);
        delay(500);

        if (kilo_ticks > g->last_changed + 32*30)
        {
            g->last_changed=kilo_ticks;
            set_motors(kilo_straight_left,0);
            delay(3400);
        }
   }

//    if (g->new_message_from_same == 1 && g->steady_counter==0 && g->distance_from_same>0 && g->distance>28)
//    {
//        g->new_message_from_same = 0;
//        g->outgoing_message.data[1]=0;
//         set_color(RGB(1, 1, 0));
//         set_motors(kilo_straight_left, kilo_straight_right);
//         delay(500);
//         set_motors(0,0);
//         delay(500);
//         set_motors(kilo_straight_left, 0);
//         delay(300);
//         set_motors(0,0);
//         delay(300);
//    }
}


void message_rx(message_t *m, distance_measurement_t *d)
{
    if(m->data[0] == 0) // then the message was from the center robot
    {
        g->new_message = 1;
        g->turn_away_flag=1;
    }

    // we get message from the same ring number robot as current robot (m->data[0] == 1)
    // and m->data[1]==1 states that the message we are getting from 
    // is from a robot that is already stopped (BLUE) within the defined radius

    if(m->data[0] == 1 && m->data[1]==1) // then the message was from the layer 1 robot
    {
        g->new_message = 1;
        g->timer=kilo_ticks;
        g->distance_from_layer_1 = estimate_distance(d);
    }


}

message_t *message_tx()
{
    return &(g->outgoing_message);
    //return &(g->steady);
}

void message_tx_success()
{
  // do nothing
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
