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

#ifndef _MAIN_
#define _MAIN_

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define APP_TITLE "Weather Wiz"
#define APP_ID "org.methusael.weatherwiz"
#define APP_INDICATOR_ID "indicator-weather-wiz"
#define APP_INDICATOR_TITLE "Weather Wiz"

#define APP_ICON_W_CLEAR "weather-clear"
#define APP_ICON_W_CLEAR_NT "weather-clear-night"
#define APP_ICON_W_CLOUDS "weather-clouds"
#define APP_ICON_W_CLOUDS_NT "weather-clouds-night"
#define APP_ICON_W_FEW_CLOUDS "weather-few-clouds"
#define APP_ICON_W_FEW_CLOUDS_NT "weather-few-clouds-night"
#define APP_ICON_W_FOG "weather-fog"
#define APP_ICON_W_OVERCAST "weather-overcast"
#define APP_ICON_W_SEVERE "weather-severe-alert"
#define APP_ICON_W_SHOWERS "weather-showers"
#define APP_ICON_W_SHOWERS_SC "weather-showers-scattered"
#define APP_ICON_W_SNOW "weather-snow"
#define APP_ICON_W_STORM "weather-storm"
#define APP_ICON_W_UNKNOWN APP_ICON_W_SEVERE
#define APP_ICON_DEFAULT "indicator-weather"

#define APP_ICON_SET icon_set
#define _BEGIN_ARR_DECL(arr, type) \
    type arr[] = {
#define _IT_DECL(elem) elem
#define _END_ARR_DECL \
    };
#define _BEGIN_ICON_SET_DECL _BEGIN_ARR_DECL(APP_ICON_SET, const char *)

#define APP_MENU_UI_STR \
"<ui>" \
"   <popup name='IndicatorPopup'>" \
"       <menuitem action='location' />" \
"       <menuitem action='weather' />" \
"       <menuitem action='temp_f' />" \
"       <menuitem action='temp_c' />" \
"       <menuitem action='rel_humidity' />" \
"       <menuitem action='wind_mph' />" \
"       <menuitem action='wind_kph' />" \
"       <menuitem action='pressure_mb' />" \
"       <separator />" \
"       <menuitem action='pref' />" \
"       <menuitem action='about' />" \
"       <menuitem action='quit' />" \
"   </popup>" \
"</ui>"

#endif
