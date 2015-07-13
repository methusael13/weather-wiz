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

#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <curl/curl.h>
#include <sys/types.h>
#include "util.h"
#include "event_log.h"
#include "timer_thread.h"
#include "json_util.h"

#define _REQUIRE_WEATHER_DEFINITIONS_
#include "weather_backend.h"

#define REQUEST_INTERVAL 3600
#define TEMP_LOC_FILE "w_geo_loc.json"
#define TEMP_WEATHER_FILE "w_weather_cond.json"

#define PATH_FORMAT "%s/.cache/%s"
#define LOC_URL_FORMAT \
    "http://api.wunderground.com/api/%s/geolookup/q/autoip.json"
#define WEATHER_URL_FORMAT_CC \
    "http://api.wunderground.com/api/%s/geolookup/conditions/q/%s/%s.json"
#define WEATHER_URL_FORMAT_ZMW \
    "http://api.wunderground.com/api/%s/geolookup/conditions/q/zmw:%s.json"

#define W_BUILD_LOCATION_URL(str, auth_key) \
    do { snprintf(str, sizeof(str), LOC_URL_FORMAT, auth_key); } \
    while (0);
#define W_BUILD_WEATHER_URL_CC(str, auth_key, country, city) \
    do { snprintf(str, sizeof(str), WEATHER_URL_FORMAT_CC, auth_key, country, city); } \
    while (0);
#define W_BUILD_WEATHER_URL_ZMW(str, auth_key, zmw) \
    do { snprintf(str, sizeof(str), WEATHER_URL_FORMAT_ZMW, auth_key, zmw); } \
    while (0);

#define INIT_LOC_FILE_PATH(str, home) \
    do { snprintf(str, sizeof(str), PATH_FORMAT, home, TEMP_LOC_FILE); } \
    while (0);
#define INIT_WEATHER_FILE_PATH(str, home) \
    do { snprintf(str, sizeof(str), PATH_FORMAT, home, TEMP_WEATHER_FILE); } \
    while (0);

/* flag should be either TRUE or FALSE */
#define W_SET_BIT_FLAG(flag, bitmask, data) \
    { data = (flag ? (data | bitmask) : (data & (~flag))); }

#define W_SET_SERVICE_STARTED(flag) \
    W_SET_BIT_FLAG(flag, W_SERVICE_STARTED, status)
#define W_SET_CONNECTION_OK(flag) \
    W_SET_BIT_FLAG(flag, W_CONNECTION_OK, status)
#define W_SET_CONNECTION_ACTIVE(flag) \
    W_SET_BIT_FLAG(flag, W_CONNECTION_ACTIVE, status)
#define W_SET_LOC_DATA_AVAILABLE(flag) \
    W_SET_BIT_FLAG(flag, W_LOC_DATA_AVAILABLE, status)
#define W_SET_WEATR_DATA_AVAILABLE(flag) \
    W_SET_BIT_FLAG(flag, W_WEATR_DATA_AVAILABLE, status)

static uint8_t status = 0;
static time_t last_update_time;
static const char *usr_home;
/* Keep this safe, lest it fall in the hands of the evil */
static const char *AUTH_KEY  = "208c2e7af8134895";

static TimerThread *timer;
static WeatherConditions wc;
static GeoIPLocation geo_loc = { .use_zmw = FALSE };

void (*weather_update_callback)(GeoIPLocation *, WeatherConditions *);

struct response_data {
    int16_t cursor;
    char data[BUFFER_SZ];
};

static char *geoip_text = NULL;
static char *weather_text = NULL;
static char loc_file_path[PATH_SIZE];
static char weather_file_path[PATH_SIZE];
static char loc_url[URL_SIZE];
static char weather_url[URL_SIZE];

static int
parse_weather_json_data(const char *, WeatherConditions *);
static int
parse_location_json_data(const char *, GeoIPLocation *);

