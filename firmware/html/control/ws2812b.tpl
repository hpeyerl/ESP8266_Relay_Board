<html><head><title>ws2812b control</title>
</head>
<body>
<div id="main">
<h1>WS2812B control page</h1>
<form method="get" action="ws2812b.cgi">
<table>
<tr><td><input type="submit" name="pattern1" value="1">Pattern1 on</td></tr>
<tr><td><input type="submit" name="pattern1" value="0">Pattern1 off</td></tr>
<tr><td><input type="submit" name="pattern2" value="1">Pattern2 on</td></tr>
<tr><td><input type="submit" name="pattern2" value="0">Pattern2 off</td></tr>
<tr><td><input type="text" name="red" id="red" value="255">Red</td></tr>
<tr><td><input type="text" name="green" id="green" value="255">Green</td></tr>
<tr><td><input type="text" name="blue" id="blue" value="255">Blue</td></tr>
</table>
</form>
<p>
<button onclick="location.href = '/';" class="float-left submit-button" >Back</button>
</div>
</body></html>
