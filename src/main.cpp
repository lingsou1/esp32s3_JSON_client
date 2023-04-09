/*
接线说明:无

程序说明:烧录进合宙的板子,实现了客户端请求JSON数据
        程序功能:
        1. 向服务器端请求json数据信息
        2. 解析服务器端响应的json信息内容。
        3. 将解析后的数据信息显示于串口监视器
        4. 利用服务器端D3引脚（按键引脚）读数来控制客户端开发板上LED的点亮和熄灭
        5.可以请求不同页面的数据实现请求部分JSON数据,降低使用的资源
        


注意事项:


函数示例:无

作者:灵首

时间:2023_4_5

*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>


WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi

#define LED_A 10
#define LED_B 11

static int ledState;
int BOOT_int;

//定义的函数
void wifi_multi_con(void);
void wifi_multi_init(void);
void wifiClientRequest(const char* host,const int httpPort);
void parseData(WiFiClient client);


/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}


/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU1029","12345678");
  wifi_multi.addAP("haoze1029","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}


/*
# brief 通过WiFiClient库向指定网址建立连接并发出信息
# param  const char* host:需要建立连接的网站的网址
# param   const int httpPort:对应的端口号
# retval  无,但是会通过串口打印一些内容
*/
void wifiClientRequest(const char* host,const int httpPort,String url){
  WiFiClient client;

  //格式很重要 String("GET ") 这个中有一个空格,应该是不能省的,省略会导致HTTP请求发送不出去,很关键的
  String httpRequest =  String("GET ") + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n\r\n";

  //输出连接的网址
  Serial.print("connecting to :");
  Serial.print(host);
  Serial.print(url);
  Serial.print("\n");


  //连接网络服务器
  if(client.connect(host,httpPort)){
    //成功后输出success
    Serial.print("success\n");

    //向服务器发送HTTP请求
    client.print(httpRequest);    

    //串口输出HTTP请求信息
    Serial.print("sending request:");   
    Serial.print(httpRequest);
    Serial.print("\n");

    //获取并显示服务器响应状态行
    //只能用单引号
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response is :");
    Serial.print(status_response);
    Serial.print("\n");

    //跳过响应头获取响应体
    if (client.find("\r\n\r\n")){
      Serial.print("Found Header End. Start Parsing.\n");
    }

    //解析JSON数据
    parseData(client);
  }
  else{
    Serial.print("connect failed!!!\n");
  }

  //结束连接
  client.stop();
}



/*
# brief   解析http请求获取的JSON数据
# param  WiFiClient client,建立一个WiFiClien对象
# retval  无
*/
void parseData(WiFiClient client){
  //建立动态内存实现解析数据
  const size_t capacity = JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(3) + 140;
  DynamicJsonDocument doc(capacity);

  //对接受的JSON数据进行对应的反序列化,并将输出存放在doc中待使用
  deserializeJson(doc, client);
  
  //解析info
  JsonObject info = doc["info"];
  if(info){
    //解析
    Serial.println("Server Json has info: true");
    const char* info_name = info["name"]; // "lingsou"
    const char* info_url = info["url"]; // "www.bilibili.com"
    const char* info_email = info["email"]; // "haoze20212021@outlook.com"

    //将指针字符转化为字符串
    String info_name_str =  info["name"].as<String>();
    String info_url_str = info["url"].as<String>();
    String info_emial_str = info["email"].as<String>();

    //串口输出
    Serial.print("info_name_str is :");
    Serial.print(info_name_str);
    Serial.print("info_url_str is :");
    Serial.print(info_url_str);
    Serial.print("info_emial_str is :");
    Serial.print(info_emial_str);
    Serial.print("\n");
  } else {
    Serial.println("Server Json has info: false\n");
  }
  
  //解析digital_pin
  JsonObject digital_pin = doc["digital_pin"];
  if (digital_pin){
    //解析
    Serial.println("Server Json has digital_pin: true");
    const char* digitaPinValue = digital_pin["digitPin"];
    const char* BOOTValue = digital_pin["BOOT"]; 

    //将指针字符转化为整型变量
    int digitaPinValueInt =  digital_pin["digitPin"].as<int>();
    int BOOT_int = digital_pin["BOOT"].as<int>();

    //根据服务器端的BOOT按键状态控制客户端的LED灯的状态
    if(BOOT_int == 1){
      digitalWrite(LED_A,1);
      digitalWrite(LED_B,1);
    }
    else{
      digitalWrite(LED_A,0);
      digitalWrite(LED_B,0);
    }

    //串口输出
    Serial.print("digitaPinValueInt is :");
    Serial.print(digitaPinValueInt);
    Serial.print("BOOT_int is :");
    Serial.print(BOOT_int);
    Serial.print("\n");
  } else {
    Serial.println("Server Json has digital_pin: false\n");
  }
  
  //解析并通过串口输出analog_pin数据
  JsonObject analog_pin = doc["analog_pin"];
  if (analog_pin){
    //解析
    Serial.println("Server Json has analog_pin: true");
    const char* analogPinValue = digital_pin["analogPin"]; 
    const char* capPinValue = digital_pin["capPin"];

    //将指针字符转化为整型变量
    int analogPinValueInt =  analog_pin["analogPin"].as<int>();
    int capPinValueInt = analog_pin["capPin"].as<int>();

    //串口输出
    Serial.print("analogPinValueInt is :");
    Serial.print(analogPinValueInt);
    Serial.print("capPinValueInt is :");
    Serial.print(capPinValueInt);
    Serial.print("\n");
  } else {
    Serial.println("Server Json has analog_pin: false\n");
  }
}

void setup() {
  //连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //设置按键引脚,这是输出模式
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_A,0);
  digitalWrite(LED_B,0);

  //wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

}

void loop() {
  //同指定网址建立连接,同时进行相关的操作
  // wifiClientRequest("192.168.0.123",80,"/");
  // delay(2000);
  // wifiClientRequest("192.168.0.123",80,"/info");
  // delay(2000);
  wifiClientRequest("192.168.0.123",80,"/digital_pin");
  delay(2000);
  // wifiClientRequest("192.168.0.123",80,"/analog_pin");
  // delay(2000);
}