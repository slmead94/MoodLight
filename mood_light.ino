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
unsigned int delay_time = 30;  // we may also change this too...
unsigned int pause = 250;

// integer variable(s) that rule whether an LED is changing
int incrementRed = 0;
int incrementBlue = 0;
int incrementGreen = 1;

// function declarations:
int check_color(int color, int increment);
void update();  // you don't actually have to declare void functions,
void transition();  // but you can call them from anywhere in the program if you do so...
void serial_output();
void warm_up();
void drop_levels();
void show_color();


void setup(void) {
    Serial.begin(9600);

    pinMode(PIN_RED, OUTPUT);  // set the type for each pin
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    update();  // give the LEDs a starting point
    warm_up();  // give the LEDs a fade up effect
    drop_levels();
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


// drops the GREEN and RED brightness levels:
void drop_levels() {  // this randomly drops the levels of two of the LEDs to get a bit of a random starting point
    long int drop_g = random(75, 150);
    long int drop_r = random(30, 100);

    for (int i=0; i<drop_g; i++) {
        RGB[1]--;
        update();
        delay(5);
    }

    for (int i=0; i<drop_r; i++) {
        RGB[0]--;
        update();
        delay(5);
    }
}


void show_color() {
    boolean cont = true;
    long int first = NULL;
    long int second = NULL;

    while (cont != 0) {  // chooses two random lights to dim, so technically it chooses one light to be shown
        first = random(0, 2);
        second = random(0, 2);

        if (first == second) {
            cont = true;
        } else {
            cont = 0;
        }
    }

    // max out all of the colors first
    for (int i=0; i<max_brightness; i++) {
        if (RGB[0] < max_brightness) {
            RGB[0]++;
        }
        if (RGB[1] < max_brightness) {
            RGB[1]++;
        }
        if (RGB[2] < max_brightness) {
            RGB[2]++;
        }

        update();
        delay(10);
    }

    for (int i=0; i<max_brightness-5; i++) {  // dims the selected lights to almost 0
        RGB[first]--;
        RGB[second]--;  // should equal 6 when the loop is done
        update();
        delay(10);
    }

    delay(2000);  // pause so the color can try to have some alone time
    for (int i=0; i<max_brightness-25; i++) {  // increases the recently dimmed lights almost back to full brightness
        RGB[first]++;
        RGB[second]++;
        update();
        delay(20);
    }
}


void serial_output() {
    int photoCell = analogRead(PIN_SENSOR_ONE); // read in latest data
    int photoCell2 = analogRead(PIN_SENSOR_TWO); // read in latest data from the other sensor

    // latest data pull from the sensors:
    Serial.print("Photocell: ");
    Serial.println(photoCell);
    Serial.print("Photocell 2: ");
    Serial.println(photoCell2);

    // other data:
    Serial.print("Global Delay: ");
    Serial.println(delay_time);  // prints the current delay time
    Serial.print("Low Output Delay: ");
    Serial.println(pause);
    Serial.println(); // in effect this is: "\n"
}


void update() {  // Updates the LEDs and the other parameter's status
    int photoCell = analogRead(PIN_SENSOR_ONE); // read in latest data
    int photoCell2 = analogRead(PIN_SENSOR_TWO); // read in latest data from the other sensor
    int newCell = (photoCell + photoCell2) / 2;  // average of the two sensors readings to create the number we'll use

    if (newCell >= 1100) {  // the darker it gets the slower the cube will get
        delay_time = 50;
        pause = 750;
    } else if (newCell >= 1000) {
        delay_time = 40;
        pause = 500;
    } else if (newCell >= 900) {
        delay_time = 25;
        pause = 250;  // default
    } else if (newCell >= 800) {
        delay_time = 20;
        pause = 150;
    } else if (newCell >= 750) {
        delay_time = 15;
        pause = 100;
    }

    analogWrite(PIN_RED, RGB[0]);
    analogWrite(PIN_GREEN, RGB[1]);
    analogWrite(PIN_BLUE, RGB[2]);
}


int check_color(int color, int increment) {
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


void transition() {  // main function that controls the LEDs
    int increment_list[3] = {incrementRed, incrementGreen, incrementBlue}; // list of increment value instances

    if (RGB[0] <= 2 || RGB[1] <= 2 || RGB[2] <= 2) {  // if and when an LED is at its lowest point, pause
        delay(pause);
    }

    for (int i=0; i<3; i++) {
        int inc = check_color(i, increment_list[i]);
        increment_list[i] = inc;  // changes local value of the increment var so we can change the global values later
    }

    // update the increment values:
    incrementRed = increment_list[0];
    incrementGreen = increment_list[1];
    incrementBlue = increment_list[2];

    if (counter % show_green_mod == 0) { // if whatever random multiple of 8 divides evenly into our counter, then run:
        show_color();
        drop_levels();  // we have to create a new starting point after toying with the levels
        show_green_mod = random(32, 256) * 8;  // create new random number to change how often green shows up
    }
}


void loop(void) {
    if (counter < 32767) { // 32767 is the highest number you can use in a standard integer
        counter = counter + 1;
        if (counter % 12 == 0) {
            serial_output();
        }
    } else {  // if it is maxed out, set the variable equal to zero
        counter = 0;
    }

    update();
    transition();

    delay(delay_time);  // delay(); in milliseconds
}