//
//  strip.h
//
//  Created by Gabriele Mondada on 2023.
//  Copyright (c) 2023 Gabriele Mondada.
//  Distributed under the terms of the MIT License.
//

#pragma once

#include "FastLED.h"

template<int len, int pin>
class Strip {

  private:
    CRGB leds[len];

  public:
    Strip() {
        FastLED.addLeds<WS2811, pin, GRB>(leds, len).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(16);
        FastLED.show();
    }

    int getLength() {
        return len;
    }

    void setIntensity(uint8_t intensity) {
        FastLED.setBrightness(intensity);
    }

    void setPixel(int index, CRGB color) {
        if (index >= 0 && index < len) {
            leds[index] = color;
        }
    }

    void setPixel(int index, byte r, byte g, byte b) {
        setPixel(index, CRGB(r, g, b));
    }

    void setPixel(int index, byte w) {
        setPixel(index, CRGB(w, w, w));
    }

    void setAll(CRGB color) {
        for (int i = 0; i < len; i++) {
            setPixel(i, color);
        }
    }

    void setAll(byte r, byte g, byte b) {
        setAll(CRGB(r, g, b));
    }

    void setAll(byte w) {
        setAll(CRGB(w, w, w));
    }

    void show() {
        FastLED.show();
    }

    void demo1() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < len; j++) {
                byte v = ((j % 3) == i) ? 255 : 0;
                setPixel(j, v, v, v);
            }
            show();
            delay(50);
        }
    }

    void demo2() {
        int width = 3;
        for (int t = 0; t < 100; t++) {
            float posf = cosf((float)t * 2.0f * (float)M_PI * 0.01f) * 0.5f + 0.5f;
            int pos = (int)floorf(posf * ((float)(len - width + 1) - 0.001f));
            setAll(0, 0, 0);
            setPixel(pos + 0, 255, 0, 255);
            setPixel(pos + 1, 255, 0, 255);
            setPixel(pos + 2, 255, 0, 255);
            show();
            delay(10);
        }
    }

    // Rainbow Cycle

    CRGB wheel(byte WheelPos) {
        CRGB c;
        if (WheelPos < 85) {
            c.r = WheelPos * 3;
            c.g = 255 - WheelPos * 3;
            c.b = 0;
        } else if (WheelPos < 170) {
            WheelPos -= 85;
            c.r = 255 - WheelPos * 3;
            c.g = 0;
            c.b = WheelPos * 3;
        } else {
            WheelPos -= 170;
            c.r = 0;
            c.g = WheelPos * 3;
            c.b = 255 - WheelPos * 3;
        }
        return c;
    }

    void demo3() {
        for (int j = 0; j < 256; j++) {
            for (int i = 0; i < len; i++) {
                CRGB c = wheel(((i * 256 / len) + j) & 255);
                setPixel(i, c);
            }
            show();
            delay(10);
        }
    }

    // Running Light

    void runningLights(byte red, byte green, byte blue, int WaveDelay) {
        int Position = 0;

        for (int j = 0; j < len; j++) {
            Position++;    // = 0; //Position + Rate;
            for (int i = 0; i < len; i++) {
                setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                         ((sin(i + Position) * 127 + 128) / 255) * green,
                         ((sin(i + Position) * 127 + 128) / 255) * blue);
            }
            show();
            delay(WaveDelay);
        }
    }

    void demo4() {
        runningLights(0xff, 0xa2, 0x03, 50);
    }
};
