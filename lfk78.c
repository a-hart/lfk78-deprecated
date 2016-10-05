#include "lfk78.h"
#include "issi.h"
#include <avr/sfr_defs.h>
#include "audio/audio.h"
#include "TWIlib.h"
#include "avr/timer_avr.h"
// #include "audio/song_list.h"

#ifdef AUDIO_ENABLE
float test_sound[][2] = SONG(ODE_TO_JOY);
#endif


uint8_t under_red = 0;
uint8_t under_green = 0;
uint8_t under_blue = 0;
uint8_t backlight = 0;
uint8_t led_toggle = 1;

uint16_t click_hz = 500;
uint16_t click_time = 2;
uint8_t click_toggle = 0;


void matrix_init_kb(void)
{
    // put your keyboard start-up code here
    // runs once when the firmware starts up

    matrix_init_user();

    // Set up 16 bit PWM: Fast PWM, mode 15, inverted
    TCCR1A = 0b11111110;
    TCCR1B = 0b00011001;
    ICR1 = 0xFFFF;
    // PWM values - 0xFFFF = off, 0x0000 = max
    OCR1C = 0x0000; // B7 - Blue
    OCR1B = 0x0000; // B6 - Green
    OCR1A = 0x0FFF; // B5 - Red
    // Set as output
    DDRB |= 0b11100000;

#ifdef AUDIO_ENABLE
    audio_init();
    // PLAY_NOTE_ARRAY(test_sound, false, 0);
#else
    // If we're not using the audio pin, drive it low
    sbi(DDRC, 6);
    cbi(PORTC, 6);
#endif

#ifdef ISSI_ENABLE
    issi_init();
    // led_test();
    set_underglow(255, 255, 255);
#endif
}

void matrix_scan_kb(void)
{
#ifdef ISSI_ENABLE
    // switch/underglow lighting update
    static uint32_t issi_device = 0;
    if(isTWIReady()){
        // If the i2c bus is available, kick off the issi update, alternate between devices
        update_issi(issi_device, 0);
        if(issi_device){
            issi_device = 0;
        }else{
            issi_device = 3;
        }
    }
#endif
    // Update layer indicator LED
    //
    // Not sure how else to reliably do this... TMK has the 'hook_layer_change'
    // but can't find QMK equiv
    static uint32_t layer_indicator = -1;
    if(layer_indicator != layer_state){
        layer_indicator = layer_state;
        if(layer_state==0){
            OCR1A = 0x0000; // B5 - Red
            OCR1B = 0x0FFF; // B6 - Green
            OCR1C = 0x0000; // B7 - Blue
        }else if(layer_state & 0x04){
            OCR1A = 0x0FFF; // B5 - Red
            OCR1B = 0x0000; // B6 - Green
            OCR1C = 0x07FF; // B7 - Blue
        }else if(layer_state & 0x02){
            OCR1A = 0x0000; // B5 - Red
            OCR1B = 0x0000; // B6 - Green
            OCR1C = 0x0FFF; // B7 - Blue
        }else{
            xprintf("unknown layer: %02X\n", layer_state);
            OCR1A = 0x0FFF; // B5 - Red
            OCR1B = 0x0FFF; // B6 - Green
            OCR1C = 0x0FFF; // B7 - Blue
        }
    }

    matrix_scan_user();
}

void click(uint16_t freq, uint16_t duration){
    if(freq >= 100 && freq <= 20000 && duration < 100){
        play_note(freq, 10);
        for (uint16_t i = 0; i < duration; i++){
            _delay_ms(1);
        }
        stop_all_notes();
    }
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record)
{
    // Test code that turns on the switch led for the key that is pressed
    //
    // xprintf("event: %d %d\n", record->event.key.col, record->event.key.row);
    // uint8_t lookup_value = switch_leds[record->event.key.row][record->event.key.col];
    // uint8_t matrix = 0;
    // if(lookup_value & 0x08){
    //     matrix = 6;
    //     issi_devices[3]->led_dirty = 1;
    // }else{
    //     issi_devices[0]->led_dirty = 1;
    // }
    // uint8_t col = (lookup_value & 0xF0) >> 4;
    // uint8_t row = lookup_value & 0x07;
    // xprintf("LED: %02X, %d %d %d\n", lookup_value, matrix, col, row);
    // activateLED(matrix, col, row, 255);

    if (click_toggle && record->event.pressed){
        click(click_hz, click_time);
    }
    if (keycode == RESET) {
        reset_keyboard_kb();
    } else {
    }
    return process_record_user(keycode, record);
}

