<html><head><title>ws2812b control</title>
<script>
function form_init() {
    var allElements=document.getElementsByTagName('input');
     var len=allElements.length;
     for(var i =0; i < len; i++)
     {
          var formElement = allElements[i];
          if(formElement.type=="range" || formElement.type=="text" || formElement.type=="radio" || formElement.type=="submit")
          {
		console.log("type: " + formElement.type + " name: " + formElement.name + " value: " + formElement.value + " id: " + formElement.id)
		  formElement.onchange= function (e) {
			return function ()
			{
				submit_input(e)
			};
		}(formElement);
          }
     }     
}
function submit_input(element) {
        value = element.value;
        name = element.name; 
       params = name + "=" + value;
       console.log( "name: " +  element.name + " value: " + element.value + " params: " + params) 
       // the done function obtains the result 
       //from the backend and puts the results in a div
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
      if (xhttp.readyState == 4 && xhttp.status == 200) {
        console.log("blah blah blah")
       }
     };

  // Send the data using post
  xhttp.open("POST", "ws2812b.cgi", true);
  xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhttp.send(params);
}
</script>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
</head>
<body onload="form_init()">
<div id="main">
<h1>RGB LED control page</h1>
<form>
<table>
<tr><td><input type="text" name="stringlen" id="stringlen" value="%cur_stringlen%" >String Length</td></tr>
%pattern_select%
<tr><td><input type="range" defaultValue="500" max="1002" min="1" step="50" name="ms_delay" id="ms_delay" value="%cur_delay%" >Pattern Delay (ms)</td></tr>
<tr><td><input type="range" defaultValue="0" max="8" min="0" step="1" name="brightness" id="brightness" value="%cur_brightness%" >Brightness</td></tr>
<tr><td><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>
<p>
<button onclick="location.href = '/';" class="float-left submit-button" >Back</button>
</div>
</body></html>
