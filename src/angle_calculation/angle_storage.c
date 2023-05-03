/**
 * @file angle_storage.c
 * @author Jack Mirenzi
 *
 * @brief Calculates the angle between two robots and stores it in a linked list
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// standard library
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// kilolib library
#include "../../kilolib/kilolib.h"
#define DEBUG
#include "../../kilolib/debug.h"

// preprocessor directives
#define DUO_DISTANCE 38  // mm
#define MSG_TYPE_ANGLE 1
#define MSG_TYPE_EMPTY 0

#ifndef M_PI
#define M_PI 3.1415926535
#endif

// global variables
uint16_t wait_time = 10;
message_t message;
uint8_t color_state;

// communicating message variables
uint8_t pair_id;
message_t message;
uint16_t delay_ms = 30;

// angle storage linked list
typedef struct Kilo_knowledge_node {
  uint8_t bot_uid;
  uint8_t distance;
  uint8_t pair_distance;
  uint8_t angle;
  uint32_t time_stamp;
  struct Kilo_knowledge_node *ptr;
}kilo_knowledge_node;


kilo_knowledge_node *nodes_head = NULL;
// clock_t dist_time;
// clock_t pair_dist_time;
// clock_t time_;



#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// long abstimediff(clock_t t1, clock_t t2) {
//     long elapsed;
//     elapsed = abs(((double)t2 - t1) / CLOCKS_PER_SEC * 1000000);
//     return elapsed;
// }

void print_knowledge() {
  #ifdef DEBUG
  kilo_knowledge_node *p = nodes_head;
  while (p != NULL) {
      printf(" (%d, %d) ", p->bot_uid, p->angle);
      p = p->ptr;
    }
  printf("\n");
  #endif
  return;
}

kilo_knowledge_node* findNodeByID(uint8_t bot_uid_des) {
  kilo_knowledge_node *p = nodes_head;
  while (p != NULL) {
      if (p->bot_uid == bot_uid_des)
        return p;
      p = p->ptr;
     }
  return NULL;
}

kilo_knowledge_node* add_node(uint8_t bot_uid) {
  kilo_knowledge_node *new_node = (kilo_knowledge_node*) malloc(sizeof(kilo_knowledge_node));
  new_node->bot_uid = bot_uid;
  new_node->distance = 0;
  new_node->pair_distance = 0;
  new_node->angle = 0;
  new_node->ptr = nodes_head;
  nodes_head = new_node;
  return new_node;
}

void set_color_safe(uint8_t color) {
  if (color_state == color)
    return;
  color_state = color;
  set_color(color);
}

void turnFor(uint8_t direction, uint16_t time_ms) {
  // direction=0 => right and direction=1 => left
  if ((kilo_uid+direction)%2) {
    spinup_motors();
    set_motors(kilo_straight_left*.75, kilo_straight_right);
    } else {
    set_motors(0, 0);
    }
  delay(time_ms);
  set_motors(0, 0);
}

void turnRightFor(uint16_t time_ms) {
  turnFor(0, time_ms);
}

void turnLeftFor(uint16_t time_ms) {
  turnFor(1, time_ms);
}

void cherp(void) {
  int cherp_strength = 75;
  set_motors(cherp_strength, cherp_strength);
  delay(25);
  set_motors(0, 0);
  delay(25);
  set_motors(cherp_strength, cherp_strength);
  delay(50);
  set_motors(0, 0);
}

int find_pair_id(int id) {
    return id + ((id%2) ? -1 : 1);
}

uint8_t get_angle(uint8_t pair_dist, uint8_t dist) {
  float d0 = DUO_DISTANCE, A, x, d1, d2;
  if (kilo_uid > pair_id) {
    d1 = pair_dist;
    d2 = dist;
    } else {
      d1 = dist;
      d2 = pair_dist;
      }
  A = acos((d1*d1+d0*d0-d2*d2)/(2*d1*d0));
  x = sqrt(.25*d0*d0+d1*d1-d1*d0*cos(A));
  return (uint8_t)(180*acos((x*x +.25*d0*d0 -d2*d2)/(x*d0))/M_PI);
}

message_t *message_tx() {
  // printf("msg sent\n");
  message.type = NORMAL;
  message.data[0] = kilo_uid;  // send this robots ID to the other robot*/
  message.data[1] = MSG_TYPE_EMPTY;
  kilo_knowledge_node *p = nodes_head;
  int i = 1;
  while ((p != NULL) && (i < 4)) {
    message.data[1] = MSG_TYPE_ANGLE;
    message.data[i*2] = p->bot_uid;
    message.data[i*2+1] = p->distance;
    if (p->ptr != NULL) {
      p = p->ptr;
    }
    i++;
  }
  // Sends number of nodes sent
  // if (i>3)
  //   i = 3;
  // message.data[1] = i;
  // It's important that the CRC is computed after the data has been set;
  // otherwise it would be wrong.

  message.crc = message_crc(&(message));
  return &(message);
}