void action_function(keyrecord_t *event, uint8_t id, uint8_t opt)
{
    int8_t sign = 1;
    xprintf("action_function: %d, opt: %02X\n", id, opt);
    if(event->event.pressed){
        switch(id){
            case LFK_CLEAR:
                // Go back to default layer
                layer_clear();
                break;
            case LFK_LED_TOGGLE:
                if(led_toggle == 0){
                    led_toggle = 1;
                    set_backlight(backlight);
                    set_underglow(under_red, under_green, under_blue);
                }else{
                    led_toggle = 0;
                    set_backlight(0);
                    set_underglow(0, 0, 0);
                }
                issi_devices[0]->led_dirty = 1;
                issi_devices[3]->led_dirty = 1;
                break;
            case LFK_LED_DOWN:
                sign = -1;  // continue to next statement
            case LFK_LED_UP:
                // Change LEDs
                //  opt 0 : toggle all LEDs on/off
                //  opt -1 or 1 : increase or decrease brightness
                //  mods:
                //      None-    backlight
                //      Control- Red
                //      Alt-     Green
                //      Gui-     Blue
                led_toggle = 1;
                uint8_t mods = get_mods();
                if(mods == 0){
                    backlight += 32 * sign;
                    set_backlight(backlight);
                }else{
                    if(mods & MOD_LCTL){
                        under_red += 32 * sign;
                    }
                    if(mods & MOD_LALT){
                        under_green += 32 * sign;
                    }
                    if(mods & MOD_LGUI){
                        under_blue += 32 * sign;
                    }
                    set_underglow(under_red, under_green, under_blue);
                }
                issi_devices[0]->led_dirty = 1;
                issi_devices[3]->led_dirty = 1;
                break;
            case LFK_CLICK_FREQ_LOWER:
                sign = -1;  // continue to next statement
            case LFK_CLICK_FREQ_HIGHER:
                click_hz += sign * 100;
                xprintf("click_hz: %d\n", click_hz);
                click(click_hz, click_time);
                break;
            case LFK_CLICK_TOGGLE:
                if(click_toggle){
                    click_toggle = 0;
                    click(4000, 100);
                    click(1000, 100);
                }else{
                    click_toggle = 1;
                    click(1000, 100);
                    click(4000, 100);
                }
                break;
            case LFK_CLICK_TIME_SHORTER:
                sign = -1;  // continue to next statement
            case LFK_CLICK_TIME_LONGER:
                click_time += sign;
                xprintf("click_time: %d\n", click_time);
                click(click_hz, click_time);
                break;
            case LFK_DEBUG_SETTINGS:
                xprintf("Click:\n");
                xprintf("  toggle: %d\n", click_toggle);
                xprintf("  freq(hz): %d\n", click_hz);
                xprintf("  duration(ms): %d\n", click_time);
                break;
            case LFK_LED_TEST:
                led_test();
                break;
        }
    }
}

void reset_keyboard_kb(){
    xprintf("programming!\n");
    OCR1A = 0x0000; // B5 - Red
    OCR1B = 0x0FFF; // B6 - Green
    OCR1C = 0x0FFF; // B7 - Blue
    reset_keyboard();
}

void led_set_kb(uint8_t usb_led)
{
    // put your keyboard LED indicator (ex: Caps Lock LED) toggling code here

    led_set_user(usb_led);
}
