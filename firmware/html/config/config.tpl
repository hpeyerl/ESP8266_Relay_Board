<html><head><title>Override config Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript">

window.onload=function(e) {
	sc('board_id',%board_id%);
};

function sc(l,v) {
document.getElementById(l).checked=v;}

</script>

</head>
<body>
<div id="main">
<p>
<b>Misc config settings</b>
</p>
<pre>
BOARD_ID_RELAY_BOARD		0x0
BOARD_ID_PHROB_THERMOCOUPLE	0x1
BOARD_ID_PHROB_TEMP_HUM		0x2
BOARD_ID_PHROB_HALL_EFFECT	0x3
BOARD_ID_PHROB_WATER		0x4
BOARD_ID_PHROB_TILT		0x5
BOARD_ID_PHROB_DUAL_RELAY	0x81
BOARD_ID_PHROB_SINGLE_RELAY	0x82
BOARD_ID_PHROB_SIGNAL_RELAY	0x83
</pre>
<form name="configform" action="config.cgi" method="post">

<table>
<tr><td>Board ID:</td><td><input type="text" name="board_id" id="board_id" value="%board_id%"/>     </td></tr>
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>

</body>
</html>