static size_t
write_response(char *ptr, size_t size, size_t nmemb, void *out_stream) {
    struct response_data *response = (struct response_data *)out_stream;

    if (response->cursor + size*nmemb >= BUFFER_SZ - 1) {
        LOG_ERROR("Buffer overflow while writing response");
        return 0;
    }

    memcpy(response->data + response->cursor, ptr, size * nmemb);
    response->cursor += size * nmemb;

    return size * nmemb;
}

static int
w_request(const char *url, char *dest) {
    CHECK_REF(url, err);

    CURL *curl = NULL;
    int16_t http_status;
    CURLcode curl_status;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    CHECK_REF(curl, err);

    struct response_data response = { .cursor = 0 };
    memset(response.data, 0, BUFFER_SZ);

    /* Set Curl options */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_status = curl_easy_perform(curl);
    if (curl_status != 0) {
#ifdef __W_DEBUG__
        LOG_ERROR("Unable to fetch data from internet");
        LOG_ARG_ERROR("%s", curl_easy_strerror(curl_status));
#endif
        goto err;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    if (http_status != HTTP_STATUS_OK) {
#ifdef __W_DEBUG__
        LOG_ARG_ERROR("Server replied with error code: %d", http_status);
#endif
        goto err;
    }

    response.data[response.cursor] = '\0';
    memcpy(dest, response.data, response.cursor + 1);

    return TRUE;
    err:
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();
        return FALSE;
}

static int
w_init_geoip_loc(GeoIPLocation *geo) {
    int pass = FALSE;
    int online = TRUE;
    /* If cache file exists, init geo_loc with cache data */
    if (file_exists_readable(loc_file_path)) {
        get_str_from_path(&geoip_text, BUFFER_SZ, loc_file_path);
        pass = parse_location_json_data(geoip_text, geo);
        if (!pass)
            LOG_WARNING("Unable to parse geoip data from json cache");
    } else {
        LOG_WARNING("Unable to load geoip cache data");
    }

    W_BUILD_LOCATION_URL(loc_url, AUTH_KEY);
    online = w_request(loc_url, geoip_text);
    if (!online && !pass) {
        W_SET_LOC_DATA_AVAILABLE(FALSE);
        return FALSE;
    }

    /* Update location info */
    if (online) {
        parse_location_json_data(geoip_text, geo);

        /* Determine whether to use zmw
           Try first with default weather url */
        W_BUILD_WEATHER_URL_CC(weather_url, AUTH_KEY,
            geo->country, geo->city);
        if (w_request(weather_url, weather_text)) {
            if (!parse_weather_json_data(weather_text, &wc)) {
                /* Unable to parse response json
                   Should have zmw data */
                json_t *root, *res;
                json_error_t j_error;

                root = json_loads(weather_text, 0, &j_error);
                res = json_get_leaf_object(root, "response.results");

                if (json_is_array(res))
                    res = json_array_get(res, 0);
                json_load_data_string(res, "zmw", geo->zmw, sizeof geo->zmw);
                geo->use_zmw = TRUE;
                json_decref(root);
            }
        }
    }
    W_SET_LOC_DATA_AVAILABLE(TRUE);
    return TRUE;
}

static int
w_load_weather_data_online(WeatherConditions *wc) {
    if (geo_loc.use_zmw) {
        W_BUILD_WEATHER_URL_ZMW(weather_url, AUTH_KEY, geo_loc.zmw);
    } else {
        W_BUILD_WEATHER_URL_CC(weather_url, AUTH_KEY,
            geo_loc.country, geo_loc.city);
    }

    if (w_request(weather_url, weather_text)) {
        parse_weather_json_data(weather_text, wc);
        W_SET_WEATR_DATA_AVAILABLE(TRUE);

        return TRUE;
    }

    return FALSE;
}

static int
w_load_weather_data_cache(WeatherConditions *wc) {
    int pass = FALSE;

    /* If cache file exists, init WeatherCondition with cache data */
    if (file_exists_readable(weather_file_path)) {
        get_str_from_path(&weather_text, BUFFER_SZ, weather_file_path);
        pass = parse_weather_json_data(weather_text, wc);
        if (!pass)
            LOG_WARNING("Unable to parse weather data from cache");
    } else {
        LOG_WARNING("Unable to load weather cache data");
    }

    W_SET_WEATR_DATA_AVAILABLE(pass);
    return pass;
}

void
w_refresh_weather_data(void) {
    if (!W_IS_LOC_DATA_AVAILABLE(status) && !w_init_geoip_loc(&geo_loc)) {
        LOG_WARNING("Failed to detect location. "
            "Check internet connection");
        return;
    }

    if (!W_IS_WEATR_DATA_AVAILABLE(status))
        if (w_load_weather_data_cache(&wc))
            weather_update_callback(&geo_loc, &wc);

    time_t curr_t = time((time_t *)0);
    if (difftime(curr_t, last_update_time) > REQUEST_INTERVAL) {
        if (w_load_weather_data_online(&wc)) {
            last_update_time = curr_t;
            weather_update_callback(&geo_loc, &wc);
        }
    }

    if (!W_IS_WEATR_DATA_AVAILABLE(status))
        LOG_WARNING("Failed to update weather. "
            "Check internet connection");
}

int
w_get_service_status(void) {
    return status;
}

void
w_start_weather_service(void) {
    if (W_IS_SERVICE_STARTED(status)) {
        LOG_WARNING("Weather service is already running");
        return;
    }

    /* Init user home directory */
    usr_home = get_usr_home_dir();
    INIT_LOC_FILE_PATH(loc_file_path, usr_home);
    INIT_WEATHER_FILE_PATH(weather_file_path, usr_home);

    geoip_text = (char *) malloc(BUFFER_SZ);
    weather_text = (char *) malloc(BUFFER_SZ);

    struct tm init_t = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0,
        .tm_mday = 1, .tm_mon = 0, .tm_year = 110
    };
    last_update_time = mktime(&init_t);

    timer = timer_thread_create(1, 5, w_refresh_weather_data);
    timer_thread_start(timer);
    W_SET_SERVICE_STARTED(TRUE);

    LOG_INFO("Weather service up and running!");
}

