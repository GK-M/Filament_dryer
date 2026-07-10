#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "tasks/display_task.h"
#include "rtos_handles.h"
#include "config.h"
#include "tasks/log_task.h"


void vDisplayTask(void *pvParameters) {

    I2C_sensors i2c_sensors;
    DS_sensors ds_sensors;
    Control_status control_status;
    Timer_data timer_data;
    Display_data display_data = {Mode::Mode_1, Screen::Main, EditVar::Setpoint};

    PID_data pid_data;

    static LiquidCrystal_I2C lcd(0x27, 20, 4);
    lcd.init();
    lcd.backlight();

    lcd.createChar(0, Stopnie);
    lcd.createChar(1, Dzwonek);

    for (;;) {

        xQueuePeek(xI2CsensorsQueue,   &i2c_sensors,   pdMS_TO_TICKS(Timing::Display_data_timeout));
        xQueuePeek(xDS18B20Queue,       &ds_sensors,    pdMS_TO_TICKS(Timing::Display_data_timeout));
        xQueuePeek(xControlDataQueue,   &control_status,pdMS_TO_TICKS(Timing::Display_data_timeout));
        xQueuePeek(xTimerQueue,         &timer_data,    pdMS_TO_TICKS(Timing::Display_data_timeout));
        xQueuePeek(xSetpointQueue,      &pid_data,      pdMS_TO_TICKS(Timing::Display_data_timeout));
        xQueuePeek(xDisplayQueue,       &display_data,  pdMS_TO_TICKS(Timing::Display_data_timeout));

        xSemaphoreTake(xI2CMutex, portMAX_DELAY);
        switch (display_data.screen) {

        case Screen::Main: {
            // Row 0: T.SET: XX°  →  XX.X°
            lcd.setCursor(0, 0);
            lcd.printf("T.SET:%3.0f", control_status.Setpoint);
            lcd.write(byte(0));
            lcd.printf(" %c %4.1f", (char)126, i2c_sensors.temp_aht);
            lcd.write(byte(0));
            lcd.print("  ");

            // Row 1: Bell  HH:MM  →  HH:MM:SS
            lcd.setCursor(0, 1);
            lcd.print(" ");
            lcd.write(byte(1));
            lcd.printf(" %02lu:%02lu %c ", timer_data.SetCzasGodz, timer_data.SetCzasMin, (char)126);
            lcd.printf("%02lu:%02lu:%02lu", timer_data.StoperCzasGodz, timer_data.StoperCzasMin, timer_data.StoperCzasSek);

            // Row 2: DS1 i DS3
            lcd.setCursor(0, 2);
            lcd.printf("DS1:%3.0f%cC DS3:%3.0f%cC", ds_sensors.DS_1, (char)223, ds_sensors.DS_3, (char)223);

            // Row 3: Wilgotnosc
            lcd.setCursor(0, 3);
            lcd.printf("Humidity: %.1f%%    ", i2c_sensors.hum_aht);
            break;
        }

        case Screen::Sensors_data: {
            // Row 0
            lcd.setCursor(0, 0);
            lcd.print("Dane z czujnikow:   ");

            // Row 1: AHT10
            lcd.setCursor(0, 1);
            lcd.printf("T_aht:%4.1f", i2c_sensors.temp_aht);
            lcd.write(byte(0));
            lcd.printf(" H:%.1f%%  ", i2c_sensors.hum_aht);

            // Row 2: BMP280
            lcd.setCursor(0, 2);
            lcd.printf("T_bmp:%4.1f", i2c_sensors.t_bmp);
            lcd.write(byte(0));
            lcd.printf(" P:%.0fhPa", i2c_sensors.p_hPa);

            // Row 3: DS18B20
            lcd.setCursor(0, 3);
            lcd.printf("T1:%2.0f%c T2:%2.0f%c T3:%2.0f%c",
                ds_sensors.DS_1, (char)223,
                ds_sensors.DS_2, (char)223,
                ds_sensors.DS_3, (char)223);
            break;
        }

        case Screen::PID_cook: {
            char kp_sel = (display_data.editvar == EditVar::Kp) ? '>' : ' ';
            char ki_sel = (display_data.editvar == EditVar::Ki) ? '>' : ' ';
            char kd_sel = (display_data.editvar == EditVar::Kd) ? '>' : ' ';

            // Row 0
            lcd.setCursor(0, 0);
            lcd.print("     PID (Cook)     ");

            // Row 1: Kp i Ki
            lcd.setCursor(0, 1);
            lcd.printf("%cKp:%-5.2f %cKi:%-5.3f", kp_sel, pid_data.Kp, ki_sel, pid_data.Ki);

            // Row 2: Kd
            lcd.setCursor(0, 2);
            lcd.printf("%cKd:%-5.3f           ", kd_sel, pid_data.Kd);

            // Row 3: Setpoint i Output
            lcd.setCursor(0, 3);
            lcd.printf(" Set:%.1f", control_status.Setpoint);
            lcd.write(byte(0));
            lcd.printf("  Out:%.0f%%  ", control_status.Output);
            break;
        }

        case Screen::PID_fan: {
            lcd.setCursor(0, 0);
            lcd.print("      PID (Fan)     ");
            lcd.setCursor(0, 1);
            lcd.print("  (nie zaimpl.)     ");
            lcd.setCursor(0, 2);
            lcd.print("                    ");
            lcd.setCursor(0, 3);
            lcd.print("                    ");
            break;
        }

        default:
            break;
        }
        xSemaphoreGive(xI2CMutex);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
