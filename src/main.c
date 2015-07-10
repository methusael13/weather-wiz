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

/* TODO: Port to GTK-3.0 */
#include <assert.h>
#include <string.h>
#include "main.h"
#include "event_log.h"
#include "weather_backend.h"

_BEGIN_ICON_SET_DECL
    APP_ICON_W_CLEAR, APP_ICON_W_CLEAR_NT,
    APP_ICON_W_CLOUDS, APP_ICON_W_CLOUDS_NT,
    APP_ICON_W_FEW_CLOUDS, APP_ICON_W_FEW_CLOUDS_NT,
    APP_ICON_W_FOG, APP_ICON_W_OVERCAST,
    APP_ICON_W_SEVERE, APP_ICON_W_SHOWERS,
    APP_ICON_W_SHOWERS_SC, APP_ICON_W_SNOW,
    APP_ICON_W_STORM, APP_ICON_W_UNKNOWN
_END_ARR_DECL

static GtkWidget *window;
static GtkUIManager *global_ui_man;
static AppIndicator *global_app_ind;
static GtkActionGroup *weather_info_group;

/* Initiate with default settings */
static WeatherDisplayable wd = {
    .use_temp_f = FALSE,
    .use_wind_mph = FALSE,
    .display_label = TRUE
};

static void
app_exit(GtkAction *action, gpointer user_data);

#define SET_ACT_LABEL(grp, name, lab) \
    gtk_action_set_label(gtk_action_group_get_action(grp, name), lab)
#define SET_ACT_VISIBLE(grp, name, vis) \
    gtk_action_set_visible(gtk_action_group_get_action(grp, name), vis)
#define SET_ACT_VISIBLE_COMBO(grp, name1, vis1, name2, vis2) \
    SET_ACT_VISIBLE(grp, name1, vis1); \
    SET_ACT_VISIBLE(grp, name2, vis2);

_BEGIN_ARR_DECL(actions, const GtkActionEntry)
    {"pref", "app-pref", "Preferences", NULL, NULL, NULL},
    {"about", "app-about", "About", NULL, NULL, NULL},
    {"quit", "app-quit", "Quit", NULL, NULL, G_CALLBACK(app_exit)}
_END_ARR_DECL

_BEGIN_ARR_DECL(weather_info_actions, const GtkActionEntry)
    {"location", "w-loc", "Loaction"},
    {"weather", "w-weather", "Weather"},
    {"temp_f", "w-temp-f", "Temperature"},
    {"temp_c", "w-temp-c", "Temperature"},
    {"rel_humidity", "w-rel-hum", "Relative Humidity"},
    {"wind_mph", "w-wind-mph", "Wind"},
    {"wind_kph", "w-wind-kph", "Wind"},
    {"pressure_mb", "w-pre-mb", "Pressure"}
_END_ARR_DECL

static void
app_update_weather_display(GtkActionGroup *group,
                           WeatherConditions *wc,
                           WeatherDisplayable *wd) {
    SET_ACT_LABEL(group, "location", wd->location_d);
    SET_ACT_LABEL(group, "weather", wd->weather_d);
    SET_ACT_LABEL(group, "temp_f", wd->temp_f_d);
    SET_ACT_LABEL(group, "temp_c", wd->temp_c_d);
    SET_ACT_LABEL(group, "rel_humidity", wd->rel_humidity_d);
    SET_ACT_LABEL(group, "wind_mph", wd->wind_mph_d);
    SET_ACT_LABEL(group, "wind_kph", wd->wind_kph_d);
    SET_ACT_LABEL(group, "pressure_mb", wd->pressure_mb_d);

    SET_ACT_VISIBLE_COMBO(group, "temp_f", wd->use_temp_f,
                         "temp_c", !wd->use_temp_f);
    SET_ACT_VISIBLE_COMBO(group, "wind_mph", wd->use_wind_mph,
                         "wind_kph", !wd->use_wind_mph);

    if (wd->display_label)
        app_indicator_set_label(global_app_ind, wd->use_temp_f ?
                                wc->temp_f : wc->temp_c, NULL);
    else
        app_indicator_set_label(global_app_ind, NULL, NULL);
}

