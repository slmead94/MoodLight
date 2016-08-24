#include <Arduino.h>

// define LED constants:
#define PIN_BLUE 9  // pin 9
#define PIN_GREEN 10  // pin 10
#define PIN_RED 11  // pin 11

// define sensor constants:
#define PIN_SENSOR_ONE 0  // analog 0
#define PIN_SENSOR_TWO 1  // analog 1

// miscellaneous variable declarations:
int counter = 0;
long int show_green_mod = random(32, 128) * 8;  // initiate random 8 multiple for modulus operation

// LED brightness array... Red, Green, Blue
int RGB[3] = {0, 0, 0};

int max_brightness = 255;  // max brightness isn't a constant because we may change later... with a photo cell
unsigned int delay_time = NULL;
unsigned int pause = NULL;

// integer variable(s) that rule whether an LED is changing
int incrementRed = 0;
int incrementBlue = 0;
int incrementGreen = 1;

// function declarations:
int check_color(int color, int increment);
long int pick_two();
void update();  // you don't actually have to declare void functions,
void transition();  // but you can call them from anywhere in the program if you do so...
void serial_output();
void warm_up();
void drop_levels(long int led_ls[2]);
long int show_color();


void setup(void) {
    long int first = NULL;
    long int second = NULL;

    Serial.begin(9600);

    pinMode(PIN_RED, OUTPUT);  // set the type for each pin
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    update();  // give the LEDs a starting point
    warm_up();  // give the LEDs a fade up effect

    first, second = pick_two();
    long int list[2] = {first, second};
    drop_levels(list);
}


void warm_up() {  // increment all the LEDs by 1 every 35 milliseconds to give a warming effect:
    unsigned int time = 60;

    for (int i=0; i<max_brightness; i++) {
        if (i < 30) {  // slower warm up during the start
            time--;
        }

        RGB[0]++;
        RGB[1]++;
        RGB[2]++;
        update();
        delay(time);
    }
}


void drop_levels(long int led_ls[2]) {  // this randomly drops the levels of two of the LEDs to get a bit of a random starting point
    long int droppers[2] = {random(30, 100)/*RED*/, random(75, 150)/*GREEN*/};

    for (int j=0; j<2; j++) {
        for (int i=0; i<droppers[j]; i++) {
            RGB[led_ls[j]]--;
            update();
            delay(5);
        }
    }
}


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


void serial_output() {
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
    Serial.print("Low Output Delay: ");
    Serial.println(pause);
    */
    Serial.println(); // in effect this is: "\n"
}


void transition() {  // main function that controls the LEDs
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

    if (counter % show_green_mod == 0) { // if whatever random multiple of 8 divides evenly into our counter, then run:
        first, second = show_color();  // in this instance the LEDs are already chosen for us in show_color()
        long int list[2] = {first, second};

        drop_levels(list);  // we have to create a new starting point after toying with the levels
        show_green_mod = random(32, 256) * 8;  // create new random number to change how often green shows up
    }
}


int check_color(int color, int increment) {  // This increments the LED if it needs it
    if (RGB[color] >= max_brightness)
        increment = 0;
    else if (RGB[color] <= 2)
        increment = 1;

    if (increment != 0)
        RGB[color]++;
    else
        RGB[color]--;

    return increment;
}


void update() {  // Updates the LEDs and the other parameter's status
    int photoCell = analogRead(PIN_SENSOR_ONE); // read in latest data
    int photoCell2 = analogRead(PIN_SENSOR_TWO); // read in latest data from the other sensor
    int newCell = (photoCell + photoCell2) / 2;  // average of the two sensors readings to create the number we'll use

    if (newCell >= 1100) {  // the darker it gets the slower the cube will get
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

    analogWrite(PIN_RED, RGB[0]);
    analogWrite(PIN_GREEN, RGB[1]);
    analogWrite(PIN_BLUE, RGB[2]);
}


void loop(void) {
    if (counter < 32767) { // 32767 is the highest number you can use in a standard integer
        counter = counter + 1;
        if (counter % 12 == 0 && counter != 0) {
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