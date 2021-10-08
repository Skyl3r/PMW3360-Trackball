// Code based on the work of SunjunKim
// Acceleration code by Skyl3r

#include <PMW3360.h>
#include <Mouse.h>

#define SS       10
#define NUMBTN   3
#define BTN1     2
#define BTN2     3
#define BTN3     4
#define DEBOUNCE 10

int btn_pins[NUMBTN] = { BTN1, BTN2, BTN3 };
char btn_keys[NUMBTN] = { MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE };
bool btn_state[NUMBTN] = { false, false, false};
uint8_t btn_buffers[NUMBTN] = {0xFF, 0xFF, 0xFF};
unsigned long lastButtonCheck = 0;;

PMW3360 sensor;


// Acceleration parameters
float SPEED_CAP = 0.00f;
float SENSITIVITY = 1.00f;
float ACCELERATION = 0.00f;
float OFFSET = 0.00f;
float EXPONENT = 0.00f;
int ACCELERATION_MODE = 1;
unsigned long lastSensorCheck = 0;


void setup() {
  Serial.begin(9600);

  if(sensor.begin(SS))
    Serial.println("Sensor initialized successfully");
  else
    Serial.println("Sensor initialization failed");

   sensor.setCPI(1600);
   lastSensorCheck = micros();

   Mouse.begin();
   buttons_init();
}

void loop() {
  check_button_states();

  PMW3360_DATA data = sensor.readBurst();
  if(data.isOnSurface && data.isMotion) {
    int mdx = constrain(data.dx, -127, 127);
    int mdy = constrain(data.dy, -127, 127);

    //Calculate speed
    int delta_time = micros() - lastSensorCheck;
    float speed = mdx * mdx + mdy * mdy;
    speed /= delta_time;

    if(speed > SPEED_CAP && SPEED_CAP != 0.00f)
      speed = SPEED_CAP;
    

    //Apply linear acceleration
    if(ACCELERATION_MODE == 1) {
      if ( speed - OFFSET > 0 )
        speed = (speed - OFFSET) * ACCELERATION;
    }

    //apply speed and sensitivity
    float delta_x = (float)mdx * speed;
    float delta_y = (float)mdy * speed;
    delta_x *= SENSITIVITY;
    delta_y *= SENSITIVITY;
    
    Mouse.move(delta_x, delta_y, 0);
  }

}

void buttons_init() {
  for(int i = 0; i < NUMBTN; i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }
}

void check_button_states() {
  unsigned long elapsed = micros() - lastButtonCheck;

  if(elapsed < (DEBOUNCE * 1000UL / 8))
    return;

  lastButtonCheck = micros();

  for(int i=0; i < NUMBTN; i++) {
    int state = digitalRead(btn_pins[i]);
    btn_buffers[i] = btn_buffers[i] << 1 | state;
    if(!btn_state[i] && btn_buffers[i] == 0xFE) {
      Mouse.press(btn_keys[i]);
      btn_state[i] = true;
    }
    else if( (btn_state[i] && btn_buffers[i] == 0x01)
              || (btn_state[i] && btn_buffers[i] == 0xFF) ) {
      Mouse.release(btn_keys[i]);
      btn_state[i] = false;
     }
  }
}
