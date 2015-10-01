<html><head><title>Sensor settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">

</head>
<body>
<div id="main">
<p>
<b>Sensor Settings</b>
</p>
<form name="sensorform" action="sensor.cgi" method="post">

<table>
<tr><td>Temp enabled?:</td><td><input type="checkbox" name="sensor-temp-enable" id="sensor-temp-enable" %sensor-temp-enable% /></td></tr>
<tr><td>Temp/Humi enabled?:</td><td><input type="checkbox" name="sensor-temp-humi-enable" id="sensor-temp-humi-enable" %sensor-temp-humi-enable% /></td></tr>
<tr><td>Thermostat input:</td><td>

<select name="thermostat1-input" id="thermostat1-input">
  <option value="0" %selectedds18b20% >DS18B20 sensor #1</option>
  <option value="1" %selecteddht22t% >DHT22 temperature</option>
  <option value="2" %selecteddht22h% >DHT22 humidity</option>
  <option value="3" %selectedmqtt% >MQTT</option>
  <option value="4" %selectedserial% >Serial input</option>  
  <option value="5" %selectedfixed% >Fixed value (10)</option>    
</select>
<tr><td>Thermostat hysteresis high:</td><td><input type="text" name="thermostat1hysteresishigh" id="thermostat1hysteresishigh" value="%thermostat1hysteresishigh%" /> (in tenths of a degree, 50 means 0.5 degrees)</td></tr>
<tr><td>Thermostat hysteresis low:</td><td><input type="text" name="thermostat1hysteresislow" id="thermostat1hysteresislow" value="%thermostat1hysteresislow%" /> (in tenths of a degree, 50 means 0.5 degrees)</td></tr>

</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="save" value="Save">
</p>
</form>

</body>
</html>
