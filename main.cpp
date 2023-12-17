// Medical Ventilation
#include "mbed.h"
#include "PM2_Libary.h"
#include "StateLed.h"
#include "MotorFunction.h"

// Configuration of what is connected to which pins.
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

enum stateMachine {
    initial,
    reference,
    teach,
    home,
    ready,
    cycle
};

int state;

// while loop gets executed every main_task_period_ms milliseconds
int main_task_period_ms = 50;   // define main task period time in ms e.g. 50 ms -> main task runns 20 times per second
Timer main_task_timer;          // create Timer object which we use to run the main task every main task period time in ms

// led on nucleo board
DigitalOut user_led(nucleo_led_pin);      // create DigitalOut object to command user led
StateLed nucleo_led(nucleo_led_pin, 0);

// mechanical button
DigitalIn nucleo_button(nucleo_button_pin);
DigitalIn user_button(user_button_pin);
bool user_button_old;

int main()
{
    // start task timer
    main_task_timer.start();

    // set pullup mode: add resistor between pin and 3.3 V, so that there is a defined potential
    user_button.mode(PullDown);

    // enable hardwaredriver dc motors
    motorEnable();
    resetMotorControll();

    while (true) { // this loop will run forever

        main_task_timer.reset();

        // Show Motor-Debug Information
        debug();

        // State Machine
        switch (stateMachine(state)) {
        case initial:
        printf("State: Initial\n");
        nucleo_led.setInterval(0);
        if (user_button.read() and !user_button_old) state = reference;
        break;

        case reference:
        printf("State: reference\n");
        nucleo_led.setInterval(11);
        if (motorReference()) state = teach;
        break;

        case teach:
        printf("State: teach\n");
        nucleo_led.setInterval(12);
        if (motorTeach(user_button.read())) state = home;
        break;

        case home:
        printf("State: home\n");
        nucleo_led.setInterval(2);
        if (motorMoveHome()) state = ready;
        break;

        case ready:
        printf("State: ready\n");
        nucleo_led.setInterval(1);
        if (user_button.read() and !user_button_old) state = cycle;
        break;

        case cycle:
        printf("State: cycle\n");
        motorCycle(1000,3000,4000);
        if (user_button.read() and !user_button_old) state = home;
        break;

        default:
        state = initial;
        break;
        }
        user_button_old = user_button.read();

        // read timer and make the main thread sleep for the remaining time span (non blocking)
        int main_task_elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(main_task_timer.elapsed_time()).count();
        thread_sleep_for(main_task_period_ms - main_task_elapsed_time_ms);
    }
}

