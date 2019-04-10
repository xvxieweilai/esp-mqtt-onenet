#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <ArduinoJson.h>
const char* ssid = "XXWL";                     //wifi名称
const char* password = "159357000";             //wifi密码   
const char* mqtt_server = "183.230.40.39";     //onenet服务器ip
const char* dev_id = "518823862";              //设备id
const char* APIkey = "3qUBlCAM2=F64zVJV3kEGXj=lIS=";//apikey
const char* product_id = "218108";             //产品id
WiFiClient espClient;
PubSubClient client(espClient);
char msg_buf[200];
int value = 0;
int iswork = 0;
char dataTemplete[] = "{\"weight\":%d,\"iswork\":%d}";             //weight和iswork数据
char msgJson[75];
int i;
unsigned short json_len = 0;
uint8_t* packet_p;
uint8_t debug_buffer_start_index = 0;
long now;

void setup() {
	pinMode(BUILTIN_LED, OUTPUT);                                      //设置端口输出模式
	Serial.begin(9600);                                      //打开串口
	setup_wifi();                                      //设置wifi
	client.setServer(mqtt_server, 6002);                                      //连接onenet服务器
	client.connect(dev_id, product_id, APIkey);                                      //连接云平台
	client.setCallback(callback);                                      //绑定接收函数
	now = millis();
}

void loop() {

	if (!client.connected()) {
		reconnect();                                      //连接失败重新连接
	}
	client.loop();                                      //客户端循环接收
	send_data();                                      //发送数据
	if (Serial.available() > 0)                                      //串口收到数据 
	{
		String cmd = Serial.readStringUntil('\n');
		switch (cmd.charAt(0))
		{
		case 'W':
			value = cmd.substring(1).toInt();                                      //判断第一位是w后面在读取重量
			break;
		case 'E':                                      //判断是e说明电机执行完毕
			iswork = 0;
			digitalWrite(BUILTIN_LED, HIGH);
		default:
			break;
		}
	}
}

void send_data()
{
	if (millis() - now > 5000) {                                      //5秒执行一次
		now = millis();
		snprintf(msgJson, 40, dataTemplete, value,iswork);                                      //构建数据
		json_len = strlen(msgJson);
		//Serial.print(json_len);
		msg_buf[0] = char(0x03);                                      //构建json数据
		msg_buf[1] = char(json_len >> 8);
		msg_buf[2] = char(json_len & 0xff);
		memcpy(msg_buf + 3, msgJson, strlen(msgJson));
		msg_buf[3 + strlen(msgJson)] = 0;
		//Serial.println(msgJson);
		client.publish("$dp", msg_buf, 3 + strlen(msgJson), false);                                      //推送数据流到服务器
	}
}
void setup_wifi() {                                      //wifi设置函数

	delay(10);
	//Serial.println();
	//Serial.print("Connecting to ");
	//Serial.println(ssid);

	WiFi.begin(ssid, password);                                      //开启wifi

	while (WiFi.status() != WL_CONNECTED) {                                      //判断wifi是否连接成功
		delay(500);
		//Serial.print(".");
	}

	//Serial.println("");
	//Serial.println("WiFi connected");
	//Serial.println("IP address: ");
	//Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {                                      //消息接收函数
	//Serial.print("Message arrived [");
	//Serial.print(topic);
	//Serial.print("] ");
	//for (int i = 0; i < length; i++) {
		//Serial.print((char)payload[i]);
	//}
	//Serial.println();

	if ((char)payload[0] == '1') {                                      //判断接收到的字符是否为1
		iswork = 1;
		digitalWrite(BUILTIN_LED, LOW);                                      //内置led熄灭
		Serial.println("iswork");                                      //发送给arduino iswork命令
	}
}

void reconnect() {
	while (!client.connected()) {                                       //判断连接是成功

		if (client.connect(dev_id, product_id, APIkey)) {                                      //mqtt消息订阅 
			client.publish("outTopic", "hello world");                                      //发送hello world
			client.subscribe("inTopic");                                      //订阅消息
		}
		else {
			delay(5000);                                      //等5秒重试
		}
	}
}