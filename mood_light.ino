/*                    What's your Mood?...
 * <<<<<<<<<<<<<<<<<-/<<>><<>><<>><<>><<>>\->>>>>>>>>>>>>>>>>>
 |>-----------------------------------------------------------<|
 * Date Created: August 22nd, 2016
 * Project: Mood Light
 * Purpose: None... Just having fun
 *
 * Creator: Spencer Mead
 |>-----------------------------------------------------------<|
 * Bugs / Glitches:
 *  - Sometimes the transition from blue to red will glitch
 *    a bit
 |>-----------------------------------------------------------<|
 * Additional Notes:
 *
 *  update() gets called all over the place, just because it's
 *  easier
 |>-----------------------------------------------------------<|
 */


#include <Arduino.h>

// define LED constants:
#define PIN_BLUE 9  // pin 9
#define PIN_GREEN 10  // pin 10
#define PIN_RED 11  // pin 11

// define sensor constants:
#define PIN_SENSOR_ONE 0  // analog 0
#define PIN_SENSOR_TWO 1  // analog 1

// function declarations:
int check_color(int color, int increment);
long int pick_two();
void update();
void update_photoCell();
void transition();
void serial_output();
void warm_up();
void drop_levels(long int led_ls[2]);
long int show_color();
long int create_random_number();

// miscellaneous variable declarations:
int counter = 0;
long int new_pattern_trigger = create_random_number();  // initiate random 8 multiple for modulus operation

// LED brightness array... Red, Green, Blue
int RGB[3] = {0, 0, 0};

int max_brightness = 255;  // max brightness isn't a constant because we may change later... with a photo cell
unsigned int delay_time = NULL;
unsigned int pause = NULL;

// integer variable(s) that rule whether an LED is changing
int incrementRed = 0;
int incrementBlue = 0;
int incrementGreen = 0;


// Runs only once
void setup(void) {
    long int first = NULL;
    long int second = NULL;

    Serial.begin(9600);

    pinMode(PIN_RED, OUTPUT);  // set the type for each pin
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    update();  // initiate the LEDs brightness value
    warm_up();  // give the LEDs a fade up effect

    first, second = pick_two();
    long int list[2] = {first, second};
    drop_levels(list);
}


/*
 * Slowly brings all of the LEDs up to max brightness
 * */
void warm_up(void) {
    unsigned int time = 60;

    for (int i=0; i<max_brightness; i++) {
        if (i % 5 && time > 20) {  // every five iterations, diminish the delay by 1
            time--;
        }

        RGB[0]++;
        RGB[1]++;
        RGB[2]++;
        update();
        delay(time);
    }
    update();
}


/*
 * Randomly drops the levels of the LEDs chosen in pick_two().
 * */
void drop_levels(long int led_ls[2]) {
    long int droppers[2] = {random(30, 100)/*Item 0 in the list*/, random(30, 150)/*Item 1 in the list*/};

    for (int j=0; j<2; j++) {
        for (int i=0; i<droppers[j]; i++) {
            RGB[led_ls[j]]--;
            update();
            delay(5);
        }
    }
}


/*
 * Picks two numbers randomly that correspond to two LEDs which then
 * will be dimmed and used to help create the cubes new pattern/feel.
 * */
long int pick_two() {
    boolean cont = true;
    long int first = NULL;
    long int second = NULL;

    while (cont != 0) {  // chooses two random lights to dim, so technically it chooses one light to be shown
        first = random(0, 2);  // first light to be extinguished
        second = random(0, 2);  // other light to be extinguished... the LED that didn't get picked gets some showtime

        if (first == second) {  // make sure we chose two different LEDs
            cont = true;
        } else {
            cont = 0;
        }
    }

    return first, second;
}

/*
 * Creates a random number to help randomize when a new pattern is created
 * */
long int create_random_number() {
    long int new_number;

    long int remainder_executioner = 0;
    long int starter = random(32, 1024);
    long int multiplier = random(5, 17);
    long int divider = random(1, 4);
    long int subtrahend = random(20, (starter * multiplier) / divider) / 4;

    if (((starter * multiplier) - subtrahend) % divider != 0) {  // I think this works...
        remainder_executioner = (((starter * multiplier) - subtrahend) % divider) / divider;
    }

    new_number = ((starter * multiplier) - subtrahend) / divider - remainder_executioner;
    new_number = round(new_number);

    return new_number; // possible numbers: 35ish - 13056
}


/*
 * Every time show_color() is called, that is when the program will be
 * re-creating the pattern randomly, and essentially rebuilds the
 * cubes look...
 *
 * show_color() is only called periodically... new_pattern_trigger
 * is the variable that controls how often it is called...
 * whatever number that is generated for new_pattern_trigger will be
 * divided into the counter variable (which counts how many times
 * the program has looped), and if the remainder is zero, it will
 * run show_color() and create a new pattern
 */