void
w_set_weather_update_listener(
    void (*on_weather_update)(GeoIPLocation *loc, WeatherConditions *wc)) {
    weather_update_callback = on_weather_update;
    LOG_INFO("Client registered for weather update callbacks");
}

void
w_stop_weather_service(void) {
    if (!W_IS_SERVICE_STARTED(status))
        return;

    timer_thread_stop(timer);
    timer_thread_destroy(timer);

    /* Cache the weather and location data */
    int res;
    res = write_str_to_file_path(geoip_text, loc_file_path);
    if (res < -1)
        LOG_ERROR("Error caching location data");
    res = write_str_to_file_path(weather_text, weather_file_path);
    if (res < -1)
        LOG_ERROR("Error caching weather data");

    free(geoip_text);
    free(weather_text);
    status = 0;

    LOG_INFO("Weather service successfully stopped!");
}

#ifdef _REQUIRE_WEATHER_DEFINITIONS_
int
w_str_cmp(const void *arg1, const void *arg2) {
    return strcmp((const char *)arg1, *(const char * const *)arg2);
}
#endif

static int
parse_weather_json_data(const char *str, WeatherConditions *wc) {
    int status;
    json_t *root;
    json_error_t error;

    root = json_loads(str, 0, &error);
    if (!root) {
        LOG_ARG_ERROR("JSON error(%d): %s", error.line, error.text);
        return FALSE;
    }

    char copy[W_TEMP_STR_SIZE];
    status = json_load_data_to_string(root,
        "current_observation.temp_f", wc->temp_f, sizeof wc->temp_f);
    CHECK_REF(status, err);
    strcpy(copy, wc->temp_f);
    snprintf(wc->temp_f, sizeof wc->temp_f, "%s%s", copy, FARENHEIT_UNIT_STR);

    status = json_load_data_to_string(root,
        "current_observation.temp_c", wc->temp_c, sizeof wc->temp_c);
    CHECK_REF(status, err);
    strcpy(copy, wc->temp_c);
    snprintf(wc->temp_c, sizeof wc->temp_c, "%s%s", copy, CELSIUS_UNIT_STR);

    status = json_load_data_to_string(root,
        "current_observation.relative_humidity",
        wc->rel_humidity, sizeof wc->rel_humidity);
    CHECK_REF(status, err);

    status = json_load_data_to_string(root,
        "current_observation.wind_mph", wc->wind_mph, sizeof wc->wind_mph);
    CHECK_REF(status, err);

    status = json_load_data_to_string(root,
        "current_observation.wind_kph", wc->wind_kph, sizeof wc->wind_kph);
    CHECK_REF(status, err);

    status = json_load_data_to_string(root,
        "current_observation.pressure_mb",
        wc->pressure_mb, sizeof wc->pressure_mb);
    CHECK_REF(status, err);

    status = json_load_data_string(root,
        "current_observation.wind_dir", wc->wind_dir, sizeof wc->wind_dir);
    CHECK_REF(status, err);

    status = json_load_data_string(root, "current_observation.weather",
        wc->weather_str, sizeof wc->weather_str);
    CHECK_REF(status, err);

#ifdef _REQUIRE_WEATHER_DEFINITIONS_
    json_t *weather;
    status = json_load_data_string_nc(root, "current_observation.icon", &weather);
    CHECK_REF(status, err);

    const char **icon;
    icon = bsearch(json_string_value(weather), weather_const_str,
        W_N_ELEMENTS(weather_const_str), sizeof *weather_const_str, w_str_cmp);

    time_t ct = time((time_t *)0);
    tzset();
    struct tm loc_time;
    localtime_r(&ct, &loc_time);

    CHECK_COND_OP(icon,
        wc->weather_id = (loc_time.tm_hour > 6 && loc_time.tm_hour < 18) ?
        weather_const_id_day[icon - weather_const_str] :
        weather_const_id_nt[icon - weather_const_str],
        wc->weather_id = WEATHER_UNKNOWN);
#endif

    json_decref(root);
    return TRUE;

    err:
        LOG_ERROR("Malformed json text. Unable to parse data");
        json_decref(root);
        return FALSE;
}