// message success callback (called *after* a message is sent)

// message recieve callback
void message_rx(message_t *m, distance_measurement_t *d) {
//   if(kilo_uid!=0 && kilo_uid!=1){
//     return;
//  }

  uint8_t sending_robot_id = m->data[0];
  // #ifdef DEBUG
  // printf("msg from: %d \n",sending_robot_id);
  // #endif
  if (sending_robot_id == kilo_uid) {
    return;
  }
  uint8_t dis = estimate_distance(d);

  if (sending_robot_id != pair_id) {
  // if (sending_robot_id!=0 && sending_robot_id != 1){
    cherp();
    kilo_knowledge_node *p = findNodeByID(sending_robot_id);
    if (p == NULL) {
      p = add_node(sending_robot_id);
    }
    p->distance = dis;
    if (p->pair_distance != 0) {
      p->angle = get_angle(p->pair_distance, p->distance);
    }
    return;
  } else {  // data is from your pair
    if (m->data[1] == MSG_TYPE_EMPTY)
      return;
    int i;
    kilo_knowledge_node *p;
    for (i=1; i< 4; i++) {
      p = findNodeByID(m->data[2*i]);
      if (p == NULL) {
        p = add_node(m->data[2*i]);
      }
      p->pair_distance = m->data[2*i+1];
      if (p->distance != 0) {
        p->angle = get_angle(p->pair_distance, p->distance);
      }
    }
  }
  return;
}

// put your setup code here, will be run once at the beginning
void setup() {
  color_state = RGB(0, 0, 0);
  pair_id = find_pair_id(kilo_uid);
}

// put your main code here, will be run repeatedly
void loop() {
  // if(findNodeByID(3)!=NULL && (kilo_uid==1 || kilo_uid==0)) {
  //   print_knowledge();
  //   uint8_t angle = findNodeByID(3)->angle;
  //   if(32<angle && angle <= 64)
  //     set_color_safe(RGB(0,2,2));
  //   else if (64<angle && angle <= 96)
  //     set_color_safe(RGB(0,2,0));
  //   else if (96<angle && angle <= 128)
  //     set_color_safe(RGB(2,2,0));
  //   else if (128<angle && angle <= 160)
  //     set_color_safe(RGB(2,0,0));
  //   else if (160<angle && angle <= 192)
  //     set_color_safe(RGB(2,0,2));
  //   else
  //     set_color_safe(RGB(0,0,2));
  // }
  if (nodes_head != NULL && (kilo_uid == 1 || kilo_uid == 0)) {
    print_knowledge();
    } else {
      set_color_safe(RGB(2, 2, 2));
      }
  // printf("%d \n",find)
  delay(wait_time);
}

int main() {
  kilo_init();
  // Register the message_rx callback function.
  // if (kilo_uid==0 && kilo_uid == 1)
  kilo_message_rx = message_rx;

  // Register the message_tx callback function.
  kilo_message_tx = message_tx;

  #ifdef DEBUG
  debug_init();
  #endif

  kilo_start(setup, loop);

  return 0;
}
