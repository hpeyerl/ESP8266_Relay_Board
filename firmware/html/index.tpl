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
<li><a href="config/config.tpl">Device_ID</a>settings.</li>
<li><a href="config/wifi/wifi.tpl">WiFi</a> settings.</li>
%config_mqtt%
<li><a href="config/httpd.tpl">HTTP Daemon</a> settings.</li>
<li><a href="config/ntp.tpl">NTP</a> settings.</li>
<li><a href="config/sensor.tpl">Sensor</a> settings.</li>
<li><a href="config/broadcastd.tpl">Broadcast Daemon</a> settings.</li>
%config_relays%
<li><a href="control/thermostat.html">Thermostat</a> control page.</li>
<li><a href="control/thingspeak.html">Thingspeak</a> report page.</li>
%control_ws2812%
%config_sensors%
%config_dht22%
%config_si7020%
%config_max31855%
%config_sensors_end%
<li><a href="control/reset.cgi" onclick="return confirm('Are you sure you want to restart?')">Restart</a> the system.</li>
<li><a href="about.tpl">About</a></li>
</ul>
</p>
</div>
</body></html>
