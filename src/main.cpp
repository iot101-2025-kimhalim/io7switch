#include <Arduino.h>
#include <IO7F32.h>

String user_html = "" ;

char*               ssid_pfix = (char*)"IOTSwitch";
unsigned long       lastPublishMillis = -pubInterval;
const int           PUSH_BUTTON = 43;
bool                buttonChanged = false;
bool                buttonPressed = false;
bool                buttonOldState = false;

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    data["switch"] = buttonPressed ? "on" : "off";

    serializeJson(root, msgBuffer);
    client.publish(evtTopic, msgBuffer);
}

void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];
}

void setup() {
    Serial.begin(115200);
    pinMode(PUSH_BUTTON, INPUT_PULLUP);

    initDevice();
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
    lastPublishMillis = -pubInterval;

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : ");
    Serial.println(WiFi.localIP());

    userCommand = handleUserCommand;
    set_iot_server();
    iot_connect();
}

void loop() {
    if (!client.connected()) {
        iot_connect();
    }
    client.loop();

    buttonPressed = digitalRead(PUSH_BUTTON) == LOW;
    if (buttonPressed != buttonOldState) {
        buttonChanged = true;
        buttonOldState = buttonPressed;
    }

    if (((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) 
        || (buttonChanged)) {
        publishData();
        buttonChanged = false;
        lastPublishMillis = millis();
    }
}