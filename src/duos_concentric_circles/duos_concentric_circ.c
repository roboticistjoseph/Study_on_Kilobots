#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32))) //sets the kth bit in array A
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32))) //checks if we have 1 at kth position
#define NUM_R 46                                      // no of robots

struct GLOBALS
{
    // global variables
    // NOTE: the use of a GLOBALS struct will also work on a normal kilobot,
    //       but is not required on a normal kilobot.
    //       It is, however, required by the simulator.
    int rcvd_id;
    int looper;
    int current_motion;
    int distance;
    int new_message;
    int timer;
    int last_changed;
    int ring_status_from_star_robot;
    int ring_status_from_planet_robot;
    int message_from_stopped_L1_robot;
    int new_message_from_L1_robot;
    int my_stop_status;
    int distance_from_L1_robot;
    int previous_dist_from_planet_robot;
    int layer_2_robot_done;
    int new_message_from_L2_robot;
    int distance_from_L2_robot;

    //**************************************
    int my_array[3];   //holds bits for 96 robots 3*(4*8bits)  int is 4bytes
    int rcvd_array[3]; //holds bits for 96 robots 3*(4*8bits)  int is 4bytes
    int time_array[NUM_R];
    int y;
    int rcvd_count;
    int my_count;
    int my_ring_number;
    int previous_dist_from_L2;
    int heard_from_L2;

    //***************************************

    message_t outgoing_message;

} * g; // there should only be one GLOBAL, this is it, remember to register it in main()

// put your setup code here, will be run once at the beginning
// this is a good place, for example, to define the intial values
// of global variables
void setup()
{
    g->rcvd_id = -1;
    //**********************************************************
    for (int n = 0; n < 3; n++)
    {
        g->my_array[n] = 0;
        g->rcvd_array[n] = 0;
    }

    for (int r = 0; r <= NUM_R - 1; r++)
    {
        g->time_array[r] = 0;
    }

    g->y = 0;
    g->rcvd_count = 0;
    g->my_count = 0;
    g->looper = -1;

    g->outgoing_message.data[4] = 0; // for rcvd array
    g->outgoing_message.data[5] = 0; // for rcvd array
    g->outgoing_message.data[6] = 0; // for rcvd array
    g->outgoing_message.data[7] = 0; // for counter value
                                     //g->messgae.data[8]=0;  // layer number
    g->my_ring_number = 0;
    g->previous_dist_from_L2 = 0;
    g->heard_from_L2 = 0;
    //**********************************************************

    g->distance = 10000;
    g->new_message = 0;
    g->timer = kilo_ticks = 0;
    g->last_changed = 0;
    g->ring_status_from_star_robot = 0;
    g->ring_status_from_planet_robot = 0;
    g->message_from_stopped_L1_robot = 0;
    g->new_message_from_L1_robot = 0;
    g->my_stop_status = 0;
    g->distance_from_L1_robot = 10000;
    g->previous_dist_from_planet_robot = 0;
    g->layer_2_robot_done = 0;
    g->new_message_from_L2_robot = 0;
    g->distance_from_L2_robot = 10000;

    // (note that because we are never changing the data in the message in this example
    // then we only need to do this once, otherwise we'd need to do it every time we
    // wanted to send a new message)

    // The type is always NORMAL.
    g->outgoing_message.type = NORMAL;

    // indicate we are a planet
    g->outgoing_message.data[0] = 1;

    //saying we stopped
    g->outgoing_message.data[1] = 0; //my ring number

    //kilo_uid
    g->outgoing_message.data[2] = kilo_uid;

    //ring status to tell other robot planets
    g->outgoing_message.data[3] = 0;

    // It's important that the CRC is computed after the data has been set;
    // otherwise it would be wrong.
    g->outgoing_message.crc = message_crc(&g->outgoing_message);
}

int count()
{
    int counter = 0;
    for (int p = 0; p < 96; p++)
    {
        if (TestBit(g->my_array, p))
        {
            counter++;
        }
    }
    return counter;
}

