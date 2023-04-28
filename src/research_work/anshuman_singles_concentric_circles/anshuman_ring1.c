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
  g->outgoing_message.data[0] = 1; //This is our ring number 
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

}

// void loop()
// {
//     if (g->new_message ==1)
//     {
//         if (g->distance_array[0]>20 && g->distance_array[0]<30||g->distance_array[1]>20 && g->distance_array[1]<30)
//         {
//             set_motors(0,0);
//             set_color(RGB(0, 0, 1));
//             g->steady_counter=1;
//             log_message("Im stopped");
//         }

//         g->new_message = 0;
//         g->array_counter = 0;
//         g->avg_distance = (g->distance_array[0]+g->distance_array[1])/2;

//         if (g->previous_avg_distance < g->avg_distance) // moving away
//         {
            
//             set_color(RGB(1, 0, 0));
//             set_motors(kilo_straight_left, 0);
//             delay(300);
//             set_motors(0,0);
//             log_message("Im away");
            
//         }

//         if (g->previous_avg_distance > g->avg_distance) 
//         {
//             set_color(RGB(0, 1, 0));
//             set_motors(kilo_straight_left, kilo_straight_right);
//             delay(2000);
//             set_motors(0,0);
//             delay(1000);
//             log_message("Im far");
//         }

//         g->previous_avg_distance=g->avg_distance;
//     }

//     else 
//     {
//         set_motors(kilo_straight_left, kilo_straight_right);
//         delay(1000);
//         set_motors(0,0);
//         delay(2000);
//         log_message("Im else");
//     }
// }


// void message_rx(message_t *m, distance_measurement_t *d)
// {
//     if (g->steady_counter==1)
//     {
//         if (kilo_ticks > g->last_changed + 64)
//         {
//             if(m->data[0] == 2)
//             {
//                 if (g->array_counter==0)
//                 {
//                     g->distance_array[0]=estimate_distance(d);
//                     g->array_counter=1;
//                 }
                
//                 if (g->array_counter==1)
//                 {
//                     g->distance_array[1]=estimate_distance(d);
//                     g->array_counter=1;
//                     g->new_message = 1;
//                 }
//             }
//         }

//     }

//     else 
//     {
//         if(m->data[0] == 2)
//         {
//             if (g->array_counter==0)
//             {
//                 g->distance_array[0]=estimate_distance(d);
//                 g->array_counter=1;
//             }
            
//             if (g->array_counter==1)
//             {
//                 g->distance_array[1]=estimate_distance(d);
//                 g->array_counter=1;
//                 g->new_message = 1;
//             }
//         }        
//     }

//}

// void top_right()
// {
//     set_motors( kilo_straight_left, kilo_straight_right);
//     delay(500);
//     set_motors(0,0);
//     set_motors( 0, kilo_straight_right);
//     delay(4400);
//     set_motors(0,0);
//     set_motors( kilo_straight_left, kilo_straight_right);
//     delay(500);
//     set_motors(0,0);
//     set_motors( kilo_straight_left, 0);
//     delay(4400);
//     set_motors(0,0);
//     delay(1500);
// }

// void bottom_right()
// {
//     set_motors( kilo_straight_left, kilo_straight_right);
//     delay(500);
//     set_motors(0,0);
//     //set_motors( 0, kilo_straight_right);
//     set_motors( kilo_straight_left, 0);
//     delay(4400);
//     set_motors(0,0);
//     set_motors( kilo_straight_left, kilo_straight_right);
//     delay(500);
//     set_motors(0,0);
//     set_motors( 0, kilo_straight_right);
//     //set_motors( kilo_straight_left, 0);
//     delay(4400);
//     set_motors(0,0);
//     delay(1500);
// }

void loop()
{
   set_color(RGB(0, 0, 0));
   if (g->new_message==1)
   {

      g->new_message=0;
       

       if (g->distance<27)
       {
         set_motors(0, 0);
         g->message_delay=1;
         set_motors( kilo_straight_left, 0);
         delay(500); 
         set_motors(0, 0);
         set_motors( kilo_straight_left, kilo_straight_right);
         delay(500);
       }


      //  if (g->distance<25)
      //  {
      //      //set_motors(0, 0);
      //      set_motors( kilo_straight_left, 0);
      //      delay(3400); 
      //      set_motors(0, 0);
      //      delay(200);
      //      set_motors( kilo_straight_left, kilo_straight_right);
      //      delay(1000);
      //  }

       if (g->distance==27||g->distance==28||g->distance==29||g->distance==30)
       {
           set_motors(0,0);
           set_color(RGB(0, 0, 1));
           g->outgoing_message.data[1]=1;
           g->steady_counter=1;
       }

       else if(g->distance>25 && g->distance<g->previous_dist)
       {

           g->steady_counter=0;
           g->outgoing_message.data[1]=0;
           set_color(RGB(1, 0, 0));
           set_motors(kilo_straight_left, kilo_straight_right);
           delay(500);
           set_motors(0,0);
           delay(500);
       }

       if (g->distance>g->previous_dist) //moving far 
       {
           g->steady_counter=0;
           g->outgoing_message.data[1]=0;
           set_color(RGB(0, 1, 0));
           set_motors(0, kilo_straight_right);
           delay(500);
           set_motors(0,0);
           delay(1000);
       }

       g->previous_dist=g->distance;
   }

   else if(kilo_ticks-g->timer>32*10 || kilo_ticks==32*30)  // if the robot hasnt recieved message in  while
   {
       g->steady_counter=0;
       g->outgoing_message.data[1]=0;
        set_motors(kilo_straight_left, kilo_straight_right);
        delay(500);
        set_motors(0,0);
        delay(500);

        // if (kilo_ticks > g->last_changed + 32)
        // {
        //     g->last_changed=kilo_ticks;
        //     set_motors(kilo_straight_left,kilo_straight_right);
        //     delay(500);
        //     set_motors(0,0);
        //     delay(500);
        // }

        if (kilo_ticks > g->last_changed + 32*30)
        {
            g->last_changed=kilo_ticks;
            set_motors(kilo_straight_left,0);
            delay(3400);
        }
   }

   if (g->new_message_from_same == 1 && g->steady_counter==0 && g->distance_from_same>0 && g->distance>28)
   {
       g->new_message_from_same = 0;
       g->outgoing_message.data[1]=0;
        set_color(RGB(1, 1, 0));
        set_motors(kilo_straight_left, kilo_straight_right);
        delay(500);
        set_motors(0,0);
        delay(500);
        set_motors(kilo_straight_left, 0);
        delay(300);
        set_motors(0,0);
        delay(300);
   }
}


void message_rx(message_t *m, distance_measurement_t *d)
{
    if(m->data[0] == 0) // then the message was from the center robot
    {
      g->timer=kilo_ticks;
      g->new_message = 1;
      g->distance = estimate_distance(d);
    }

    // we get message from the same ring number robot as current robot (m->data[0] == 1)
    // and m->data[1]==1 states that the message we are getting from 
    // is from a robot that is already stopped (BLUE) within the defined radius
    if(m->data[0] == 1 && m->data[1]==1) 
    {
        g->stopped=m->data[1];
        g->timer=kilo_ticks;
        g->new_message_from_same = 1;
        g->distance_from_same = estimate_distance(d);
    }

    if (g->message_delay==1)
    {
        if (kilo_ticks > g->message_delay_timer + 32*5)
        {
          g->message_delay_timer =kilo_ticks;
          if(m->data[0] == 0) // then the message was from the center robot
          {
            g->timer=kilo_ticks;
            g->new_message = 1;
            g->distance = estimate_distance(d);
          }
        }
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
