/*
  WiFi AVRISP
  Copyright (c) 2016 Subhajit Das

  Licence Disclaimer:
    This file is part of WiFi AVRISP.

    WiFi AVRISP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WiFi AVRISP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ESP8266WebServer.h>  // reuired for Server opertaions
#include <LedBlinker.h>




#define MAXSTRENGTH 5
#define MIN_dBm -100
#define MAX_dBm -50

#define VERBOSE 0

LedBlinker ledBlinker(LED_BUILTIN);
ESP8266WebServer ConfigServer(80);
String networksStr;


//extern Storage DB;
String URLDecode(String param) {
  Serial.printf("URLDecode %s\n", param.c_str());
  param.replace("+", " ");
  param.replace("%21", "!");
  param.replace("%23", "#");
  param.replace("%24", "$");
  param.replace("%26", "&");
  param.replace("%27", "'");
  param.replace("%28", "(");
  param.replace("%29", ")");
  param.replace("%2A", "*");
  param.replace("%2B", "+");
  param.replace("%2C", ",");
  param.replace("%2F", "/");
  param.replace("%3A", ":");
  param.replace("%3B", ";");
  param.replace("%3D", "=");
  param.replace("%3F", "?");
  param.replace("%40", "@");
  param.replace("%5B", "[");
  param.replace("%5D", "]");
  return param;
}

int strIndex = 0;

String splitStr()
{
  if (strIndex == -1)
    return "";
  byte i = networksStr.indexOf('^', strIndex) + 1;
  strIndex = networksStr.indexOf('^', i);
  if (strIndex == -1)
    return "";
  else
    return networksStr.substring(i, strIndex);
}

String getWiFiSigStr(int p, String buff) {
  int sginal = map(WiFi.RSSI(p), MIN_dBm, MAX_dBm, 0, MAXSTRENGTH);
  buff = "[";
  for (byte j = 1; j <= MAXSTRENGTH; j ++)
  {
    //Serial.printf("%s", (j <= sginal) ? "I" : ".");
    buff += (j <= sginal) ? "I" : ".";
  }
  //     Serial.print("] ");
  buff += "] ";
  return buff;
}

void initConfigServer() {
  // host is dynamically generated
  // local IP address is send as host
  ConfigServer.on("/host.js", []() {
    String content = String("var host = \"") + WiFi.softAPIP().toString() + "\";\r\n";
    ConfigServer.send(200, "text/javascript", content);
  });



  ConfigServer.on("/data.js", []() {
    //String content = String("var ip = '") + (/*WiFi.localIP().length() < 3*/ 0 ?  WiFi.softAPIP().toString() : WiFi.localIP()) + "';\r\n";
    String content = String("var ip = '") + WiFi.softAPIP().toString() + "';\r\n";
    content += String("var gateway = '") + "Gateway" + "';\r\n";
    content += String("var host = '") + WiFi.softAPIP().toString() + "';\r\n";
    content += String("var subnet = '") + "Wifi subnet" + "';\r\n";
    content += String("var wifimode = '") + "AP" + "';\r\n";
    content += String("var ssid = '") + WiFi.SSID() + "';\r\n";
    content += String("var enc = '") + "WiFi.getProtection()" + "';\r\n";
    content += String("var configmode = '") + "WiFi.getConfigMode()" + "';\r\n";
    ConfigServer.send(200, "text/javascript", content);
    Serial.printf("/data.js SSID %s host %s\n", WiFi.SSID().c_str(), WiFi.softAPIP().toString().c_str() );
  });



  // a javascript array of options is send
  ConfigServer.on("/wifilist.js", []() {
    String Options = "var Options = [";
    String buff;
    byte n = WiFi.scanNetworks();

    Serial.println("/wifilist.js \n number of networks found " );
    Serial.println(n);
    if (n > 0)
    {
      for (byte i = 0; i < n; ++i) {
        Options += "{\"enc\":\"" + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "f" : "t") + "\",";
        Options += "\"sig\":\"" + getWiFiSigStr(i, buff) + "\",";
        Options += "\"ssid\":\"" + WiFi.SSID(i) + "\"}";

        Serial.println(WiFi.SSID(i));

        if (i < (n - 1))
          Options += ",\r\n";
        else
          Options += "\r\n";

      }
    }
    Options += "];";
    ConfigServer.send(200, "text/javascript", Options);
    Serial.println("/wifilist.js");
    Serial.println(Options);
  });

  ConfigServer.on("/", []() {
    String html = "<!DOCTYPE html> <html> <head> <title>ESP WiFi Configuration Server</title> <meta charset= \"UTF-8\" > <meta name= \"viewport\" content= \"width=device-width, initial-scale=1.0\" > <style type= \"text/css\" > form{ text-align: center; margin: auto; align-content: center; } table{ margin: auto; align-content: center; text-align: center; border-style: solid; border-width: 1px; background-color: #00bcd4; color: snow; } #header{ font-size: 200%; font-weight: bold; } #btn{ background-color: snow; border-style: solid; border-width: 1px; } table#t01 tr:nth-child(odd) { background-color:#fff; } </style> <!-- dynamically generated script --> <script type=\"text/javascript\" src=\"data.js\"></script> <script> function init() { document.getElementById(\"host\").innerHTML = host; document.getElementById(\"wifimode\").innerHTML = wifimode; document.getElementById(\"enc\").innerHTML = (enc === \"0\" ? \"Open\" : \"Protected\"); document.getElementById(\"ssid\").innerHTML = ssid; document.getElementById(\"configmode\").innerHTML = (configmode === \"0\" ? \"Auto\" : \"Manual\"); document.getElementById(\"ip2\").innerHTML = ip; if (configmode === \"1\") { document.getElementById(\"ip\").innerHTML = ip; document.getElementById(\"gateway\").innerHTML = gateway; document.getElementById(\"subnet\").innerHTML = subnet; } else if (configmode === \"0\"){ document.getElementById(\"ipop1\").style.display = \"none\"; document.getElementById(\"ipop2\").style.display = \"none\"; document.getElementById(\"ipop3\").style.display = \"none\"; } } </script> </head> <body onload=\"init()\"> <div> <table> <tr> <td colspan= \"2\" ><span id= \"header\" >WiFi Configuration</span></td> </tr> <tr> <td>Hostname :</td> <td id=\"host\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> <td>Mode:</td> <td id=\"wifimode\"></td> </tr> <tr> <td>Type:</td> <td id=\"enc\"></td> </tr> <tr> <td>SSID :</td> <td id=\"ssid\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <!-- IP configuration start --> <tr> <td>IP config:</td> <td id=\"configmode\"></td> </tr> <!-- manual config start --> <tr id=\"ipop1\"> <td>IP address</td> <td id=\"ip\"></td> </tr> <tr id=\"ipop2\"> <td>Gateway</td> <td id=\"gateway\"></td> </tr> <tr id=\"ipop3\"> <td>Subnet</td> <td id=\"subnet\"></td> </tr> <!-- manual config end --> <!-- IP configuration end --> <tr> <td colspan=\"2\"><br><a href=\"config.html\">Change Config</a></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> <td colspan=\"2\">Tip: Please connect to <span id=\"ip2\"></span>:384<br> to upload program to AVR (AVR ISP) </td> </tr> <tr> <td colspan=\"2\">Created by Subhajit Das 25/06/2016 </td> </tr> </table> </div> </body> </html>";


    //server.send(200, "text/html", "<h1>You are connected</h1>");
    ConfigServer.send(200, "text/html", html);
    Serial.println("on /");
  });

  // configuration page
  ConfigServer.on("/config.html", []() {
    /*File page = SPIFFS.open("/config.html", "r");
      ConfigServer.streamFile(page, "text/html");
      page.close();*/
    //String html ="<!DOCTYPE html> <html> <head> <title>ESP WiFi Configuration Server</title> <meta charset= \"UTF-8\" > <meta name= \"viewport\" content= \"width=device-width, initial-scale=1.0\" > <style type= \"text/css\" > form{ text-align: center; margin: auto; align-content: center; } table{ margin: auto; align-content: center; text-align: center; border-style: solid; border-width: 1px; background-color: #00bcd4; color: snow; } #header{ font-size: 200%; font-weight: bold; } #btn{ background-color: snow; border-style: solid; border-width: 1px; } table td:nth-child(odd) { text-align: right; } table td:nth-child(even) { text-align: left; } </style> <!-- dynamically generated script --> <script type=\"text/javascript\" src=\"host.js\"></script> <script type=\"text/javascript\" src=\"wifilist.js\"></script> <script type= \"text/javascript\" > function init() { try { // set host document.getElementById(\"hstl1\").value = host; document.getElementById(\"hstl2\").innerHTML = host; } catch (e) { } try { // add options to wifilist var wifiList = document.getElementById(\"wifilist\"); for (var i = 0; i < Options.length; i++) { var optnode = document.createElement(\"option\"); optnode.value = Options[i].ssid; wifiList.appendChild(optnode); } } catch (e) { } disableManualIP(); disablePass(); checkSecurity(); } function checkSecurity() { try { var ssid = document.getElementById(\"ssid\").value; for (var i = 0; i < Options.length; i++) { if (ssid == Options[i].ssid) { if (Options[i].enc == \"f\") { document.getElementById(\"ep\").checked = false; document.getElementById(\"dp\").checked = true; disablePass(); } else if (Options[i].enc == \"t\") { document.getElementById(\"dp\").checked = false; document.getElementById(\"ep\").checked = true; enablePass(); } } } } catch (e) { } } // functions to enable/disable options function disableManualIP() { document.getElementById(\"ipop1\").style.display = \"none\"; document.getElementById(\"ipop2\").style.display = \"none\"; document.getElementById(\"ipop3\").style.display = \"none\"; } function enableManualIP() { document.getElementById(\"ipop1\").style.display = \"table-row\"; document.getElementById(\"ipop2\").style.display = \"table-row\"; document.getElementById(\"ipop3\").style.display = \"table-row\"; } function disablePass() { document.getElementById(\"passfield\").style.display = \"none\"; } function enablePass() { document.getElementById(\"passfield\").style.display = \"table-row\"; } </script> </head> <body onload=\"init()\"> <div> <form action= \"updateconfig\" method= \"post\" > <table> <tr> <td colspan= \"2\" ><span id= \"header\" >WiFi Configuration</span></td> </tr> <tr> <td>Hostname :</td> <td><input type=\"text\" name=\"host\" id=\"hstl1\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> <td>Mode:</td> <td> <input type=\"radio\" name=\"wifimode\" value=\"ap\">Direct <input type=\"radio\" name=\"wifimode\" value=\"sta\" checked=\"checked\">Connect </td> </tr> <tr> <td>Type:</td> <td> <input type=\"radio\" name=\"encryption\" value=\"true\" onclick=\"enablePass()\" id=\"ep\">Protected <input type=\"radio\" name=\"encryption\" value=\"false\" checked=\"checked\" onclick=\"disablePass()\" id=\"dp\">Open </td> </tr> <tr> <td>SSID :</td> <td><input name=\"ssid\" id=\"ssid\" list= \"wifilist\" onkeyup=\"checkSecurity()\" onchange=\"checkSecurity()\"> <datalist id= \"wifilist\" > </datalist> </td> </tr> <tr id=\"passfield\"> <td>Password :</td> <td><input type=\"password\" name=\"pass\" id=\"pass\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <!-- IP configuration start --> <tr> <td>IP config:</td> <td> <input type=\"radio\" name=\"configmode\" value=\"auto\" checked=\"checked\" onclick=\"disableManualIP()\">Auto <input type=\"radio\" name=\"configmode\" value=\"manual\" onclick=\"enableManualIP()\">Manual </td> </tr> <!-- manual config start --> <tr id=\"ipop1\"> <td>IP address</td> <td><input type=\"text\" name=\"ip\"></td> </tr> <tr id=\"ipop2\"> <td>Gateway</td> <td><input type=\"text\" name=\"gateway\"></td> </tr> <tr id=\"ipop3\"> <td>Subnet</td> <td><input type=\"text\" name=\"subnet\"></td> </tr> <!-- manual config end --> <!-- IP configuration end --> <tr> <td colspan=\"2\" style=\"text-align: center\"><br><input id=\"btn\" type=\"submit\" value=\"Connect\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> <td colspan=\"2\" style=\"text-align: center\">Tip: Please connect to <span id=\"hstl2\"></span>:384<br> to upload program to AVR (AVR ISP) </td> </tr> </table> </form> </div> </body> </html>";
    //String html = "<!DOCTYPE html> <html> <head> <title>ESP WiFi Configuration Server</title> <meta charset= \"UTF-8\" > <meta name= \"viewport\" content= \"width=device-width, initial-scale=1.0\" > <style type= \"text/css\" > form{ text-align: center; margin: auto; align-content: center; } table{ margin: auto; align-content: center; text-align: center; border-style: solid; border-width: 1px; background-color: #00bcd4; color: snow; } #header{ font-size: 200%; font-weight: bold; } #btn{ background-color: snow; border-style: solid; border-width: 1px; } table td:nth-child(odd) { text-align: right; } table td:nth-child(even) { text-align: left; } </style> <!-- dynamically generated script --> <script type=\"text/javascript\" src=\"host.js\"></script> <script type=\"text/javascript\" src=\"wifilist.js\"></script> <script type= \"text/javascript\" > function init() { try { // set host document.getElementById(\"hstl1\").value = host; document.getElementById(\"hstl2\").innerHTML = host; } catch (e) { } try { // add options to wifilist var wifiList = document.getElementById(\"wifilist\"); for (var i = 0; i < Options.length; i++) { var optnode = document.createElement(\"option\"); optnode.value = Options[i].ssid; wifiList.appendChild(optnode); } } catch (e) { } disableManualIP(); disablePass(); checkSecurity(); } function checkSecurity() { try { var ssid = document.getElementById(\"ssid\").value; for (var i = 0; i < Options.length; i++) { if (ssid == Options[i].ssid) { if (Options[i].enc == \"f\") { document.getElementById(\"ep\").checked = false; document.getElementById(\"dp\").checked = true; disablePass(); } else if (Options[i].enc == \"t\") { document.getElementById(\"dp\").checked = false; document.getElementById(\"ep\").checked = true; enablePass(); } } } } catch (e) { } } // functions to enable/disable options function disableManualIP() { document.getElementById(\"ipop1\").style.display = \"none\"; document.getElementById(\"ipop2\").style.display = \"none\"; document.getElementById(\"ipop3\").style.display = \"none\"; } function enableManualIP() { document.getElementById(\"ipop1\").style.display = \"table-row\"; document.getElementById(\"ipop2\").style.display = \"table-row\"; document.getElementById(\"ipop3\").style.display = \"table-row\"; } function disablePass() { document.getElementById(\"passfield\").style.display = \"none\"; } function enablePass() { document.getElementById(\"passfield\").style.display = \"table-row\"; } </script> </head> <body onload=\"init()\"> <div> <form action= \"updateconfig\" method= \"post\" > <table> <tr> <td colspan= \"2\" ><span id= \"header\" >WiFi Configuration</span></td> </tr> <tr> <td>Hostname :</td> <td><input type=\"text\" name=\"host\" id=\"hstl1\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> <td>Type:</td> <td> <input type=\"radio\" name=\"encryption\" value=\"true\" onclick=\"enablePass()\" id=\"ep\">Protected <input type=\"radio\" name=\"encryption\" value=\"false\" checked=\"checked\" onclick=\"disablePass()\" id=\"dp\">Open </td> </tr> <tr> <td>SSID :</td> <td><input name=\"ssid\" id=\"ssid\" list= \"wifilist\" onkeyup=\"checkSecurity()\" onchange=\"checkSecurity()\"> <datalist id= \"wifiList\" > <option value=\"115x175 mm\">115x175 mm</option> <option value=\"115x176 mm\">115x176 mm</option></datalist> </td> </tr> <tr id=\"passfield\"> <td>Password :</td> <td><input type=\"password\" name=\"pass\" id=\"pass\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <!-- IP configuration start --> <!-- manual config start --> <!-- manual config end --> <!-- IP configuration end --> <tr> <td colspan=\"2\" style=\"text-align: center\"><br><input id=\"btn\" type=\"submit\" value=\"Update Setup\"></td> </tr> <!-- blank space --> </table> </form> </div> </body> </html>";

    String html = "<!DOCTYPE html> <html> <head> <title>ESP WiFi Configuration Server</title> <meta charset= \"UTF-8\" > <meta name= \"viewport\" content= \"width=device-width, initial-scale=1.0\" > <style type= \"text/css\" > form{ text-align: center; margin: auto; align-content: center; } table{ margin: auto; align-content: center; text-align: center; border-style: solid; border-width: 1px; background-color: #00bcd4; color: snow; } #header{ font-size: 200%; font-weight: bold; } #btn{ background-color: snow; border-style: solid; border-width: 1px; } table td:nth-child(odd) { text-align: right; } table td:nth-child(even) { text-align: left; } </style> <!-- dynamically generated script --> <script type=\"text/javascript\" src=\"host.js\"></script> <script type=\"text/javascript\" src=\"wifilist.js\"></script> <script type= \"text/javascript\" > \
    function init() \
    { try { document.getElementById(\"hstl1\").value = host; document.getElementById(\"hstl2\").innerHTML = host; \
    } catch (e) { } \
    try {var wifilist = document.getElementById(\"wifilist\"); for (var i = 0; i < Options.length; i++) { var optnode = document.createElement(\"option\"); optnode.value = Options[i].ssid; optnode.innerHTML = Options[i].sig; wifilist.appendChild(optnode); } } catch (e) { } \
     enablePass();  } \
    function checkSecurity() { console.log (\"checkSecurity\"); try { var ssidVars = document.getElementsByName(\"ssidInput\");  var ssidVar = ssidVars[0];  \
    for (var i = 0; i < Options.length; i++) { if (ssidVar.value == Options[i].ssid) { console.log (\" FOUND!!!\"); if (Options[i].enc == \"f\") { document.getElementById(\"ep\").checked = false; document.getElementById(\"dp\").checked = true; disablePass(); } else if (Options[i].enc == \"t\") { document.getElementById(\"dp\").checked = false; document.getElementById(\"ep\").checked = true; enablePass(); } } } } catch (e) { } } \
    function disablePass() { console.log (\"disablePass\"); document.getElementById(\"passfield\").style.display = \"none\"; } function enablePass() { document.getElementById(\"passfield\").style.display = \"table-row\"; } </script> </head> <body onload=\"init()\"> <div> <form action= \"updateconfig\" method= \"post\" > <table> <tr> <td colspan= \"2\" ><span id= \"header\" >WiFi Configuration</span></td> </tr> <tr> <td>Hostname :</td> <td><input type=\"text\" name=\"host\" id=\"hstl1\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <tr> \
    <td>Type:</td> <td> <input type=\"radio\" name=\"encryption\" value=\"true\" onclick=\"enablePass()\" id=\"ep\">Protected <input type=\"radio\" name=\"encryption\"value=\"false\" checked=\"checked\" onclick=\"disablePass()\" id=\"dp\">Open \
        </td> </tr> <tr> <td>SSID :</td> <td>";

    String htmlTail = "\<input list=\"wifilist\" name=\"ssidInput\" onkeyup=\"checkSecurity()\" onselect=\"checkSecurity() \" onchange=\"checkSecurity()\"><datalist id=\"wifilist\"></datalist></td> </tr> <tr id=\"passfield\"> <td>Password :</td> <td><input type=\"password\" name=\"pass\" id=\"pass\"></td> </tr> <tr><td><br></td></tr> <!-- blank space --> <!-- IP configuration start --> <!-- manual config start --> <!-- manual config end --> <!-- IP configuration end --> <tr> <td colspan=\"2\" style=\"text-align: center\"><br><input id=\"btn\" type=\"submit\" value=\"Update Setup\"></td> </tr> <!-- blank space --> </table> </form> </div> </body> </html>";
    html += htmlTail;
    ConfigServer.send(200, "text/html", html);
    Serial.println("/config.html");
  });

  // config update command
  ConfigServer.on("/updateconfig", []() {
    Serial.println("/updateconfig");
    // get all post data
    String wifimode = URLDecode(ConfigServer.arg("wifimode"));
    String ssid = URLDecode(ConfigServer.arg("ssidInput"));
    String encryption = URLDecode(ConfigServer.arg("encryption")); // protected/open
    String pass = URLDecode(ConfigServer.arg("pass"));
    String host = URLDecode(ConfigServer.arg("host"));


    /* Input error checking */
    String ErrMsg;
    boolean err = false;
    IPAddress test;

    if (host.length() < 3 || host.length() > 20) {
      ErrMsg = "Invalid host name length (min 3, max 20)";
      err = true;
    } else if (ssid.length() < 3 || ssid.length() > 30) {
      ErrMsg = "Invalid SSID length (min 3, max 30)";
      err = true;
    } else if (encryption != "true" && encryption != "false") { // important parameters missing
      ErrMsg = "Invalid protection type";
      err = true;
    } else if (encryption == "true" && pass.length() < 8) { // password missing for encrypted wifi
      ErrMsg = "Invalid password length (min 8)";
      err = true;
    }

    if (err) {
      String content = String(F("<!DOCTYPE HTML>\r\n"
                                "<html>\n"
                                "  <body style=\"text-align: center;\">\n")) +
                       ErrMsg
                       + F("  </body>\n"
                           "</html>");
      ConfigServer.send(200, "text/html", content);
      return;
    }
    /* Input error checking ends */

    /* writing to storage start */
    /*
      DB.clear();
      DB.setWiFiMode(wifimode);
      DB.setSSID(ssid);
      if (encryption == "true") {
      DB.setProtection(byte(1));
      DB.setPassword(pass);
      } else {
      DB.setProtection(byte(0));
      }
      DB.setHostname(host);
      if (configmode == "manual") {
      DB.setConfigMode(byte (1));
      DB.setIP(ip);
      DB.setGateway(gateway);
      DB.setSubnet(subnet);
      } else {
      DB.setConfigMode(byte (0));
      }
      DB.finalize();*/
    /* writing to storage end */

    String content = "<!DOCTYPE HTML>\r\n"
                     "<html>\n"
                     "  <body style=\"text-align: center;\">\n"
                     "    <h3>WiFi settings updated</h3>\n"
                     "    <h4>SSID: " + ssid + "</h4>\n"
                     "    <h4>Protection: " + encryption + "</h4>\n"
                     "    <h4>Host: " + host + "</h4>\n"
                     "    <h3>Restart to Connect</h3>\n"
                     "  </body>\n"
                     "</html>";

    ConfigServer.send(200, "text/html", content);

    delay(100);
    // restart ESP

    ledBlinker.stop();
    ESP.restart();
  });

  // error page
  ConfigServer.onNotFound([]() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += ConfigServer.uri();
    message += "\nMethod: ";
    message += (ConfigServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += ConfigServer.args();
    message += "\n";
    for (uint8_t i = 0; i < ConfigServer.args(); i++) {
      message += " " + ConfigServer.argName(i) + ": " + ConfigServer.arg(i) + "\n";
    }
    ConfigServer.send(404, "text/plain", message);
  });

  // starting the local server with the defined handlers
  Serial.println("HTTP server started");
  ConfigServer.begin();
}

