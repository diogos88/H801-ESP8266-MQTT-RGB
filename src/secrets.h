#pragma once

const char *ssid = "No Internet Access";
const char *password = "^-/<8++<0L/g>cB";

const char* mqtt_server = "192.168.2.24";
const int mqtt_port = 1883;
const char* mqtt_user = "pi";
const char* mqtt_password = "raspberry";
char* mqtt_command_topic = (char *)"HA/LEDController001/";
char* mqtt_status_topic = (char *)"HA/LEDController001/Status/";
char* mqtt_rgb_topic = (char *)"rgb/";
char* mqtt_w1_topic = (char *)"w1/";
char* mqtt_w2_topic = (char *)"w2/";