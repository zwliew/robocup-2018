#include <Wire.h>
#include <Math.h>

#include "Angles.h"
#include "Ultrasonic.h"

// Debug flags
//#define DEBUG_LIGHT
#define DEBUG_COMPASS
#define DEBUG_LOCOMOTION
//#define DEBUG_US
//#define DEBUG_GATE
#define DEBUG_CAMERA
#define NO_DEBUG_OPT
//#define NO_COMPASS
#define NO_DRIBBLER
#define NO_ULTRASONIC

// Flags to enable/disable manually
#define IS_STRIKER

#ifdef NO_DEBUG_OPT
#define BAUD_RATE 9600
#endif

void setup()
{
#ifdef NO_DEBUG_OPT
  Serial.begin(BAUD_RATE);
#endif

#ifdef NO_DEBUG_OPT
  Serial.print("Setting up Main for the ");
#ifdef IS_STRIKER
  Serial.println("striker.");
#else
  Serial.println("goalkeeper.");
#endif
#endif

  Wire.begin();

  InitLoc();
#ifndef NO_DRIBBLER
  InitSld();
  InitDribbler();
#endif
  InitCamera();
#ifndef NO_COMPASS
  InitCmp();
#endif

#ifdef NO_DEBUG_OPT
  Serial.println("Main Setup complete.");
#endif
}

// Returns true if the loop should end early.
// Meant for debugging.
bool debugLoop() {
  return false;
}