void ConfigServerRun() {
  ConfigServer.handleClient();
}



void scanWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void scanNetworks()
{
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
#if VERBOSE
  Serial.println("scan done");
#endif

  if (n == 0)
    Serial.println("no networks found");
  else
  {
    /*Serial.print(n);
      Serial.println(" networks found");*/
    int sginal;

    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found

      sginal = map(WiFi.RSSI(i), MIN_dBm, MAX_dBm, 0, MAXSTRENGTH);
      if ( sginal > 0 )
      {
        Serial.print("[");
        networksStr += "^[";
        for (int j = 1; j <= MAXSTRENGTH; j ++)
        {
          Serial.printf("%s", (j <= sginal) ? "I" : ".");
          networksStr += (j <= sginal) ? "I" : ".";
        }
        Serial.print("] ");
        networksStr += "] ";
#if VERBOSE
        Serial.printf("%-2d: %-16s ", i + 1, WiFi.SSID(i).c_str());
#else
        Serial.print(WiFi.SSID(i));
        networksStr += WiFi.SSID(i);
#endif


#if VERBOSE
        printEncryptionType(WiFi.encryptionType(i));
#else
        Serial.println("");
#endif
        delay(10);
      }
    }
    Serial.println("=====================================================");
  }
}

bool connectToWiFi()
{
  scanWiFi();
  scanNetworks();
  Serial.print("Configuring access point...");
  WiFi.softAP("ESPap", "12345678");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  initConfigServer();
  ledBlinker.start(0.5);
  return false;
}