void loop()
{
    g->looper++;
    if (g->looper == 0 || g->looper == 1 || g->looper == 2)
    {
        delay(8000);
    }
    //set_color(RGB(0, 0, 0));
    g->outgoing_message.data[1] = 0;
    //g->outgoing_message.data[4]=0;
    g->my_stop_status = 0;
    //log_message("Ring status from star %d",g->ring_status_from_star_robot);

    if (g->new_message == 1)
    {
        g->new_message = 0;
        set_color(RGB(0, 0, 0));

        //g->distance==25||g->distance==26||g->distance==27||g->distance==28||
        //if (g->distance == 20 ||g->distance == 21 ||g->distance == 22 ||g->distance == 23 ||g->distance == 24 ||g->distance == 25 || g->distance == 26 || g->distance == 27 || g->distance == 28 || g->distance == 29 || g->distance == 30 || g->distance == 31) // stopping criteria
        if (g->distance>10 && g->distance<=31)
        {
            set_motors(0, 0);
            delay(1000);
            set_color(RGB(0, 0, 1));

            g->my_ring_number = 1;
            g->outgoing_message.data[1] = 1; // Im ring 1
            g->my_stop_status = 1;
            //g->messgae.data[8]=1;  // I'm ring 1
        }

        if (g->distance < 10 && g->ring_status_from_star_robot == 0 && g->ring_status_from_planet_robot == 0) //&& g->distance>=20
        {
            // OTTE: turn around... left robot moves straight, right robot stops, then both move straight (assuming both robot exicute this concurently)


            //g->outgoing_message.data[1]=0;
            //g->message.data[3]=0;
            set_color(RGB(1, 0, 0));
            if (kilo_uid < NUM_R / 2)
            {
                //set_color(RGB(1, 1, 1));
                set_motors(kilo_straight_left, kilo_straight_right);
                delay(2000);
                set_motors(0, 0);
                delay(500);
            }

            else if (kilo_uid >= NUM_R / 2)
            {
                //set_color(RGB(1, 0, 1));
                set_motors(0, 0);
                delay(2000);
                set_motors(0, 0);
                delay(500);
            }
            //set_color(RGB(0, 1, 1));
            //set_motors(kilo_straight_left, 0);
            //delay(500);
            //set_motors(0, 0);
            //delay(500);
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(650);
            g->my_stop_status = 0;
        }

        if (g->distance == 32 || g->distance == 33 || g->distance == 34 || g->distance == 35)
        {

            // OTTE: a little too far away so move straight (assuming both robot exicute this concurently) or pivot (assuming that the other robot is stopped)



            //g->outgoing_message.data[1]=0;
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(150);
            set_motors(0, 0);
            delay(100);
            //g->my_stop_status=0;
        }


        // OTTE: Ring interaction stuff for circle 1
        if (g->ring_status_from_star_robot == 1) //if the ring is formed then I will tell other planets about it
        {
            if (g->distance < 10)
            {
                set_color(RGB(1, 1, 1));
                set_motors(0, 0);
                delay(1000);
            }
            g->outgoing_message.data[3] = 1; //ring status to tell other robot planets
        }
        else
        {
            g->outgoing_message.data[3] = 0;
        }
    }

    if (g->new_message_from_L1_robot == 1)
    {
        g->new_message_from_L1_robot = 0;

        if (g->message_from_stopped_L1_robot == 1 && g->my_stop_status == 0 && g->ring_status_from_planet_robot == 0 && g->distance >= 31) //steering for formation of L1
        {

            // OTTE: not stopped and ring not formed, and recieved message from a robot on circle 1, so move striaght and then turn 

            g->message_from_stopped_L1_robot = 0;
            set_color(RGB(0, 1, 0));
            set_motors(kilo_straight_left, kilo_straight_right);
            delay(700);
            set_motors(0, 0);
            delay(500);

            if (kilo_uid < NUM_R / 2)
            {
                //set_color(RGB(1, 1, 1));
                set_motors(kilo_straight_left, kilo_straight_right);
                delay(300);
                set_motors(0, 0);
                delay(300);
            }

            else if (kilo_uid >= NUM_R / 2)
            {
                //set_color(RGB(1, 0, 1));
                set_motors(0, 0);
                delay(300);
                set_motors(0, 0);
                delay(300);
            }
            // set_motors(kilo_straight_left, 0);
            // delay(300);
            // set_motors(0, 0);
            // delay(300);
        }

        if (g->message_from_stopped_L1_robot == 1 && g->my_stop_status == 0 && g->ring_status_from_planet_robot == 1 && g->distance > 30) //ring 2 formation
        {
            g->message_from_stopped_L1_robot = 0;


            // OTTE: ring 1 is formed, this robor is not stopped, and it is far from the seed robot

            if (g->distance_from_L1_robot >= 28)
            {
                // OTTE: far from ring 1 robot, so aim to form ring 2

                g->my_stop_status = 1;          // OTTE: not sure why we stop... perhaps to avoid scrweing things up?
                set_color(RGB(1, 0, 0));
                set_motors(0, 0);
                delay(10000);
                g->my_ring_number = 2;
                g->outgoing_message.data[1] = g->my_ring_number; // indicating we the L2 robot
            }

            if (g->distance_from_L1_robot < 28) //chnage this for "perfect-ness of the circle". Larger the value, better the circle but trades off with time
            {

                // OTTE: close to ring 1 robot...


                if (g->distance_from_L1_robot > g->previous_dist_from_planet_robot) //going away
                {
                    // OTTE: moving away, it looks like we just move forward a bit


                    g->previous_dist_from_planet_robot = g->distance_from_L1_robot;
                    set_color(RGB(1, 0, 1));
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(1000);
                    set_motors(0, 0);
                    //g->my_stop_status=1; //try removing this
                }

                if (g->distance_from_L1_robot < g->previous_dist_from_planet_robot)
                {
                    // OTTE: going toward the circle 1 robot, we change stop status to stopped, and then pivot and then move straight (assuming both robot exicute this concurently)

                    g->my_stop_status = 1;
                    //set_color(RGB(0, 1, 0));

                    if (kilo_uid < NUM_R / 2)
                    {
                        //set_color(RGB(1, 1, 1));
                        set_motors(kilo_straight_left, kilo_straight_right);
                        delay(1500);
                        set_motors(0, 0);
                        delay(300);
                    }

                    else if (kilo_uid >= NUM_R / 2)
                    {
                        //set_color(RGB(1, 0, 1));
                        set_motors(0, 0);
                        delay(1500);
                        set_motors(0, 0);
                        delay(300);
                    }

                    //set_motors(kilo_straight_left, 0);
                    //delay(1500);
                    //set_motors(0, 0);
                    //delay(300);
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(500);
                    g->previous_dist_from_planet_robot = g->distance_from_L1_robot;
                }
            }
        }
    }

    if (g->new_message_from_L2_robot == 1)
    {

        // OTTE: got message from robot on circle 2 

        g->new_message_from_L2_robot = 0;

        // OTTE ---------- START different cases given a message from a robot on circle 1

        if (g->my_stop_status == 1 && g->ring_status_from_planet_robot == 1 && g->distance > 32)
        {
            // OTTE already stopped, and the circle 1 is already formed, and we're greater than 32 from, presumably, the robot on circle 2


            //log_message("COUNTING ");
            for (g->y = 0; g->y < 96; g->y++) // OR-ING
            {
                g->my_array[g->y / 32] = g->my_array[g->y / 32] | g->rcvd_array[g->y / 32];
            }

            g->outgoing_message.data[4] = g->my_array[0];
            g->outgoing_message.data[5] = g->my_array[1];
            g->outgoing_message.data[6] = g->my_array[2];
            g->my_count = count();

            if (g->rcvd_count > g->my_count)
            {
                g->my_count = g->rcvd_count;
            }
            log_message("Red N %d", g->my_count);
            g->outgoing_message.data[7] = g->my_count;
        }

        if (g->my_stop_status == 0 && (g->my_count >= 10 || g->rcvd_count >= 10))
        {
            // OTTE: not stopped, and we've heard from 10 robots on the circle (or 10 this time)





            //log_message("FTS");
            if ((g->distance_from_L2_robot == 25 || g->distance_from_L2_robot == 26 || g->distance_from_L2_robot == 27 || g->distance_from_L2_robot == 28 || g->distance_from_L2_robot == 29 || g->distance_from_L2_robot == 30 || g->distance_from_L2_robot == 31 || g->distance_from_L2_robot == 32) && g->distance_from_L1_robot > 30 && g->distance > 30)
            {

               // Otte: if in appoperiate range of circle 2 and also greater than 30 from all other types of robots in circles, then stop


                set_color(RGB(1, 1, 0));
                set_motors(0, 0);
                log_message("Brrrrrr");
                g->my_stop_status = 1;
                delay(10000);
            }

            if (g->distance_from_L2_robot < 28 && g->distance > 30) //chnage this for "perfect-ness of the circle". Larger the value, better the circle but trades off with time
            {

                // OTTE: keep moving away

                if (g->distance_from_L2_robot > g->previous_dist_from_L2) //going away
                {
                    g->previous_dist_from_L2 = g->distance_from_L2_robot;
                    set_color(RGB(1, 0, 1));
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(1000);
                    set_motors(0, 0);
                    //g->my_stop_status=1; //try removing this
                }

                if (g->distance_from_L2_robot < g->previous_dist_from_L2)
                {

                    // Otte: turn left, then move straight. Not sure what this is about


                    g->my_stop_status = 1;
                    //set_color(RGB(0, 1, 0));

                    if (kilo_uid < NUM_R / 2)
                    {
                        //set_color(RGB(1, 1, 1));
                        set_motors(kilo_straight_left, kilo_straight_right);
                        delay(1500);
                        set_motors(0, 0);
                        delay(300);
                    }

                    else if (kilo_uid >= NUM_R / 2)
                    {
                        //set_color(RGB(1, 0, 1));
                        set_motors(0, 0);
                        delay(1500);
                        set_motors(0, 0);
                        delay(300);
                    }

                    //set_motors(kilo_straight_left, 0);
                    //delay(1500);
                    //set_motors(0, 0);
                    //delay(300);
                    set_motors(kilo_straight_left, kilo_straight_right);
                    delay(500);
                    g->previous_dist_from_L2 = g->distance_from_L2_robot;
                }
            }
        }
        // OTTE ---------- END different cases given a message from a robot on circle 1

    }

    if ((kilo_ticks - g->timer > 32 * 10 || kilo_ticks == 32 * 15) && g->my_stop_status == 0) // if the robot hasnt recieved message in  while
    {
        set_color(RGB(0, 0, 0));
        //g->steady_counter=0;
        //g->message.data[3]=0;
        set_motors(kilo_straight_left, kilo_straight_right);
        delay(500);
        set_motors(0, 0);
        delay(500);
        //g->my_stop_status=0;

        if (kilo_ticks > g->last_changed + 32 * 30)
        {

            // OTTE: if not stopped and havn't recieved a message in a while then move forward a bit


            g->last_changed = kilo_ticks;
            if (kilo_uid < NUM_R / 2)
            {
                set_color(RGB(1, 1, 1));
                set_motors(kilo_straight_left, kilo_straight_right);
                delay(3400);
                set_motors(0, 0);
                delay(300);
            }

            else if (kilo_uid >= NUM_R / 2)
            {
                set_color(RGB(1, 0, 1));
                set_motors(0, 0);
                delay(3400);
                set_motors(0, 0);
                delay(300);
            }

            set_color(RGB(0, 0, 0));
        }
    }
}