static int
parse_location_json_data(const char *str, GeoIPLocation *loc) {
    int status;
    json_t *root;
    json_error_t error;

    root = json_loads(str, 0, &error);
    if (!root) {
        LOG_ARG_ERROR("JSON error(%d): %s", error.line, error.text);
        return FALSE;
    }

    status = json_load_data_string(root, "location.country_name",
        loc->country, sizeof loc->country);
    CHECK_REF(status, err);

    status = json_load_data_string(root, "location.state",
        loc->state, sizeof loc->state);
    CHECK_REF(status, err);

    status = json_load_data_string(root, "location.city",
        loc->city, sizeof loc->city);
    CHECK_REF(status, err);

    json_decref(root);
    return TRUE;

    err:
        LOG_ERROR("Malformed json text. Unable to parse data");
        json_decref(root);
        return FALSE;
}

#define WD_ITEM(dst, format, ...) \
    snprintf(dst, sizeof(dst), format, __VA_ARGS__);

void
w_weather_get_displayable(WeatherDisplayable *wd, GeoIPLocation *loc,
                          WeatherConditions *wc) {
    if (!loc || !wc) return;

    WD_ITEM(wd->location_d, "%s, %s", loc->city,
        strlen(loc->state) ? loc->state : loc->country);
    WD_ITEM(wd->temp_f_d, "Temperature: %s", wc->temp_f);
    WD_ITEM(wd->temp_c_d, "Temperature: %s", wc->temp_c);
    WD_ITEM(wd->rel_humidity_d, "Relative Humidity: %s", wc->rel_humidity);
    WD_ITEM(wd->wind_mph_d, "Wind: %s mi/h %s", wc->wind_mph, wc->wind_dir);
    WD_ITEM(wd->wind_kph_d, "Wind: %s km/h %s", wc->wind_kph, wc->wind_dir);
    WD_ITEM(wd->pressure_mb_d, "Pressure: %s mb", wc->pressure_mb);
    strncpy(wd->weather_d, wc->weather_str, sizeof wd->weather_d);
}
#undef WD_ITEM
