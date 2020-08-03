#include <avr/pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"=====(



<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">


<style>



.radio-toolbar {
  margin: 10px;
  /* Only have room for two colums */
  width: 160px;
}

.radio-toolbar input[type="radio"] {
  opacity: 0;
  position: fixed;
  width: 0;
}

.radio-toolbar label {
  display: inline-block;
  background-color: #ddd;
  font-family: sans-serif, Arial;
  font-size: 16px;
  border: 2px solid #444;
  border-radius: 4px;
  width: 40px;
  height: 40px;
  text-align: center;
}

.radio-toolbar label:hover {
  background-color: #dfd;
}

.radio-toolbar input[type="radio"]:focus + label {
  border: 2px dashed #444;
}

.radio-toolbar input[type="radio"]:checked + label {
  background-color: #bfb;
  border-color: #4c4;
}

.pair {
  margin: 10px;
  float: left;
}

</style>

</head>
<body>
<h1>Hello world 4!</h1>
<label for="favcolor">Select your favorite color:</label>
<input type="color" id="favcolor" name="favcolor" value="#ff0000">
<label>Choose a pattern:
  <select class="pattern" name="pattern">
    <option value="chocolate">Chocolate</option>
  </select>
</label>
<p></p>
<div class="result"></div>
<p></p>
<div class="slidecontainer">
  <input type="range" min="1" max="255" value="50" class="slider" id="myRange">
  <p>Value: <span id="demo"></span></p>
</div>

<input type="button" style="border-style:inset;" value="Port 1"/>


<div class="radio-toolbar">
  <div class="pair">
    <input type="radio" id="radioOne" name="radioPin" value="1" checked>
    <label for="radioOne">1</label>
    </br>
    <input type="radio" id="radioTwo" name="radioPin" value="2">
    <label for="radioTwo">2</label>
  </div>
  <div class="pair">
    <input type="radio" id="radioFive" name="radioPin" value="5">
    <label for="radioFive">5</label> 
    </br>
    <input type="radio" id="radioSix" name="radioPin" value="6">
    <label for="radioSix">6</label> 
  </div>

<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">


<style>



.radio-toolbar {
  margin: 10px;
  /* Only have room for two colums */
  width: 160px;
}

.radio-toolbar input[type="radio"] {
  opacity: 0;
  position: fixed;
  width: 0;
}

.radio-toolbar label {
  display: inline-block;
  background-color: #ddd;
  font-family: sans-serif, Arial;
  font-size: 16px;
  border: 2px solid #444;
  border-radius: 4px;
  width: 40px;
  height: 40px;
  text-align: center;
}

.radio-toolbar label:hover {
  background-color: #dfd;
}

.radio-toolbar input[type="radio"]:focus + label {
  border: 2px dashed #444;
}

.radio-toolbar input[type="radio"]:checked + label {
  background-color: #bfb;
  border-color: #4c4;
}

.pair {
  margin: 10px;
  float: left;
}


</style>

</head>
<body>
<h1>Hello world 4!</h1>

<div class="radio-toolbar">
  <div class="pair">
    <input type="radio" id="radioOne" name="radioPin" value="1" checked>
    <label for="radioOne">1</label>
    </br>
    <input type="radio" id="radioTwo" name="radioPin" value="2">
    <label for="radioTwo">2</label>
  </div>
  <div class="pair">
    <input type="radio" id="radioFive" name="radioPin" value="5">
    <label for="radioFive">5</label> 
    </br>
    <input type="radio" id="radioSix" name="radioPin" value="6">
    <label for="radioSix">6</label> 
  </div>
  <div class="pair">
    <input type="radio" id="radioThree" name="radioPin" value="3">
    <label for="radioThree">3</label> 
    </br>
    <input type="radio" id="radioFour" name="radioPin" value="4">
    <label for="radioFour">4</label> 
  </div>
  <div class="pair">
    <input type="radio" id="radioSeven" name="radioPin" value="7">
    <label for="radioSeven">7</label> 
    </br>
    <input type="radio" id="radioEight" name="radioPin" value="8">
    <label for="radioEight">8</label> 
  </div>  
</div>



<label>Choose a pattern:
  <select class="pattern" name="pattern">
    <option value="empty"></option>
    <option value="combine">_Combine With</option>
    <option value="chocolate">Chocolate</option>
  </select>
</label>
<p></p>

<!-- HTML classes for customizing patterns chosen -->
<div class="customizer" id="combineCustomizer">
<label>Choose Pin: 
  <select class="pattern" name="combine_pin">
    <option value="1">1</option>
    <option value="2">2</option>
    <option value="3">3</option>
    <option value="4">4</option>
    <option value="5">5</option>
    <option value="6">6</option>
    <option value="7">7</option>
    <option value="8">8</option>
  </select>
</label>

<!-- Add an offset input text box here? -->

</div>



<div class="customizer" id="normalCustomizer">
<div class="slidecontainer">
  <input type="range" min="1" max="255" value="50" class="slider" id="myRange">
  <p>Value: <span id="demo"></span></p>
</div>

<label for="favcolor">Select your favorite color:</label>
<input type="color" id="favcolor" name="favcolor" value="#ff0000">

</div>



<!------------------------------------------------->


<script>


var output = document.getElementById("demo");

// Set up defaults
hideAllCustomizers();
// Load already saved data from wemos d1 mini

let l = new Lion('Fuzzy');

// Can call selected animation which will output itself and the customizer query parameters


class Animation {
  constructor() {
    this.customizer = document.getElementById("normalCustomizer");
  }
  
  hideCustomizer() {
    this.customizer.style.display = "none";
  }
    
  showCustomizer() {
    this.customizer.style.display = "block";
  }
  
  output() {
}

class Combine extends Animation {
  constructor() {
    super.contructor();
    this.customizer = document.getElementById("combineCustomizer");
  }
}
    


// Selector on change
const selectElement = document.querySelector('.pattern');
selectElement.addEventListener('change', (event) => {
  hideAllCustomizers();
  switch(event.target.value) {
    case "chocolate":
      document.getElementById("normalCustomizer").style.display = "block";
      break;
    case "combine":
      document.getElementById("combineCustomizer").style.display = "block";
      break;
  }
  //fetch(`/set?brightness=${event.target.value}`);
});


// Slider on change
var slider = document.getElementById("myRange");

output.innerHTML = slider.value;
slider.oninput = function() {
  output.innerHTML = this.value;
  //fetch(`/set?brightness=${this.value}`);
}


// Pin selection on change
var x = document.getElementsByName("radioPin");
// loop through list
for (var i=0, len=x.length; i<len; i++) {
  x[i].onclick = function() { // assign onclick handler function to each
    // put clicked radio button's value in total field
    output.innerHTML = this.value;
    document.getElementById("favcolor").value = "#0000FF"
  };
}


function hideAllCustomizers() {
  // Go to each element in class and hide itself
  var divsToHide = document.getElementsByClassName("customizer"); //divsToHide is an array
  for(var i = 0; i < divsToHide.length; i++){
    divsToHide[i].style.display = "none"; // collapse space used
  }
}

function applyCurrentValues() {
  // Go to pattern class and get url string


}

</script>
</body>
</html>


)=====";