// recieve message callback function
void message_rx(message_t *m, distance_measurement_t *d)
{

    if (m->data[0] == 0) // getting message from star robot
    {
        g->timer = kilo_ticks;
        g->new_message = 1;
        g->distance = estimate_distance(d);

        if (m->data[1] >= 10) // star robot saying that it has reached desired number of neighbors
        {
            g->ring_status_from_star_robot = 1; // ring has been formed
        }
        else if (m->data[1] < 10)
        {
            g->ring_status_from_star_robot = 0;
        }
    }

    if (m->data[0] == 1) // getting message from planet robot
    {
        g->new_message_from_L1_robot = 1;

        if (m->data[2]==kilo_uid +(NUM_R/2)||m->data[2]==kilo_uid-(NUM_R/2))
        {

        }

        else
        {
            if (m->data[1] == 1) // saying that we got a message from a L1 robot
            {
                g->distance_from_L1_robot = estimate_distance(d);
                g->message_from_stopped_L1_robot = 1;
                g->timer = kilo_ticks;
            }

            if (m->data[1] == 2) // getting message from L2 robot (alrready stopped)
            {
                //g->timer=kilo_ticks;
                g->heard_from_L2 = kilo_ticks;
                g->new_message_from_L2_robot = 1;
                g->distance_from_L2_robot = estimate_distance(d);

                //************
                //g->time_array[m->data[2]]=kilo_ticks;
                SetBit(g->my_array, m->data[2]); // setting m->data[1]th bit equal 1 , m->data[1] is the kilouid of the robot we recieve message from
                g->rcvd_array[0] = m->data[4];
                g->rcvd_array[1] = m->data[5];
                g->rcvd_array[2] = m->data[6];
                g->rcvd_count = m->data[7];
                //*************
            }

            if (m->data[3] == 1) // a planet sent us message that the ring has formed
            {
                g->ring_status_from_planet_robot = 1;
            }
        }
    }
}

// send message function
message_t *message_tx()
{
    //log_message_from_sim("|| P  data[0]: %u", g->outgoing_message.data[0]);

    return &(g->outgoing_message);
}

// sucessfull send message callback
void message_tx_success()
{
    // do nothing
}

int main()
{
    GLOBALS *g_safe = (GLOBALS *)malloc(sizeof(GLOBALS));

#ifdef USING_SIMULATION
    // register the global variables (only necessary on simulation)
    //g is a pointer
    //so the address of g is a pointer to a pointer,the address of it is getting casted to a pointer to a void pointer

    kilo_globals = (void **)&g;
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
