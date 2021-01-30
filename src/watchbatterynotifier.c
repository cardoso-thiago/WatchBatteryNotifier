#include <tizen.h>
#include <service_app.h>
#include "watchbatterynotifier.h"
#include <device/battery.h>
#include <device/callback.h>
#include <notification.h>

bool almost_low = false;
bool low = false;
bool really_low = false;

void create_notification(char* text) {
	notification_h notification = NULL;
	notification = notification_create(NOTIFICATION_TYPE_NOTI);
	if(notification != NULL) {
		notification_set_text(notification, NOTIFICATION_TEXT_TYPE_TITLE, "Battery Alert", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		notification_set_text(notification, NOTIFICATION_TEXT_TYPE_CONTENT, text, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		notification_set_sound(notification, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL);
		int ret = 0;
		ret = notification_post(notification);
		dlog_print(DLOG_INFO, LOG_TAG, "Send notification result: %d", ret);
	}
}

void battery_changed(device_callback_e type, void *value, void *user_data) {
	int battery_percent = 0;
	if (device_battery_get_percent(&battery_percent) == DEVICE_ERROR_NONE)
	{
		dlog_print(DLOG_INFO, LOG_TAG, "Battery percentage: %d", battery_percent);

		if(battery_percent <= 20 && !almost_low) {
			almost_low = true;
			dlog_print(DLOG_INFO, LOG_TAG, "Almost Low Battery Event");
			create_notification("Battery getting low, pay attention.");
		} else if(battery_percent <= 15 && !low) {
			low = true;
			dlog_print(DLOG_INFO, LOG_TAG, "Low Battery Event");
			create_notification("Low Battery!");
		} else if(battery_percent <= 10 && !really_low) {
			really_low = true;
			dlog_print(DLOG_INFO, LOG_TAG, "Really Low Battery Event");
			create_notification("Really Low Battery! Please, charge me.");
		} else if(battery_percent > 20){
			almost_low = false;
			low = false;
			really_low = false;
		}
	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "Battery info device error");
	}
}

bool service_app_create(void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Starting battery notifier service...");
	int result;
	result = device_add_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, battery_changed, NULL);
	dlog_print(DLOG_INFO, LOG_TAG, "Result of add callback: %d", result);
	return true;
}

void service_app_terminate(void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Ending battery notifier service...");
	int result;
	result = device_remove_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, battery_changed);
	dlog_print(DLOG_INFO, LOG_TAG, "Result of remove callback: %d", result);
	return;
}

void service_app_control(app_control_h app_control, void *data)
{
	return;
}

static void
service_app_lang_changed(app_event_info_h event_info, void *user_data){return;}

static void
service_app_region_changed(app_event_info_h event_info, void *user_data){}

static void
service_app_low_battery(app_event_info_h event_info, void *user_data){}

static void
service_app_low_memory(app_event_info_h event_info, void *user_data){}

int main(int argc, char* argv[])
{
	char ad[50] = {0,};
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	return service_app_main(argc, argv, &event_callback, ad);
}
