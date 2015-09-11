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
RELAY_BOARD		0
PHROB_DUAL_RELAY	1
PHROB_SINGLE_RELAY	2
PHROB_THERMOCOUPLE	3
PHROB_TEMP_HUM		4
PHROB_HALL_EFFECT	5
PHROB_WATER		6
PHROB_TILT		7
PHROB_SIGNAL_RELAY	8
</pre>
<form name="configform" action="config.cgi" method="post">

<table>
<tr><td>Board ID:</td><td><input type="text" name="board_id" id="board_id" value="%board_id%"/>     </td></tr>
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>

</body>
</html>
