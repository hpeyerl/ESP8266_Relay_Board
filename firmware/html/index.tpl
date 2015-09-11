<html>
<head><title>ESP8266 Relay Board</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>Main - Board ID: %board_id%</h1>
<p>
<ul>
<li><a href="config/wifi/wifi.tpl">WiFi</a> settings.</li>
<li><a href="config/mqtt.tpl">MQTT</a> settings.</li>
<li><a href="config/httpd.tpl">HTTP Daemon</a> settings.</li>
<li><a href="config/ntp.tpl">NTP</a> settings.</li>
<li><a href="config/sensor.tpl">Sensor</a> settings.</li>
<li><a href="config/broadcastd.tpl">Broadcast Daemon</a> settings.</li>
<li>Relay <a href="config/relay.tpl">settings</a>.</li>
<li><a href="control/relay.html">Relay</a> control page.</li>
<li><a href="control/thermostat.html">Thermostat</a> control page.</li>
<li><a href="control/thingspeak.html">Thingspeak</a> report page.</li>
<li>Sensor readings:</li>
<li>    <a href="control/dht22.tpl">DHT22</a></li>
<li>    <a href="control/ds18b20.tpl">DS18B20</a>.</li>
<li>    <a href="control/si7020.tpl">SI7020</a></li>
<li>    <a href="control/max31855.tpl">MAX31855</a>.</li>
<li><a href="control/reset.cgi" onclick="return confirm('Are you sure you want to restart?')">Restart</a> the system.</li>
<li><a href="about.tpl">About</a></li>
</ul>
</p>
</div>
</body></html>
