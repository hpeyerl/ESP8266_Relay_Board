<html><head><title>ws2812b control</title>
</head>
<body>
<div id="main">
<h1>WS2812B control page</h1>
<form method="get" action="ws2812b.cgi">
<table>
<tr><td><input type="text" name="stringlen" id="stringlen" value="%cur_stringlen%" >String Length</td></tr>
<tr><td><input type="text" name="ms_delay" id="ms_delay" value="%cur_delay%" >Pattern Delay (ms)</td></tr>
%pattern_select%
</table>
<input type="submit" value="Submit">
</form>
<p>
<button onclick="location.href = '/';" class="float-left submit-button" >Back</button>
</div>
</body></html>
