// Medical Ventilation
#include "mbed.h"
#include "PM2_Libary.h"
#include <cstdio>
#include <string>
#include "ButtonFunction.h"
#include "StateLed.h"
#include "MotorFunction.h"

// logical variable main task
//bool do_execute_main_task = false;  // this variable will be toggled via the user button (blue button) to or not to execute the main task

// user button on nucleo board
//Timer user_button_timer;            // create Timer object which we use to check if user button was pressed for a certain time (robust against signal bouncing)
//InterruptIn user_button(PC_13);     // create InterruptIn interface object to evaluate user button falling and rising edge (no blocking code in ISR)
//void user_button_pressed_fcn();     // custom functions which gets executed when user button gets pressed and released, definition below
//void user_button_released_fcn();

// Konfiguration was auf welchen Pin's angeschlossen ist.
// IN
PinName nucleo_button_pin = PC_13;  // user button on nucleo board
PinName user_button_pin = PC_6;    
//PinName mechanical_button_pin = PC_5;

// OUT
PinName nucleo_led_pin = LED1;

// Motor
//PinName enable_dc_motors_pin = PB_15;
//PinName pwm_M1_pin = PB_13;
//PinName encoder_M1_pin1 = PA_6;
//PinName encoder_M1_pin2 = PC_7;

/*
ToDo:
einen Bereich für den Motor
einen Bereich für den Button    - OK
einen Bereich für die LED       - OK
einen Bereich für den Fluss/Ablauf
*/


//void reference_button_pressed_fcn();
//void reference_button_released_fcn();

//bool referenceMotor();
//bool teachtMotor();
//bool moveHomeMotor();
//bool cycleMotor();

//void setRotateMotor(float speed, bool direction);
//void setStopMotor();

enum stateMachine {
    initial,
    reference,
    teach,
    home,
    ready,
    cycle
};

//State stateMachine(int state);
int state;

//bool reference_activ = false;
//int reference_step = 0;
//float referencePos;
//bool motorStoped;

//const bool move_main_direction = false;

// while loop gets executed every main_task_period_ms milliseconds
int main_task_period_ms = 50;   // define main task period time in ms e.g. 50 ms -> main task runns 20 times per second
Timer main_task_timer;          // create Timer object which we use to run the main task every main task period time in ms

// led on nucleo board
DigitalOut user_led(nucleo_led_pin);      // create DigitalOut object to command user led
//StateLed nucleo_led(nucleo_led_pin);

// mechanical button
DigitalIn nucleo_button(nucleo_button_pin);
DigitalIn user_button(user_button_pin);
bool user_button_old;
//InterruptIn reference_button(mechanical_button_pin);  // create DigitalIn object to evaluate extra mechanical button, you need to specify the mode for proper usage, see below
//ButtonPressed user_button_pressed(user_button_pin);
//ButtonPressed nucleo_button_pressed(nucleo_button_pin);

// 78:1, 100:1, ... Metal Gearmotor 20Dx44L mm 12V CB
//DigitalOut enable_motors(enable_dc_motors_pin);    // create DigitalOut object to enable dc motors

//float   pwm_period_s = 0.00005f;    // define pwm period time in seconds and create FastPWM objects to command dc motors
//FastPWM pwm_M1(pwm_M1_pin);         // motor is closed-loop position controlled (angle controlled)

//EncoderCounter  encoder_M1(encoder_M1_pin1, encoder_M1_pin2); // create encoder objects to read in the encoder counter values

// create SpeedController and PositionController objects, default parametrization is for 78.125:1 gear box
//float max_voltage = 12.0f;                  // define maximum voltage of battery packs, adjust this to 6.0f V if you only use one batterypack
//float counts_per_turn = 20.0f * 78.125f;    // define counts per turn at gearbox end: counts/turn * gearratio
//float kn = 180.0f / 12.0f;                  // define motor constant in rpm per V
//float k_gear = 100.0f / 78.125f;            // define additional ratio in case you are using a dc motor with a different gear box, e.g. 100:1
//float kp = 0.1f;                            // define custom kp, this is the default speed controller gain for gear box 78.125:1
//float max_speed_rps = 0.5f;                 // define maximum speed that the position controller is changig the speed, has to be smaller or equal to kn * max_voltage

//PositionController positionController_M1(counts_per_turn * k_gear, kn / k_gear, kp * k_gear, max_voltage, pwm_M1, encoder_M1); // parameters adjusted to 100:1 gear, we need a different speed controller gain here

