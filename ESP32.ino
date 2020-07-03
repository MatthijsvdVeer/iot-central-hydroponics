// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "WiFi.h"
#include "src/iotc/common/string_buffer.h"
#include "src/iotc/iotc.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define WIFI_SSID "<WIFI SSID>"
#define WIFI_PASSWORD "<WIFI PASSWORD>"
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char *SCOPE_ID = "<SCOPE ID>";
const char *DEVICE_ID = "<DEVICE ID>";
const char *DEVICE_KEY = "<DEVICE KEY>";

void on_event(IOTContext ctx, IOTCallbackInfo *callbackInfo);
void on_settings_updated(IOTContext ctx, IOTCallbackInfo *callbackInfo);
#include "src/connection.h"

void on_event(IOTContext ctx, IOTCallbackInfo *callbackInfo)
{
    // ConnectionStatus
    if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0)
    {
        LOG_VERBOSE("Is connected ? %s (%d)",
                    callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO",
                    callbackInfo->statusCode);
        isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
        return;
    }

    // payload buffer doesn't have a null ending.
    // add null ending in another buffer before print
    AzureIOT::StringBuffer buffer;
    if (callbackInfo->payloadLength > 0)
    {
        buffer.initialize(callbackInfo->payload, callbackInfo->payloadLength);
    }

    LOG_VERBOSE("- [%s] event was received. Payload => %s\n",
                callbackInfo->eventName, buffer.getLength() ? *buffer : "EMPTY");

    if (strcmp(callbackInfo->eventName, "Command") == 0)
    {
        LOG_VERBOSE("- Command name was => %s\r\n", callbackInfo->tag);
    }

    if (strcmp(callbackInfo->eventName, "SettingsUpdated") == 0)
    {
        LOG_VERBOSE("- Setting name was => %s\r\n", callbackInfo->tag);
    }
}

void setup()
{
    Serial.begin(115200);

    connect_wifi(WIFI_SSID, WIFI_PASSWORD);
    connect_client(SCOPE_ID, DEVICE_ID, DEVICE_KEY);

    sensors.begin();
    dht.begin();

    if (context != NULL)
    {
        lastTick = 0; // set timer in the past to enable first telemetry a.s.a.p
    }
}

void loop()
{
    if (isConnected)
    {
        unsigned long ms = millis();
        if (ms - lastTick > 600000)
        {
            sensors.requestTemperatures();
            float waterTemp = sensors.getTempCByIndex(0);
            float humidity = dht.readHumidity();
            float temperature = dht.readTemperature();
            
            char msg[128] = {0};
            int pos = 0, errorCode = 0;

            lastTick = ms;
            pos = snprintf(msg, sizeof(msg) - 1, "{\"waterTemp\": %f, \"ambientTemp\": %f, \"ambientHum\": %f}", waterTemp, temperature, humidity);
            errorCode = iotc_send_telemetry(context, msg, pos);
            msg[pos] = 0;

            if (errorCode != 0)
            {
                LOG_ERROR("Sending message has failed with error code %d", errorCode);
            }
        }

        iotc_do_work(context); // do background work for iotc
    }
    else
    {
        iotc_free_context(context);
        context = NULL;
        connect_client(SCOPE_ID, DEVICE_ID, DEVICE_KEY);
    }
}
