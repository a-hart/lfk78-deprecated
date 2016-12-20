#include "lfk78_revc.h"
#include <avr/sfr_defs.h>
#include "avr/timer_avr.h"


void matrix_init_kb(void)
{
    // eeprom_update_word(0, 0xFF);
    xprintf("1- init\n");
    // _delay_ms(250);
    // xprintf("2- matrix_init_kb\n");
    // _delay_ms(1000);
    // xprintf("3- matrix_init_kb\n");
    // _delay_ms(1000);
    // xprintf("4- matrix_init_kb\n");

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

    for(int x=0; x < 10; x++){
        xprintf("1- init\n");
        _delay_ms(100);
        switch(x % 3){
            case 0:
                OCR1C = 0x0000; // B7 - Blue
                OCR1B = 0x0FFF; // B6 - Green
                OCR1A = 0x0000; // B5 - Red
                break;
            case 1:
                OCR1C = 0x0FFF; // B7 - Blue
                OCR1B = 0x0000; // B6 - Green
                OCR1A = 0x0000; // B5 - Red
                break;
            case 2:
                OCR1C = 0x0000; // B7 - Blue
                OCR1B = 0x0000; // B6 - Green
                OCR1A = 0x0FFF; // B5 - Red
                break;
        }
    }
    OCR1C = 0x0FFF; // B7 - Blue
    OCR1B = 0x0FFF; // B6 - Green
    OCR1A = 0x0FFF; // B5 - Red

}

void matrix_scan_kb(void)
{
    OCR1C = 0x0FFF; // B7 - Blue
    OCR1B = 0x0000; // B6 - Green
    OCR1A = 0x0000; // B5 - Red
    _delay_ms(500);
    xprintf("matrix_scan_kb\n");
    // Update layer indicator LED
    //
    // Not sure how else to reliably do this... TMK has the 'hook_layer_change'
    // but can't find QMK equiv
    // static uint32_t layer_indicator = -1;
    // if(layer_indicator != layer_state){
    //     layer_indicator = layer_state;
    //     if(layer_state==0){
    //         OCR1A = 0x0000; // B5 - Red
    //         OCR1B = 0x0FFF; // B6 - Green
    //         OCR1C = 0x0000; // B7 - Blue
    //     }else if(layer_state & 0x04){
    //         OCR1A = 0x0FFF; // B5 - Red
    //         OCR1B = 0x0000; // B6 - Green
    //         OCR1C = 0x07FF; // B7 - Blue
    //     }else if(layer_state & 0x02){
    //         OCR1A = 0x0000; // B5 - Red
    //         OCR1B = 0x0000; // B6 - Green
    //         OCR1C = 0x0FFF; // B7 - Blue
    //     }else{
    //         xprintf("unknown layer: %02X\n", layer_state);
    //         OCR1A = 0x0FFF; // B5 - Red
    //         OCR1B = 0x0FFF; // B6 - Green
    //         OCR1C = 0x0FFF; // B7 - Blue
    //     }
    // }

    matrix_scan_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record)
{
    // Test code that turns on the switch led for the key that is pressed
    //
    xprintf("event: %d %d\n", record->event.key.col, record->event.key.row);
    if (keycode == RESET) {
        reset_keyboard_kb();
    } else {
    }
    return process_record_user(keycode, record);
}

void action_function(keyrecord_t *event, uint8_t id, uint8_t opt)
{

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
