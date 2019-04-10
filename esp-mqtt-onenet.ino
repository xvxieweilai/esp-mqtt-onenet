#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <ArduinoJson.h>
const char* ssid = "XXWL";                     //wifi����
const char* password = "159357000";             //wifi����   
const char* mqtt_server = "183.230.40.39";     //onenet������ip
const char* dev_id = "518823862";              //�豸id
const char* APIkey = "3qUBlCAM2=F64zVJV3kEGXj=lIS=";//apikey
const char* product_id = "218108";             //��Ʒid
WiFiClient espClient;
PubSubClient client(espClient);
char msg_buf[200];
int value = 0;
int iswork = 0;
char dataTemplete[] = "{\"weight\":%d,\"iswork\":%d}";             //weight��iswork����
char msgJson[75];
int i;
unsigned short json_len = 0;
uint8_t* packet_p;
uint8_t debug_buffer_start_index = 0;
long now;

void setup() {
	pinMode(BUILTIN_LED, OUTPUT);                                      //���ö˿����ģʽ
	Serial.begin(9600);                                      //�򿪴���
	setup_wifi();                                      //����wifi
	client.setServer(mqtt_server, 6002);                                      //����onenet������
	client.connect(dev_id, product_id, APIkey);                                      //������ƽ̨
	client.setCallback(callback);                                      //�󶨽��պ���
	now = millis();
}

void loop() {

	if (!client.connected()) {
		reconnect();                                      //����ʧ����������
	}
	client.loop();                                      //�ͻ���ѭ������
	send_data();                                      //��������
	if (Serial.available() > 0)                                      //�����յ����� 
	{
		String cmd = Serial.readStringUntil('\n');
		switch (cmd.charAt(0))
		{
		case 'W':
			value = cmd.substring(1).toInt();                                      //�жϵ�һλ��w�����ڶ�ȡ����
			break;
		case 'E':                                      //�ж���e˵�����ִ�����
			iswork = 0;
			digitalWrite(BUILTIN_LED, HIGH);
		default:
			break;
		}
	}
}

void send_data()
{
	if (millis() - now > 5000) {                                      //5��ִ��һ��
		now = millis();
		snprintf(msgJson, 40, dataTemplete, value,iswork);                                      //��������
		json_len = strlen(msgJson);
		//Serial.print(json_len);
		msg_buf[0] = char(0x03);                                      //����json����
		msg_buf[1] = char(json_len >> 8);
		msg_buf[2] = char(json_len & 0xff);
		memcpy(msg_buf + 3, msgJson, strlen(msgJson));
		msg_buf[3 + strlen(msgJson)] = 0;
		//Serial.println(msgJson);
		client.publish("$dp", msg_buf, 3 + strlen(msgJson), false);                                      //������������������
	}
}
void setup_wifi() {                                      //wifi���ú���

	delay(10);
	//Serial.println();
	//Serial.print("Connecting to ");
	//Serial.println(ssid);

	WiFi.begin(ssid, password);                                      //����wifi

	while (WiFi.status() != WL_CONNECTED) {                                      //�ж�wifi�Ƿ����ӳɹ�
		delay(500);
		//Serial.print(".");
	}

	//Serial.println("");
	//Serial.println("WiFi connected");
	//Serial.println("IP address: ");
	//Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {                                      //��Ϣ���պ���
	//Serial.print("Message arrived [");
	//Serial.print(topic);
	//Serial.print("] ");
	//for (int i = 0; i < length; i++) {
		//Serial.print((char)payload[i]);
	//}
	//Serial.println();

	if ((char)payload[0] == '1') {                                      //�жϽ��յ����ַ��Ƿ�Ϊ1
		iswork = 1;
		digitalWrite(BUILTIN_LED, LOW);                                      //����ledϨ��
		Serial.println("iswork");                                      //���͸�arduino iswork����
	}
}

void reconnect() {
	while (!client.connected()) {                                       //�ж������ǳɹ�

		if (client.connect(dev_id, product_id, APIkey)) {                                      //mqtt��Ϣ���� 
			client.publish("outTopic", "hello world");                                      //����hello world
			client.subscribe("inTopic");                                      //������Ϣ
		}
		else {
			delay(5000);                                      //��5������
		}
	}
}