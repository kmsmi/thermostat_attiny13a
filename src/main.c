#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "ds18b20.h"
#include "uart.h"

#define H_ID 0xBCFA // ID 0xBC: Датчик температуры, 0xFA: Комнатный, 0xF0: Уличный

#define SEND_PERIOD 30 // Период между отправками данных в секундах
#define SEND_LONG_CNT 600 // Период между отправками данных в секундах
#define RF_POWER PB2   // Пин для Вкл/Откл HC-12
#define BAT PB3        // Пин отслеживания питания батареек

#define RF_POWER_ON PORTB |= (1 << RF_POWER)
#define RF_POWER_OFF PORTB &= ~(1 << RF_POWER)

// #define WDT_INTERRUPT_ENABLE WDTCR |= (1 << WDTIE)

struct data // Структура данных для отправки по радиоканалу
{
    uint16_t th_id; // ID что бы приемник знал от кого принимает данные
    uint16_t temp; // Температура
    bool bat; // Батарея
} tx_data = {H_ID, 0, false};

volatile uint8_t send_cnt = 0;
volatile uint16_t send_long_cnt = 0;

void send();

ISR(WDT_vect) {
    send_cnt++;
    send_long_cnt++;
}

void rf_off() {
    _delay_ms(50);
    RF_POWER_OFF; // Выключаем HC-12
}

//
void rf_on() {
    RF_POWER_ON; // Включаем HC-12
    _delay_ms(50);
}

void send() {
    rf_on();
    uart_write((uint8_t *) &tx_data, sizeof(tx_data)); // Отправяем по uart
    rf_off();
}

int main() {
    ADCSRA &= ~(1 << ADEN); // выкл ацп
    ACSR &= ~(1 << ACIE);
    // Запрещаем прерывание аналогового компаратора. Иначе при изменении бита ACD произойдет прерывание.
    ACSR |= (1 << ACD); // выкл компаратор
    PRR |= (1 << PRADC); // Запрещаем АЦП

    DDRB &= ~(1 << BAT); // Пин PB3 устанавливаем как вход
    PORTB |= (1 << BAT); // и подтягиваем их к питанию через внутренний резистор(PULL_UP)

    DDRB |= (1 << RF_POWER); // Пин для питания HC-12 устанваливавем как выход

    DDRB &= ~(1 << PB1); // Неиспользуемый вывод устанавливаем как вход(на всякий случай)
    PORTB |= (1 << PB1); // и подтягиваем к питанию через внутренний резистор(PULL_UP)

    RF_POWER_OFF; // Выключаем HC-12

    uart_init(); // Инициализация UART
    setResolution(TEMP_11_BIT); // Устанавливаем разрешение дачика 11 бит
    requestTemp(); // Запрос датчику для чтения температуры

    // инициализация ватчдога
    wdt_reset(); // сбрасываем
    wdt_enable(WDTO_1S); // разрешаем ватчдог 1 сек
    WDTCR |= (1 << WDTIE); // Разрешаем прерывания по ватчдогу. Иначе будет резет.
    sei(); // разрешаем прерывания глобально

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Режим сна максимальный

    // ReSharper disable once CppDFAEndlessLoop
    while (1) {
        if (send_cnt == 1) {
            uint16_t t = tx_data.temp;
            bool b = tx_data.bat;
            tx_data.temp = readTemp(false); // Температура
            tx_data.bat = PINB & (1 << BAT); // Батарея
            // Отправляем только если данные изменились или прошло 10 мин. с последней отправки
            if (!(tx_data.temp == t && tx_data.bat == b) || send_long_cnt >= SEND_LONG_CNT) {
                send_long_cnt = 0;
                send(); // Отправляем
            }
        } else if (send_cnt >= SEND_PERIOD) {
            requestTemp(); // Запрос датчику для чтения температуры
            send_cnt = 0;
        }

        WDTCR |= (1 << WDTIE); // Разрешаем прерывания по ватчдогу. Иначе будет резет.
        sleep_enable(); // разрешаем сон
        sleep_bod_disable(); // отключаем BOD во время сна.
        sleep_cpu(); // спать!
    }
}