#ifdef IS_STRIKER
void loop()
{
  if (debugLoop())
  {
    return;
  }
#ifndef NO_DRIBBLER
  // If we are in possession of the ball, dribble.
  const unsigned int gate_reading = ReadGate();
  if (IsBallInGate(gate_reading))
  {
    Dribble();
  }
#endif

#ifndef NO_ULTRASONIC
  // Get ultrasonic distances.
  unsigned int left, right, back;
  ReadUltrasonic(&left, &right, &back);
#endif

  // If we are out of the field,
  // move in the other direction
  //unsigned int proximity = INVALID;
  int out_corr_x = 0;
  int out_corr_y = 0;
  const bool out[4] = {
    IsFrontOut(),
    IsLeftOut(),
    IsRightOut(),
    IsBackOut()
  };
  if (out[0]) {
    out_corr_y = -1;
  } else if (out[3]) {
    out_corr_y = 1;
  }
  if (out[1]) {
    out_corr_x = 1;
  } else if (out[2]) {
    out_corr_x = -1;
  }
  if (out_corr_x || out_corr_y) {
    float out_corr_dir = atan2(out_corr_y, out_corr_x);
    if (out_corr_dir < 0) {
      out_corr_dir = -out_corr_dir + HALF_PI;
    } else if (out_corr_dir < HALF_PI) {
      out_corr_dir = HALF_PI - out_corr_dir;
    } else {
      out_corr_dir = TWO_PI - (out_corr_dir - HALF_PI);
    }
    out_corr_dir *= RAD_TO_DEG;
    switch (CalcQuadrant(int(out_corr_dir))) {
      case FIRST_QUAD:
      case FOURTH_QUAD:
        //proximity = FindEdgeProx(right);
        break;
      case SECOND_QUAD:
      case THIRD_QUAD:
        //proximity = FindEdgeProx(left);
        break;
      default:
        //proximity = FAR;
        break;
    }
    Move(0.55, out_corr_dir, INVALID);
    delay(500);
    return;
  }
#ifndef NO_DRIBBLER
  // If we are in possession of the ball, reposition then shoot.
  if (IsBallInGate(gate_reading))
  {
    const int ctr_dist = DistanceFromCenter(left, right);
    const bool within_goalie_area = WithinGoalieArea(back);
    if (ctr_dist > 20)
    {
      Move(0.4, within_goalie_area ? LEFT_FRONT_DEG : LEFT_DEG, INVALID);
    }
    else if (ctr_dist < -20)
    {
      Move(0.4, within_goalie_area ? RIGHT_FRONT_DEG : RIGHT_DEG, INVALID);
    }
    else
    {
      StopDribble();
      Move(0.4, FRONT_DEG, FAR);
      Shoot();
    }
    return;
  }
#endif
  // Otherwise, track and follow the ball
  unsigned int angle;
  float distance;
  TrackBall(&angle, &distance);
  if (angle == NO_DEG && distance == NO_DEG)
  {
    Move(0, NO_DEG, INVALID);
  }
  else
  {
    switch (CalcQuadrant(angle))
    {
    case FIRST_QUAD:
    case FOURTH_QUAD:
      angle += 33;
      //proximity = FindEdgeProx(right);
      Move(0.3, angle, INVALID);
      break;
    case SECOND_QUAD:
    case THIRD_QUAD:
      angle -= 33;
      //proximity = FindEdgeProx(left);
      Move(0.3, angle, INVALID);
      break;
    default:
      angle = FRONT_DEG;
      //proximity = INVALID;
      Move(0.4, angle, INVALID);
      break;
    }
  }
}
#else
void loop()
{
  if (debugLoop())
  {
    return;
  }
  // (0, 115) to (45, 0)

#ifndef NO_DRIBBLER
  // Dribble all the time to mess with the ball
  Dribble();
#endif

#ifndef NO_ULTRASONIC
  // Get ultrasonic distances
  unsigned int left, right, back;
  ReadUltrasonic(&left, &right, &back);
#endif

#ifndef NO_DRIBBLER
  // If we are in possession of the ball, shoot.
  const unsigned int gate_reading = ReadGate();
  if (IsBallInGate(gate_reading))
  {
    Shoot();
  }
#endif

  // If we are out of the field,
  // move in the other direction
  //unsigned int proximity = INVALID;
  int out_corr_x = 0;
  int out_corr_y = 0;
  const bool out[4] = {
      IsFrontOut(),
      IsLeftOut(),
      IsRightOut(),
      IsBackOut()};
  if (out[0])
  {
    out_corr_y = -1;
  }
  else if (out[3])
  {
    out_corr_y = 1;
  }
  if (out[1])
  {
    out_corr_x = 1;
  }
  else if (out[2])
  {
    out_corr_x = -1;
  }
  if (out_corr_x || out_corr_y)
  {
    float out_corr_dir = atan2(out_corr_y, out_corr_x);
    if (out_corr_dir < 0)
    {
      out_corr_dir = -out_corr_dir + HALF_PI;
    }
    else if (out_corr_dir < HALF_PI)
    {
      out_corr_dir = HALF_PI - out_corr_dir;
    }
    else
    {
      out_corr_dir = TWO_PI - (out_corr_dir - HALF_PI);
    }
    out_corr_dir *= RAD_TO_DEG;
    switch (CalcQuadrant(int(out_corr_dir)))
    {
    case FIRST_QUAD:
    case FOURTH_QUAD:
      //proximity = FindEdgeProx(right);
      break;
    case SECOND_QUAD:
    case THIRD_QUAD:
      //proximity = FindEdgeProx(left);
      break;
    default:
      //proximity = FAR;
      break;
    }
    Move(0.55, out_corr_dir, INVALID);
    delay(500);
    return;
  }

#ifndef NO_ULTRASONIC
  // Ensure the bot is within the goalie area
  const bool within_goalie_area = WithinGoalieArea(back);
  if (!within_goalie_area)
  {
    if (right > left)
    {
      //proximity = FindEdgeProx(right);
      Move(0.4, RIGHT_BACK_DEG, INVALID);
    }
    else
    {
      //proximity = FindEdgeProx(left);
      Move(0.4, LEFT_BACK_DEG, INVALID);
    }
    return;
  }
#endif

  // Otherwise, track and follow the ball
  unsigned int angle;
  float distance;
  TrackBall(&angle, &distance);
  if (angle == NO_DEG && distance == NO_DEG)
  {
    unsigned int to_base_deg = NO_DEG;
#ifndef NO_ULTRASONIC
    const int ctr_dist = DistanceFromCenter(left, right);
    if (back >= 60)
    {
      if (ctr_dist > 10)
      {
        to_base_deg = LEFT_BACK_DEG;
      }
      else if (ctr_dist < -10)
      {
        to_base_deg = RIGHT_BACK_DEG;
      }
    }
    else
    {
      if (ctr_dist > 15)
      {
        to_base_deg = LEFT_DEG;
      }
      else if (ctr_dist < -15)
      {
        to_base_deg = RIGHT_DEG;
      }
    }
#endif
    Move(0.3, to_base_deg, INVALID);
  }
  else
  {
    unsigned int quadrant = CalcQuadrant(angle);
    switch (quadrant)
    {
    case FIRST_QUAD:
    case FOURTH_QUAD:
      angle += 33;
      //proximity = FindEdgeProx(right);
      Move(0.3, angle, INVALID);
      break;
    case SECOND_QUAD:
    case THIRD_QUAD:
      angle -= 33;
      //proximity = FindEdgeProx(left);
      Move(0.3, angle, INVALID);
      break;
    default:
      Move(0.4, FRONT_DEG, INVALID);
      break;
    }
  }
}
#endif
