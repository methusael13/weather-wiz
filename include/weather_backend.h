/*
An Indicator to display local weather information

The MIT License (MIT)

Copyright (c) 2015 Methusael Murmu
Authors:
    Methusael Murmu <blendit07@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _WEATHER_BACKEND_
#define _WEATHER_BACKEND_

#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>

typedef enum W_STATUS {
    WEATHER_CLEAR,              // clear, sunny, mostlysunny
    WEATHER_CLEAR_NIGHT,        // nt_clear, nt_sunny, nt_mostlysunny
    WEATHER_CLOUDS,             // cloudy, mostlycloudy
    WEATHER_CLOUDS_NIGHT,       // nt_cloudy, nt_mostlycloudy
    WEATHER_FEW_CLOUDS,         // partlycloudy, partlysunny
    WEATHER_FEW_CLOUDS_NIGHT,   // nt_partlycloudy, nt_partlysunny
    WEATHER_FOG,                // fog, nt_fog
    WEATHER_OVERCAST,           // hazy, nt_hazy
    WEATHER_SEVERE_ALERT,       // [no available binding]
    WEATHER_SHOWERS,            // rain, nt_rain
    WEATHER_SHOWERS_SCATTERED,  // chancerain
    WEATHER_SNOW,               // snow, flurries, slit, nt_snow, nt_flurries, nt_slit
    WEATHER_STORM,              // tstorms, nt_tstorms
    WEATHER_UNKNOWN				// [default binding]
} WEATHER_STATUS;

#ifdef _REQUIRE_WEATHER_DEFINITIONS_
    const char *weather_const_str[] = {
        "chancerain", "clear",
        "cloudy", "flurries",
        "fog", "hazy", "mostlycloudy",
        "mostlysunny", "partlycloudy",
        "partlysunny", "rain", "slit",
        "snow", "sunny", "tstorms"
    };

    const WEATHER_STATUS weather_const_id[] = {
        WEATHER_SHOWERS_SCATTERED, WEATHER_CLEAR,
        WEATHER_CLOUDS, WEATHER_SNOW,
        WEATHER_FOG, WEATHER_OVERCAST, WEATHER_CLOUDS,
        WEATHER_CLEAR, WEATHER_FEW_CLOUDS,
        WEATHER_FEW_CLOUDS, WEATHER_SHOWERS, WEATHER_SNOW,
        WEATHER_SNOW, WEATHER_CLEAR, WEATHER_STORM
    };
#endif

#ifdef _REQUIRE_W_STATUS_STR_
    /* For testing  purposes only */
    const char *w_status_str[] = {
        "WEATHER_CLEAR", "WEATHER_CLEAR_NIGHT", "WEATHER_CLOUDS",
        "WEATHER_CLOUDS_NIGHT", "WEATHER_FEW_CLOUDS",
        "WEATHER_FEW_CLOUDS_NIGHT",
        "WEATHER_FOG", "WEATHER_OVERCAST", "WEATHER_SEVERE_ALERT",
        "WEATHER_SHOWERS", "WEATHER_SHOWERS_SCATTERED", "WEATHER_SNOW",
        "WEATHER_STORM", "WEATHER_UNKNOWN"
    };
#define W_STATUS_STR(i) w_status_str[i]
#endif

#define FARENHEIT_UNIT_STR "°F"
#define CELSIUS_UNIT_STR "°C"

/* Size definitions for GeoIP and WeatherCond structs */
#define W_LOC_STR_SIZE 30
#define W_LOC_ZMW_SIZE 20
#define W_WIND_DIR_STR_SIZE 10
#define W_WEATHER_STR_SIZE 40

typedef struct GeoIPLoc {
    int  use_zmw;
    char city[W_LOC_STR_SIZE];
    char state[W_LOC_STR_SIZE];
    char country[W_LOC_STR_SIZE];
    char zmw[W_LOC_ZMW_SIZE];
} GeoIPLocation;

#define W_NUMBER_STR_SIZE 6
#define W_TEMP_STR_SIZE 9

typedef struct WeatherCond {
    char temp_f[W_TEMP_STR_SIZE];
    char temp_c[W_TEMP_STR_SIZE];
    char rel_humidity[W_NUMBER_STR_SIZE];
    char wind_mph[W_NUMBER_STR_SIZE];
    char wind_kph[W_NUMBER_STR_SIZE];
    char pressure_mb[W_NUMBER_STR_SIZE];
    char wind_dir[W_WIND_DIR_STR_SIZE];
    char weather_str[W_WEATHER_STR_SIZE];
    WEATHER_STATUS weather_id;
} WeatherConditions;

#define W_WEATHER_DISP_LOC_SZ 50
#define W_WEATHER_DISP_TEMP_SZ 25
#define W_WEATHER_DISP_RH_SZ 30
#define W_WEATHER_DISP_WIND_SZ 30
#define W_WEATHER_DISP_PRE_SZ 20

typedef struct WeatherD {
    int use_temp_f;
    int use_wind_mph;
    int display_label;
    char location_d[W_WEATHER_DISP_LOC_SZ];
    char temp_f_d[W_WEATHER_DISP_TEMP_SZ];
    char temp_c_d[W_WEATHER_DISP_TEMP_SZ];
    char rel_humidity_d[W_WEATHER_DISP_RH_SZ];
    char wind_mph_d[W_WEATHER_DISP_WIND_SZ];
    char wind_kph_d[W_WEATHER_DISP_WIND_SZ];
    char pressure_mb_d[W_WEATHER_DISP_PRE_SZ];
    char weather_d[W_WEATHER_STR_SIZE];
} WeatherDisplayable;

#define W_SERVICE_STARTED 1
#define W_CONNECTION_OK 2
#define W_CONNECTION_ACTIVE 4
#define W_LOC_DATA_AVAILABLE 8
#define W_WEATR_DATA_AVAILABLE 16

#define W_IS_SERVICE_STARTED(status) (status & W_SERVICE_STARTED)
#define W_IS_CONNECTION_OK(status) (status & W_CONNECTION_OK)
#define W_IS_CONNECTION_ACTIVE(status) (status & W_CONNECTION_ACTIVE)
#define W_IS_LOC_DATA_AVAILABLE(status) (status & W_LOC_DATA_AVAILABLE)
#define W_IS_WEATR_DATA_AVAILABLE(status) (status & W_WEATR_DATA_AVAILABLE)

/* Gets weather service status 
 * The returned integer is a combination of the following bit flags:
 *     W_WEATR_DATE_AVAILABLE(16)   |
 *     W_LOC_DATA_AVAILABLE(8)      |
 *     W_CONNECTION_ACTIVE(4)       |
 *     W_CONNECTION_OK(2)           |
 *     W_SERVICE_STARTED(1)
 */
int
w_get_service_status(void);

/* Inititiates the GeoIPLocation struct using data from curl,
 * and starts the weather backend service.
 */
void
w_start_weather_service(void);

/* Sets the callback function to be notified on weather update
 * events. The wc struct is filled with necessary data from 
 * the weather API.
 */
void
w_set_weather_update_listener(
    void (*on_weather_update)(GeoIPLocation *loc, WeatherConditions *wc));

/* Cleans up any active references and stops weather service */
void
w_stop_weather_service(void);

void
w_refresh_weather_data(void);

void
w_weather_get_displayable(WeatherDisplayable *wd,
    GeoIPLocation *loc,
    WeatherConditions *wc);

#endif