long int show_color() { // shows a random color by itself for 2 seconds
    long int first = NULL;
    long int second = NULL;

    first, second = pick_two();

    // max out all of the colors first
    for (int j=0; j<3; j++) {
        for (int i = 0; i < max_brightness; i++) {
            if (RGB[j] < max_brightness) {
                RGB[j]++;
            }
            update();
            delay(10);
        }
    }

    for (int i=0; i<max_brightness-5; i++) {  // dims the selected lights to almost 0
        RGB[first]--;
        RGB[second]--;  // should equal 6 when the loop is done
        update();
        delay(10);
    }

    delay(2000);  // pause so the color can actually be seen for a bit
    for (int i=0; i<max_brightness-25; i++) {  // increases the recently dimmed lights almost back to full brightness
        RGB[first]++;
        RGB[second]++;
        update();
        delay(20);
    }

    return first, second;
}


/*
 * Sensor data output
 * */
void serial_output(void) {
    int photoCell = analogRead(PIN_SENSOR_ONE); // read in latest data
    int photoCell2 = analogRead(PIN_SENSOR_TWO); // read in latest data

    // latest data reading from the sensors:
    Serial.print("Photocell: ");
    Serial.println(photoCell);
    Serial.print("Photocell 2: ");
    Serial.println(photoCell2);
    /*
    // other data:
    Serial.print("Global Delay: ");
    Serial.println(delay_time);  // prints the current delay time
    Serial.print("Low Output Delay: "); // the delay when an LED reaches <= 2
    Serial.println(pause);
    */
    Serial.println(); // in effect this is: "\n"
}


/*
 * Runs check_color() for each LED and then sets the increment values again
 * in case they have changed.
 *
 * Also the main block of code that re-creates the blocks pattern if
 * new_pattern_trigger divides evenly into counter.
 * */
void transition(void) {  // main function that controls the LEDs
    long int first = NULL;
    long int second = NULL;
    int increment_list[3] = {incrementRed, incrementGreen, incrementBlue}; // list of increment values

    for (int i=0; i<3; i++) {
        int inc = check_color(i, increment_list[i]);
        increment_list[i] = inc;  // changes local value of the increment var so we can change the global values later
    }

    // update the increment values:
    incrementRed = increment_list[0];
    incrementGreen = increment_list[1];
    incrementBlue = increment_list[2];

    if (counter % new_pattern_trigger == 0) { // if whatever random multiple of 8 divides evenly into our counter, then run:
        update();
        first, second = show_color();  // in this instance the LEDs are already chosen for us in show_color()
        long int list[2] = {first, second};

        drop_levels(list);  // we have to create a new starting point after toying with the levels
        new_pattern_trigger = create_random_number();
    }
}


/*
 * Checks to see if the sent LED should be incremented. If not, and it's dark, it
 * will set the increment value to do so in the future.
 * */
int check_color(int color, int increment) {  // This increments the LED if it needs it
    if (RGB[color] >= max_brightness)
        increment = 0;
    else if (RGB[color] <= 2)
        increment = 1;

    if (increment != 0)
        RGB[color]++;
    else
        RGB[color]--;
    update();

    return increment;
}


/*
 * update() reads in the latest data from the PhotoCells
 * and then using that, updates the cubes behavior...
 * At the end it assigns the latest brightness value
 * to each LED
 * */
void update_photoCell(void) {  // Updates the LEDs and the other parameter's status
    int newCell;
    int photoCell = analogRead(PIN_SENSOR_ONE); // read in latest data
    int photoCell2 = analogRead(PIN_SENSOR_TWO); // read in latest data from the other sensor

    if (photoCell > photoCell2) {
        newCell = photoCell2;
    } else {
        newCell = photoCell;
    }

    if (newCell >= 1019) {  // the darker it gets the slower the cube will get
        delay_time = 50;
        pause = 750;
    } else if (newCell >= 1000) {  // the darkest its gotten is 10237
        delay_time = 40;
        pause = 500;
    } else if (newCell >= 900) {  // fully lit room usually winds up around 800 - 900
        delay_time = 25;
        pause = 250;
    } else if (newCell >= 800) {
        delay_time = 20;
        pause = 150;
    } else {
        delay_time = 15;
        pause = 100;
    }
}


/*
 * Assigns the given PWMs the amount of power to output
 * ...This is probably called unnecessarily a lot, but
 * I'd rather call it more than needed than not enough.
 * */
void update(void) {
    analogWrite(PIN_RED, RGB[0]);
    analogWrite(PIN_GREEN, RGB[1]);
    analogWrite(PIN_BLUE, RGB[2]);
}


/* Main loop */
void loop(void) {
    if (counter < 32767) { // 32767 is the highest number you can use in a standard integer
        counter = counter + 1;
        if (counter % 12 == 0) {
            update_photoCell();
            serial_output();
        }
    } else {  // if it is maxed out, set the variable equal to zero
        counter = 0;
    }

    update();

    if (RGB[0] <= 2 || RGB[1] <= 2 || RGB[2] <= 2) {  // if and when an LED is at its lowest point, pause
        delay(pause);
    }

    transition();
    delay(delay_time);  // delay(); in milliseconds
}