//
//  pirate.ino
//
//  Created by Gabriele Mondada on 2023.
//  Copyright (c) 2023 Gabriele Mondada.
//  Distributed under the terms of the MIT License.
//

#include <Arduino.h>
#include "AS5600.h"    // https://github.com/RobTillaart/AS5600
#include "strip.h"
#include "player.h"
#include "util.h"

AS5600 as5600;
Strip<7, D5> leds;
Player player;

enum LedState {
    stateBlink,
    stateError,
    stateSuccess,
    stateOff
};

float freq_of_note(int midi_note) {
    return 440.0f * powf(2.0f, (float)(midi_note - 69) / 12.0f);
}

int32_t get_rotary_abs_pos(int32_t hysteresis) {
    static int32_t out;
    int32_t in = as5600.readAngle();    // 12-bit value
    int32_t delta12 = (in - out) & 0x0fff;
    int32_t delta = delta12 << 20 >> 20;    // extend sign to 32-bit value
    if (delta > hysteresis) {
        out += delta - hysteresis;
    } else if (delta < -hysteresis) {
        out += delta + hysteresis;
    }
    return out;
}

int32_t get_wheel_pos() {
    return asym_div(get_rotary_abs_pos(100), 4096 / 4);
}

void setup() {
    digitalWrite(D8, 0);
    pinMode(D8, OUTPUT);    // lock
    digitalWrite(D1, LOW);
    pinMode(D1, OUTPUT);    // AUDIO_EN
    digitalWrite(D7, 1);
    pinMode(D7, OUTPUT);    // N_POWER_OFF
    pinMode(D4, OUTPUT);    // LED
    pinMode(D0, INPUT);

    Serial.begin(115200);
    delay(500);
    Serial.write("\n\n=== Begin ===\n");

    Wire.begin(D2, D6);
    as5600.begin();
    player.begin();    // must be done after serial init
    digitalWrite(D8, 0);
    pinMode(D8, OUTPUT);    // lock - must be configurer after as5600
}

void led_control(int step, LedState state) {
    bool blink = (millis() / 250) % 2 != 0;
    for (int i = 0; i < leds.getLength(); i++) {
        if (i < step) {
            leds.setPixel(i, 255);
        } else if (i == step) {
            switch (state) {
                case stateBlink:
                    leds.setPixel(i, blink ? 255 : 0);
                    break;
                case stateError:
                    leds.setPixel(i, 255, 0, 0);
                    break;
                case stateSuccess:
                    leds.setPixel(i, 0, 255, 0);
                    break;
                case stateOff:
                    leds.setPixel(i, 0);
                    break;
            }
        } else {
            leds.setPixel(i, 0);
        }
    }
    leds.show();
}

void loop() {
    digitalWrite(D1, HIGH);    // enable audio

    int sequence[] = { 2, 1, -2, 3, -1, -1, 2 };
    int step = 0;
    int start_pos = get_wheel_pos();
    int prev_pos = start_pos;
    uint32_t last_activity_time = millis();
    for (;;) {
        int pos = get_wheel_pos();

        if (pos != prev_pos)
            last_activity_time = millis();
        if (millis() - last_activity_time > 30000)
            goto power_off;

        if (pos == start_pos + sequence[step]) {
            // step succeed
            led_control(step, stateSuccess);
            delay(750);
            if (step == ARRAY_LEN(sequence) - 1)
                break;
            start_pos = pos;
            step++;
        } else if (pos != prev_pos && sign(pos - prev_pos) != sign(sequence[step])) {
            // bad direction
            led_control(step, stateError);
            delay(1000);
            start_pos = pos;
            step = 0;
        }
        prev_pos = pos;
        led_control(step, stateBlink);
        delay(20);
    }

    for (int i = 0; i < 5; i++) {
        digitalWrite(D4, 0);
        leds.setAll(255);
        leds.show();
        delay(100);
        digitalWrite(D4, 1);
        leds.setAll(0);
        leds.show();
        delay(100);
    }

    player.push_beep(2000, 0.1, 0.2);
    delay(100);
    player.push_beep(2000, 0.1, 0.2);

    digitalWrite(D8, 1);
    delay(2000);
    digitalWrite(D8, 0);

power_off:
    digitalWrite(D1, LOW);    // disable audio
    delay(500);
    digitalWrite(D7, 0);    // switch off the board
    delay(2000);
}
