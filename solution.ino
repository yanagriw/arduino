#include "funshield.h"

constexpr size_t interval_to_boost = 400;
constexpr size_t boost_interval = 200;
constexpr size_t max_throws = 9;
constexpr size_t min_throws = 1;
int throws = 1;
int dices = 4;
int random_number = 0;
enum States {RANDOM, CONFIG};
States state = CONFIG;
long last = 0;

struct Display{

  void setup(){
    pinMode(latch_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
  }

  void write_glyph( byte glyph, byte position) {
    digitalWrite( latch_pin, LOW);
    shiftOut( data_pin, clock_pin, MSBFIRST, glyph);
    shiftOut( data_pin, clock_pin, MSBFIRST, position);
    digitalWrite( latch_pin, HIGH);
  }

  void write_random_number (int random_number){
    for (int i = 3; random_number != 0; i--){
      write_glyph(digits[random_number%10], digit_muxpos[i]);
      random_number /= 10;
    }
  }

  void write_config (int dices) {
    write_glyph(digits[throws], digit_muxpos[0]);
    write_glyph(0b10100001, digit_muxpos[1]);
    if (dices < 10) {write_glyph(digits[dices], digit_muxpos[2]);}
    else {for (int i = 3; i != 1; i--){
        write_glyph(digits[dices%10], digit_muxpos[i]);
        dices /= 10;
      }
    }
  }
};

struct Button {
  bool pressed = false;
  bool boost = false;
  size_t pin;
  size_t last_read;

  Button (size_t button_pin){
    pin = button_pin;
  }

  void setup(){
    pinMode (pin, INPUT);
  }

  bool is_on(){
    int readedValue = digitalRead(pin);
    if (!pressed){
      if(!readedValue){
        pressed = true;
        last_read = millis();
        return true;
      }
    }

    else if(!boost){
      if(!readedValue && millis() - last_read > interval_to_boost){
        boost = true;
        last_read += interval_to_boost;
        return true;
      }
      else if(readedValue){
        pressed = false;
      }
    }

    else{
      if(!readedValue && millis() - last_read > boost_interval){
        last_read += boost_interval;
        return true;
      }
      else if (readedValue){
        boost = false;
        pressed = false;
      }
    }
    return false;
  }
};

struct Buttons{
  Button buttons[3] = {
    Button(button1_pin),
    Button(button2_pin),
    Button(button3_pin)
  };

  void setup(){
    for (int i = 0; i < 2; i++){
      buttons[i].setup();
    }
  }

  bool is_on(int button_number){
    return buttons[button_number - 1].is_on();
  }

  size_t last_read(int button_number){
    return buttons[button_number - 1].last_read;
  }
};


struct Arduino{
  Display display = Display();
  Buttons buttons = Buttons();

  void setup(){
    buttons.setup();
    display.setup();
  }

  int create_random (){
    //randomSeed(buttons.last_read(1));
    int sum = 0;
    for (int i = 0; i < throws; i++){
      sum += random(1, dices+1);
    }
    return sum;
  }

  void increase_throws(){
    if (throws < max_throws){
      throws++;
    } else {
      throws = min_throws;
    }
  }

  void increase_dices(){
    if (dices == 12){dices = 20;}
    else if (dices == 20){dices = 100;}
    else if (dices == 100) {dices = 4;}
    else { dices += 2;}
  }

  void refresh(){
    long current = millis();
    if (buttons.is_on(1) && current - last > boost_interval){
      state = RANDOM;
      random_number = create_random();
      last = millis();
    }

    if (buttons.is_on(2) && current - last > boost_interval){
      state = CONFIG;
      increase_throws();
      last = millis();
    }

    if (buttons.is_on(3) && current - last > boost_interval){
      state = CONFIG;
      increase_dices();
      last = millis();
    }
  }
};

Arduino arduino = Arduino();

void setup() {
  arduino.setup();
}

void loop() {
  arduino.refresh();

  if (state == RANDOM){
    arduino.display.write_random_number(random_number);
  }

  if (state == CONFIG){
    arduino.display.write_config(dices);
  }
}
