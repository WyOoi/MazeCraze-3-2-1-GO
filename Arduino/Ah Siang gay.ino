SR04 wf_sr04_left = SR04(LEFT_US_SENSOR_ECHO,LEFT_US_SENSOR_TRIG);
SR04 wf_sr04_front = SR04(FRONT_US_SENSOR_ECHO,FRONT_US_SENSOR_TRIG);

int WF_Left=0;
int WF_Front=0;

int WF_Left_Status=0;
int WF_Front_Status=0;

float WF_Error = 0;
float WF_Kp = 11;
float WF_Ki = 0.003;
float WF_Kd = 0.2; 
int WF_LeftTurnSpeed = 0;
int WF_RightTurnSpeed = 0;
int WF_Correction = 0;
float WF_Integral = 0;
float WF_Derivative = 0;
float WF_LastError = 0;



void WALL_FOLLOWER (IRrecv irrecv) {

     while (!IS_REMOTE_BUTTON_STOP(GET_REMOTE_CODE(irrecv))) {

      WF_Front_Status = WF_GET_FRONT_US_STATUS();
      WF_Left_Status = WF_GET_LEFT_US_STATUS();

      if(WF_Front_Status == NO_OBSTACLE && WF_Left_Status == OBSTACLE) {
        WF_CONTINUE_WALL();
      } else if(WF_Front_Status == NO_OBSTACLE && WF_Left_Status == NO_OBSTACLE) {
        WF_LEFT();
      } else {
        WF_RIGHT();
      }

      }

  WF_STOP();
}


void WF_INITIALIZE_WALL(void) {
WF_Integral = 0;
WF_Derivative = 0;
WF_LastError = 0;

}

void WF_CONTINUE_WALL (void)
{
    WF_Left = wf_sr04_left.Distance();
    WF_Error = WF_Left - WF_DISTANCE;
    WF_Integral = (WF_Error + WF_Integral);
    WF_Derivative = (WF_Error - WF_LastError);

    WF_Correction = WF_Kp * WF_Error + WF_Kd * WF_Derivative + WF_Ki * WF_Integral;

    WF_LeftTurnSpeed = 128 - WF_Correction;
    WF_RightTurnSpeed = 128 + WF_Correction;

    motor3.setSpeed(WF_LeftTurnSpeed);
    motor3.run(FORWARD);
    motor4.setSpeed(WF_RightTurnSpeed);
    motor4.run(FORWARD);

    WF_LastError = WF_Error;
}

int WF_GET_LEFT_US_STATUS(void) {
     if(wf_sr04_left.Distance() < WF_DISTANCE)
      return OBSTACLE;
     else
      return NO_OBSTACLE;
}

int WF_GET_FRONT_US_STATUS(void) {
     if(wf_sr04_front.Distance() < WF_FRONT_DISTANCE)
      return OBSTACLE;
     else
      return NO_OBSTACLE;
}


void WF_RIGHT (void)
{
   //Serial.println("Right ..");
   motor3.setSpeed(WF_SPEED);
   motor3.run(FORWARD);
   motor4.setSpeed(WF_NO_SPEED);
   motor4.run(FORWARD);
}

void WF_LEFT (void)
{
   //Serial.println("Left ..");
   motor3.setSpeed(WF_NO_SPEED);
   motor3.run(FORWARD);
   motor4.setSpeed(WF_SPEED);
   motor4.run(FORWARD);
}

void WF_STOP (void)
{
  //Serial.println("Stop..");
   motor3.setSpeed(WF_NO_SPEED);
   motor4.setSpeed(WF_NO_SPEED);
}
