//
// motor controller code by Neil Nie & Michael Meng
// Steering Controller Module | self driving golf cart
// (c) Yongyang Nie, All rights reserved
//

// IMPORTANT NOTES:
// --------------------------------
// RED motor wire connected to side
// RED batt wire connected to mid
// MOVE dir 1 is out
// --------------------------------

#include <ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Bool.h>

#define RPWM 7
#define LPWM 6
#define M_PI 3.14159265358979

#define la_max 700
#define la_min 190
#define la_md = 415
#define pot_pin 0

unsigned long count;      // count for encode *might have duplicate variables
double pos = 0.0;         // steering position
boolean joystick_enabled = false;
float cmd_val = 0.0;
float target_pos = 0.0;
bool killed;

ros::NodeHandle nh;

// the ros callback methods must be declared before the ros subscriber statement.

void steering_callback( const std_msgs::Float32& cmd_msg) {

  // target_pos = cmd_msg.data;
  double input = cmd_msg.data;
  if (input < -M_PI) {
    input = -M_PI;
  } else if (input > M_PI) {
    input = M_PI;
  }

  if (!joystick_enabled) {
    double cmd = mapf(input, -M_PI, M_PI, la_min, la_max);
    target_pos = cmd;
  }
}

void joystick_callback( const std_msgs::Float32& cmd_msg) {
  
  if (joystick_enabled && !killed) {
    cmd_val = cmd_msg.data;
    if (cmd_val > 0 && pos > la_min) {
      move_actuator(255, 0);
      // going out
    } else if (cmd_val < 0 && pos < la_max) {
      move_actuator(255, 1);
    } else {
      stop_actuator();
    }
  }
}

void joystick_enabled_callback( const std_msgs::Bool& cmd_msg) {
  
  if (joystick_enabled == true) {
      target_pos = pos;
    }
    joystick_enabled = cmd_msg.data;
}

void killswitch_callback( const std_msgs::Bool& cmd_msg) {
  killed = cmd_msg.data;
}

// ----------------------------------------------------------------------------------------
// declare all subscribers
ros::Subscriber<std_msgs::Float32> sub1("/vehicle/dbw/steering_cmds/", steering_callback);
ros::Subscriber<std_msgs::Float32> sub2("/sensor/joystick/left_stick_x", joystick_callback);
ros::Subscriber<std_msgs::Bool> sub3("/sensor/joystick/enabled", joystick_enabled_callback);
ros::Subscriber<std_msgs::Bool> ks_sub("/vehicle/safety/killed", killswitch_callback);

// declare the publisher
std_msgs::Float32 pos_msg;
ros::Publisher pos_pub("/sensor/vehicle/steering/actuator_position", &pos_msg);
// ----------------------------------------------------------------------------------------

void setup() {

  nh.initNode();
  nh.subscribe(sub1);
  nh.subscribe(sub2);
  nh.subscribe(sub3);
  nh.subscribe(ks_sub);

  nh.advertise(pos_pub);

  //setup motor
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);

  pos = analogRead(pot_pin);
  target_pos = pos;
}

void loop() {

  pos = analogRead(pot_pin);

  pos_msg.data = pos;
  pos_pub.publish(&pos_msg);

  if (killed) {
    target_pos = pos;
  }

  if (!joystick_enabled) {

    if (abs(pos - target_pos) > 20) {
      if (pos < target_pos)
        move_actuator(255, 1);
      else if (pos > target_pos)
        move_actuator(255, 0);
    }
    else {
      stop_actuator();
    }
  }

  nh.spinOnce();
  delay(5);
}


// END ------ Linear Actuator Encoder Interrupt ----------------------

// BEGIN ---- Helper Methods  ----------------------------------------

void move_actuator(int spd, boolean dir) {
  if (dir) {
    analogWrite(LPWM, 0);
    analogWrite(RPWM, spd);
  } else {
    analogWrite(LPWM, spd);
    analogWrite(RPWM, 0);
  }
}

void stop_actuator() {
  analogWrite(LPWM, 0);
  analogWrite(RPWM, 0);
  delay(5);
}

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// END ------ Helper Methods  ----------------------------------------
