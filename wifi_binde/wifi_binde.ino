#include <WiFi.h>
#include <ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>

const char* ssid     = "robo";
const char* password = "xxxxxxxxxxxxxxxxx";

//ROS Server
IPAddress server(192,168,2,118);
//ROS Port
const uint16_t serverPort = 11411;

ros::NodeHandle nh;
//Make a chatter publisher
std_msgs::String str_msg;
geometry_msgs::Twist twist_msg;
ros::Publisher chatter("chatter", &str_msg);
ros::Publisher chatter_twist("chatter_twist", &twist_msg);

//say hello
char hello[13] = "hello world!";

void setup()
{
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  // Set the connection to rosserial socket server
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  //Get IP durch ROS
  Serial.print("IP = ");
  Serial.println(nh.getHardware()->getLocalIP());

  //Start to be polite
  nh.advertise(chatter);
  nh.advertise(chatter_twist);
}

void loop()
{
  if (nh.connected()) {
    Serial.println("Connected");
    // Say hello
    str_msg.data = hello;
    twist_msg.linear.x = 0.01;
    twist_msg.linear.y = 0.02;
    chatter.publish( &str_msg );
    chatter_twist.publish( &twist_msg );
    } else {
      Serial.println("Not Connected");
    }
    nh.spinOnce();
    // Loop exproximativity at 1Hz
    delay(1000);
}
