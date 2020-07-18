#include <avr/pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"=====(



<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>Hello world</title>
</head>
<body>
<h1>Hello world 4!</h1>
<label for="favcolor">Select your favorite color:</label>
<input type="color" id="favcolor" name="favcolor" value="#ff0000">

<label>Choose an ice cream flavor:
  <select class="ice-cream" name="ice-cream">
    <option value="">Select One …</option>
    <option value="chocolate">Chocolate</option>
    <option value="255">Bright</option>
    <option value="50">Dark</option>
  </select>
</label>

<p></p>
<div class="result"></div>

<p></p>
<div class="slidecontainer">
  <input type="range" min="1" max="255" value="50" class="slider" id="myRange">
  <p>Value: <span id="demo"></span></p>
</div>


<script>
const selectElement = document.querySelector('.ice-cream');
selectElement.addEventListener('change', (event) => {
  fetch(`/set?brightness=${event.target.value}`);
});

var slider = document.getElementById("myRange");
var output = document.getElementById("demo");
output.innerHTML = slider.value;

slider.oninput = function() {
  output.innerHTML = this.value;
  fetch(`/set?brightness=${this.value}`);
}
</script>
</body>
</html>


)=====";