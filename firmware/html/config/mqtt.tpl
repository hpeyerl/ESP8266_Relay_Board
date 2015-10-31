<html><head><title>MQTT settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript">

window.onload=function(e) {
	sc('mqtt-enable',%mqtt-enable%);
	sc('mqtt-use-ssl',%mqtt-use-ssl%);
	di();
};

function sc(l,v) {
document.getElementById(l).checked=v;}

function sd(l,v) {
if(document.getElementById(l)) document.getElementById(l).disabled=v;}

function di(){
var v=true;
if (document.getElementById('mqtt-enable').checked) v=false;
sd('mqtt-host',v);
sd('mqtt-port',v);
sd('mqtt-keepalive',v);
sd('mqtt-deep-sleep-time',v);
sd('mqtt-devid',v);
sd('mqtt-user',v);
sd('mqtt-pass',v);
sd('mqtt-use-ssl',v);
sd('mqtt-send-config',v);
sd('mqtt-relay-subs-topic',v);
sd('mqtt-temphum-temp-pub-topic',v);
sd('mqtt-temphum-humi-pub-topic',v);
sd('mqtt-temp-pub-topic',v);

}

</script>

</head>
<body>
<div id="main">
<p>
<b>MQTT Settings</b>
</p>
<form name="mqttform" action="mqtt.cgi" method="post">

<table>
<tr><td>MQTT enabled?:</td><td><input type="checkbox" name="mqtt-enable" id="mqtt-enable" onclick="di();" %mqtt-enabled% /></td></tr>
<tr><td>Host:</td><td><input type="text" name="mqtt-host" id="mqtt-host" value="%mqtt-host%"/>     </td></tr>
<tr><td>Port:</td><td><input type="text" name="mqtt-port" id="mqtt-port" value="%mqtt-port%"/>     </td></tr>
<tr><td>Keepalive (sec):</td><td><input type="text" name="mqtt-keepalive" id="mqtt-keepalive" value="%mqtt-keepalive%"/> </td></tr>
<tr><td>Deep Sleep Time (sec):</td><td><input type="text" name="mqtt-deep-sleep-time" id="mqtt-deep-sleep-time" value="%mqtt-deep-sleep-time%"/> </td></tr>
<tr><td>Device ID:</td><td><input type="text" name="mqtt-devid" id="mqtt-devid" value="%mqtt-devid%"/>   </td></tr>
<tr><td>User:</td><td><input type="text" name="mqtt-user" id="mqtt-user" value="%mqtt-user%"/>     </td></tr>
<tr><td>Password:</td><td><input type="password" name="mqtt-pass" id="mqtt-pass" value="%mqtt-pass%"/>     </td></tr>
<tr><td>Use SSL?:</td><td><input type="checkbox" name="mqtt-use-ssl" id="mqtt-use-ssl" /> (max 1024 bit key size)  </td></tr>
<tr><td>Send IO Config?:</td><td><input type="checkbox" name="mqtt-send-config" id="mqtt-send-config" %mqtt-send-config-enabled% />  </td></tr>
<tr><td>Relays subs topic:</td><td><input type="text" name="mqtt-relay-subs-topic" id="mqtt-relay-subs-topic" value="%mqtt-relay-subs-topic%">     </td></tr>
<tr><td>Relay pulse subs topic:</td><td><input type="text" name="mqtt-relay-subs-pulse-topic" id="mqtt-relay-subs-pulse-topic" value="%mqtt-relay-subs-pulse-topic%">     </td></tr>
%config_temphum1%
%config_temphum2%
%config_temp%
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>

</body>
</html>
