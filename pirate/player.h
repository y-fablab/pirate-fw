//
//  player.h
//
//  Created by Gabriele Mondada on 2023.
//  Copyright (c) 2023 Gabriele Mondada.
//  Distributed under the terms of the MIT License.
//

#pragma once

#include <math.h>
#include <functional>
#include <I2S.h>

#define PLAYER_ACTION_FIFO_BIT 4
#define PLAYER_ACTION_FIFO_LEN (1 << PLAYER_ACTION_FIFO_BIT)
#define PLAYER_ACTION_FIFO_MASK (PLAYER_ACTION_FIFO_LEN - 1)

class Player {
  private:
    class Action {
      public:
        int32_t input_sts;
        std::function<void()> func;
    };

    int16_t sample_error = 0;

    Action action_fifo[PLAYER_ACTION_FIFO_LEN];
    uint32_t action_fifo_in = 0;
    uint32_t action_fifo_out = 0;

    /**
     * If sample is INT16_MAX, output is always 1.
     * If sample is INT16_MIN, output is always 0.
     * If sample is exactly between INT16_MIN and INT16_MAX,
     * i.e. sample == -0.5, the output is 0101010101... but
     * this value is not possible in an int16_t.
     */
    uint32_t pdm(int16_t sample) {
        uint32_t m = 0;
        for (uint8_t i = 0; i < 32; i++) {
            m <<= 1;
            if (sample >= sample_error) {
                m |= 1;
                sample_error += INT16_MAX - sample;
            } else {
                sample_error += INT16_MIN - sample;
            }
        }
        return m;
    }

  public:
    int32_t input_sts;

    void begin() {
        i2s_begin();
        i2s_set_rate(44100);
    }

    void push_sample(int16_t sample) {
        i2s_write_sample(pdm(sample));
        input_sts++;
        if (action_fifo_in != action_fifo_out) {
            int32_t action_sts = action_fifo[action_fifo_out & PLAYER_ACTION_FIFO_MASK].input_sts;
            int32_t diff = action_sts - input_sts;
            if (diff <= 0) {
                action_fifo[action_fifo_out & PLAYER_ACTION_FIFO_MASK].func();
                action_fifo_out++;
            }
        }
    }

    void ramp_in() {
        for (int i = 0; i < 2048; i++) {
            int16_t vi = i << 4;
            push_sample(INT16_MIN + vi);
        }
    }

    void ramp_out() {
        for (int16_t i = 0; i < 2048; i++) {
            int16_t vi = i << 4;
            push_sample(-vi);
        }
    }

    void push_action(int32_t input_sts, std::function<void()> func) {
        uint32_t content_size = action_fifo_in - action_fifo_out;
        if (content_size < PLAYER_ACTION_FIFO_LEN) {
            Action action;
            action.input_sts = input_sts;
            action.func = func;
            action_fifo[action_fifo_in & PLAYER_ACTION_FIFO_MASK] = action;
            action_fifo_in++;
        }
    }

    void push_action_rel(int32_t delta_input_sts, std::function<void()> func) {
        int32_t sts = input_sts + delta_input_sts;
        push_action(sts, func);
    }

    /**
     * Push a square waveform beep of given frequency, duration and volume.
     * The waveform is not centered on 0. It is desinged to be played when
     * at INT16_MIN.
     * @param freq in Hz
     * @param len in seconds
     * @param volume from 0.0f to 1.0f
     */
    void push_beep(float freq, float len, float volume) {
        float half_period_time = 0.5f / freq;
        float half_period_sample_count = 44100.0f * half_period_time;
        int half_period_count = (int)ceilf(len / half_period_time);
        half_period_count = (half_period_count + 1) / 2 * 2;
        int16_t amplitude = INT16_MIN + (int16_t)(2.0f * (float)INT16_MAX * volume);
        int current = 0;
        for (int i = 0; i < half_period_count; i++) {
            int next = (int)((float)(i + 1) * half_period_sample_count);
            int16_t vi = ((i % 2) == 0) ? amplitude : INT16_MIN;
            for (int j = current; j < next; j++) {
                push_sample(vi);
            }
            current = next;
        }
    }
};
