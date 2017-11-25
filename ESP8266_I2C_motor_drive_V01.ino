/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */
// Cerevo.Inc modefy 2015/9/6

// Espressif ESP8266を使ってミニ四駆をラジコン化する
// FREE WING modify 2017/11/25
// http://www.neko.ne.jp/~freewing/hardware/espressif_esp8266_mini_yonku_radio_control/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "ESP-Cerevo";
const char *password = "";

//const char *ssid = "ESP-Cerevo2";
//const char *password = "techblog2";

ESP8266WebServer server(80);

/* set I2C library*/
#include <Wire.h>

//#define ADDR	0x64
// FREE WING modify 2017/11/25
// A0='L', A1='L' = 0xC0h
#define ADDR	0x60

// FREE WING add 2017/11/25
#define MOTOR_IN_STANDBY	0b00
#define MOTOR_IN_REVERSE	0b01
#define MOTOR_IN_NORMAL		0b10
#define MOTOR_IN_BRAKE		0b11

#define command_start	0
#define command_stop	1
#define command_back	2

char state = command_stop;

String form ="<html><head><meta name=viewport content=width=100></head>"
"<form action=start><input type= submit	value=start	target=tif></form>"
"<form action=back><input type=submit value=back target=tif></form>"
"<form action=stop><input type=submit value=stop target=tif></form>"
"<iframe src=javascript:false style=display:none name=tif id=tif></iframe>"
"</html>";

/* Just a little test message.	Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.print("Configuring access point...");

	// FREE WING modify 2017/11/25
//	Wire.begin(4, 14);
	Wire.begin(); // 4, 5 // (SDA, SCL)
	delay(40);

	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);

	// And as regular external functions:
	server.on("/stop", handle_stop);
	server.on("/start", handle_start);
	server.on("/back", handle_back);

	server.begin();
	Serial.println("HTTP server started");
}


void loop() {
	server.handleClient();
}


void handleRoot() {
	server.send(200, "text/html", form);
}


void handle_stop() {
	Serial.print("stop");

	if((state == command_start)||(state == command_back)) {
		stop_motor();

		state = command_stop;
	}

	server.send(200, "text/html", form);
}


void handle_start() {
	Serial.print("start");

	if(state == command_back){

		stop_motor();
		delay(10);

		start_motor();

	}else if(state == command_stop){
		start_motor();
	}

	state = command_start;
	server.send(200, "text/html", form);
}


void handle_back() {
	Serial.print("back");

	if(state == command_start){

		stop_motor();
		delay(10);

		reverse_motor();

	}else if(state == command_stop){
		reverse_motor();
	}

	state = command_back;
	server.send(200, "text/html", form);
}


// FREE WING add 2017/11/25
void write_motor(int motor_in, int vset) {

	Wire.beginTransmission(ADDR);
	Wire.write(0x00);
	Wire.write((vset << 2) + motor_in);
	Wire.endTransmission();
}

void start_motor() {

	int motor_in = MOTOR_IN_NORMAL;
	int vset = 0x06;
	write_motor(motor_in, vset);
	delay(10);

	vset = 0x0d;
	write_motor(motor_in, vset);
	delay(10);

	vset = 0x10;
	write_motor(motor_in, vset);
}


void reverse_motor() {

	int motor_in = MOTOR_IN_REVERSE;
	int vset = 0x06;
	write_motor(motor_in, vset);
	delay(10);

	vset = 0x0d;
	write_motor(motor_in, vset);
	delay(10);

	vset = 0x10;
	write_motor(motor_in, vset);
}


void stop_motor() {

	int motor_in = MOTOR_IN_STANDBY;
	int vset = 0x06;
	write_motor(motor_in, vset);
	delay(10);

	motor_in = MOTOR_IN_BRAKE;
	write_motor(motor_in, vset);
}