static void
on_weather_update(GeoIPLocation *loc, WeatherConditions *wc) {
    if (!global_app_ind) return;

    w_weather_get_displayable(&wd, loc, wc);
    app_update_weather_display(weather_info_group, wc, &wd);
    app_indicator_set_icon_full(global_app_ind,
                                APP_ICON_SET[wc->weather_id],
                                wd.weather_d);

    if (!gtk_action_group_get_visible(weather_info_group))
        gtk_action_group_set_visible(weather_info_group, TRUE);
}

static void
app_exit(GtkAction *action, gpointer user_data) {
    if (W_IS_SERVICE_STARTED(w_get_service_status())) {
        w_stop_weather_service();
        assert(!W_IS_SERVICE_STARTED(w_get_service_status()));
    }

    LOG_INFO("Exiting Weather-Wiz...");
    gtk_main_quit();
}

static void
app_activate(void) {
    if (W_IS_SERVICE_STARTED(w_get_service_status()))
        return;

    LOG_INFO("Starting Weather-Wiz...");
    w_set_weather_update_listener(on_weather_update);
    w_start_weather_service();
    assert(W_IS_SERVICE_STARTED(w_get_service_status()));
}

static void
app_initiate_indicator(GtkMenu *menu, AppIndicator **app_ind) {
    if (*app_ind) return;

    *app_ind = app_indicator_new(APP_INDICATOR_ID, APP_ICON_DEFAULT,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_menu(*app_ind, GTK_MENU(menu));
    app_indicator_set_attention_icon_full(*app_ind, APP_ICON_W_SEVERE,
                                          "Sit tight! Weather's bad");
    app_indicator_set_title(*app_ind, APP_INDICATOR_TITLE);
    app_indicator_set_status(*app_ind, APP_INDICATOR_STATUS_ACTIVE);
}

static int
app_activate_indicator(void) {
    LOG_INFO("Initiating indicator...");

    /* Setup actions, ui-manager and menus */
    GtkActionGroup *ac_grp;
    weather_info_group = gtk_action_group_new("WeatherInfo");
    gtk_action_group_set_visible(weather_info_group, FALSE);

    ac_grp = gtk_action_group_new("IndicatorActions");
    gtk_action_group_add_actions(weather_info_group, weather_info_actions,
                                 G_N_ELEMENTS(weather_info_actions), window);
    gtk_action_group_add_actions(ac_grp, actions, G_N_ELEMENTS(actions),
                                 window);

    global_ui_man = gtk_ui_manager_new();
    g_object_set_data_full(G_OBJECT(window),
                          "ui_manager", global_ui_man,
                           g_object_unref);
    gtk_ui_manager_insert_action_group(global_ui_man, weather_info_group, 0);
    gtk_ui_manager_insert_action_group(global_ui_man, ac_grp,
                                       G_N_ELEMENTS(weather_info_actions));

    GError *error = NULL;
    if (!gtk_ui_manager_add_ui_from_string(global_ui_man,
                                           APP_MENU_UI_STR,
                                          -1, &error)) {
        LOG_ARG_ERROR("Unable to build menus: %s", error->message);
        g_error_free(error);
        error = NULL;
        goto err;
    }

    GtkMenu *menu;
    menu = GTK_MENU(gtk_ui_manager_get_widget(global_ui_man, "/ui/IndicatorPopup"));
    app_initiate_indicator(menu, &global_app_ind);

    g_object_unref(ac_grp);
    g_object_unref(menu);

    LOG_INFO("Indicator initiated!");
    return TRUE;
    err:
        return FALSE;
}

int
main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    /* Dummy window */
    window =  gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    if (!app_activate_indicator()) {
        gtk_widget_destroy(window);
        return 1;
    }

    app_activate();
    gtk_main();

    return 0;
}