int main()
{
    // attach button fall and rise functions to user button object
    //reference_button.fall(&reference_button_pressed_fcn);
    //reference_button.rise(&reference_button_released_fcn);
    // start timer
    main_task_timer.start();

    // set pullup mode: add resistor between pin and 3.3 V, so that there is a defined potential
    user_button.mode(PullDown);
    //reference_button.mode(PullUp);


    motorEnable();
    resetMotorControll();
    // enable hardwaredriver dc motors: 0 -> disabled, 1 -> enabled
    //enable_motors = 1;

    // motor M1 is used open-loop, we need to initialize the pwm and set pwm output to zero at the beginning, range: 0...1 -> u_min...u_max: 0.5 -> 0 V
    //pwm_M1.period(pwm_period_s);
    //pwm_M1.write(0.5f);

    // set the soft pwm period for the servo objects
    // servo_S1.SetPeriod(servo_period_mus);
    // servo_S2.SetPeriod(servo_period_mus);

    while (true) { // this loop will run forever

        main_task_timer.reset();

        debug();
        //if (user_button.read() == 1) {
        //    positionController_M1.setDesiredRotation(1.0/360);
        //} else if (reference_button.read() == 0) {
        //    positionController_M1.setDesiredRotation(-1.0/360);
        //}

        
    //string test ="Main";
    //printf("%s\n", test.c_str());
    user_led = !user_led;

        switch (stateMachine(state)) {
        case initial:
        printf("State: Initial\n");
        //nucleo_led.setInterval("100,-100");
        //if (user_button_pressed.getShortPressed(true)) state = reference;
        if (user_button.read() and !user_button_old) state = reference;
        break;

        case reference:
        printf("State: reference\n");
        //nucleo_led.setInterval("100,-100,100,-300");
        if (motorReference()) state = teach;
        break;

        case teach:
        printf("State: teach\n");
        if (motorTeach(user_button.read())) state = home;
        break;

        case home:
        printf("State: home\n");
        if (motorMoveHome()) state = ready;
        break;

        case ready:
        printf("State: ready\n");
        //nucleo_led.setInterval("300,-300");
        //if (user_button_pressed.getShortPressed(true)) state = cycle;
        if (user_button.read() and !user_button_old) state = cycle;
        break;

        case cycle:
        printf("State: cycle\n");
        motorCycle(1000,3000);
        //if (user_button_pressed.getLongPressed(true)) state = home;
        if (user_button.read() and !user_button_old) state = home;
        break;

        default:
        state = initial;
        break;
        }
        user_button_old = user_button.read();
        
        /*
        if (do_execute_main_task) {

            // read analog input
            // ir_distance_mV = 1.0e3f * ir_analog_in.read() * 3.3f;

            // command dc motors if mechanical button is pressed
            if (mechanical_button.read()) {
                pwm_M1.write(0.75f); // write output voltage to motor M1
                // speedController_M2.setDesiredSpeedRPS(0.5f); // set a desired speed for speed controlled dc motors M2
                // positionController_M3.setDesiredRotation(1.5f, max_speed_rps); // set a desired rotation for position controlled dc motors M3
            } else {
                pwm_M1.write(0.5f);
                // speedController_M2.setDesiredSpeedRPS(0.0f);
                // positionController_M3.setDesiredRotation(0.0f, max_speed_rps);
            }

            // check if servos are enabled
            // if (!servo_S1.isEnabled()) servo_S1.Enable();
            // if (!servo_S2.isEnabled()) servo_S2.Enable();
            // command servo position, this needs to be calibrated
            // servo_S1.SetPosition(servo_S1_angle);
            // if (servo_S1_angle < 1.0f & servo_counter%loops_per_seconds == 0 & servo_counter != 0) {
            //     servo_S1_angle += 0.01f;
            // }
            // servo_S2.SetPosition(servo_S2_angle);
            // if (servo_S2_angle < 1.0f & servo_counter%loops_per_seconds == 0 & servo_counter != 0) {
            //     servo_S2_angle += 0.01f;
            // }
            // servo_counter++;

            // read ultra sonic distance sensor
            // us_distance_cm = us_range_finder.read_cm();

            // visual feedback that the main task is executed
            // extra_led = 1;

        } else {

            // ir_distance_mV = 0.0f;

            pwm_M1.write(0.5f);
            // speedController_M2.setDesiredSpeedRPS(0.0f);
            // positionController_M3.setDesiredRotation(0.0f, max_speed_rps);

            // servo_S1_angle = 0;
            // servo_S2_angle = 0;
            // servo_S1.SetPosition(servo_S1_angle);
            // servo_S2.SetPosition(servo_S2_angle);
            // if (servo_S1.isEnabled()) servo_S1.Disable();
            // if (servo_S2.isEnabled()) servo_S2.Disable();

            // us_distance_cm = 0.0f;

            // extra_led = 0;
        }
        */

        //user_led = !user_led;

        // do only output via serial what's really necessary (this makes your code slow)
        // printf("IR sensor (mV): %3.3f, Encoder M1: %3d, Speed M2 (rps) %3.3f, Position M3 (rot): %3.3f, Servo S1 angle (normalized): %3.3f, Servo S2 angle (normalized): %3.3f, US sensor (cm): %3.3f\r\n",
        //        ir_distance_mV,
        //        encoder_M1.read(),
        //        speedController_M2.getSpeedRPS(),
        //        positionController_M3.getRotation(),
        //        servo_S1_angle,
        //        servo_S2_angle,
        //        us_distance_cm);

        // read out the imu, the actual frames of the sensor reading needs to be figured out
        // imu.updateGyro();
        // imu.updateAcc();
        // imu.updateMag();
        // printf("%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\r\n", imu.readGyroX(), imu.readGyroY(), imu.readGyroZ(),
        // imu.readAccX(), imu.readAccY(), imu.readAccZ(), imu.readMagX(), imu.readMagY(), imu.readMagZ());

        // read timer and make the main thread sleep for the remaining time span (non blocking)*/
        int main_task_elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(main_task_timer.elapsed_time()).count();
        thread_sleep_for(main_task_period_ms - main_task_elapsed_time_ms);
    }
}


//bool teachtMotor();
//bool moveHomeMotor();
//bool cycleMotor();
//void setRotateMotor(float speed, bool direction);
//void setStopMotor();

