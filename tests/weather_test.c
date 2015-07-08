#include <stdio.h>
#include <unistd.h>
#define _REQUIRE_W_STATUS_STR_
#include "../include/weather_backend.h"

int should_exit = 0;

void weather_info(GeoIPLocation loc, WeatherConditions wc) {
    printf("Weather info received:\n");
    printf("Location: %s, %s\n", loc.city, loc.country);
    printf("\tTemp(F): %.1f\n", wc.temp_f);
    printf("\tTemp(C): %.1f\n", wc.temp_c);
    printf("\tRelative Humidity: %.1f%%\n", wc.rel_humidity);
    printf("\tWind(mph): %.1f mph\n", wc.wind_mph);
    printf("\tWind(kph): %.1f kph\n", wc.wind_kph);
    printf("\tPressure: %.0f mb\n", wc.pressure_mb);
    printf("\tWeather: %s\n", wc.weather_str);
    printf("\tW_STATUS: %s\n", W_STATUS_STR(wc.weather_id));

    should_exit++;
}

int main(int argc, char *argv[]) {
    w_set_weather_update_listener(weather_info);
    w_start_weather_service();

    while (should_exit < 1)
        usleep(10);
    w_stop_weather_service();

    return 0;
}
