#pragma once

const char *ssid = "Network";
const char *password = "p4ssw0rd";

const char* mqtt_server = "127.0.0.1";
const int mqtt_port = 1883;
const char* mqtt_user = "user";
const char* mqtt_password = "p4ssw0rd";
char* mqtt_command_topic = (char *)"inTopic/";
char* mqtt_status_topic = (char *)"outTopic/";
char* mqtt_rgb_topic = (char *)"rgb/";
char* mqtt_w1_topic = (char *)"w1/";
char* mqtt_w2_topic = (char *)"w2/";