#include <avr/pgmspace.h>

// For now just edit the below in a separate .html file that you create
// However at some point, it'll be worth it to generate this .h file using
// a build time script from an actual .html file that's checked in and slightly
// easier to use! 
// https://docs.platformio.org/en/latest/projectconf/advanced_scripting.html
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
    <option value="Animation">Animation</option>
  </select>
</label>
<p></p>

<!-- HTML classes for customizing patterns chosen -->
<div class="customizer" id="AnimationCustomizer">
	<div class="slidecontainer">
	  Brightness:
	  <input type="range" min="1" max="255" value="50" class="slider" name="brightness" >
	  
	</div>

	<label for="favcolor">Select your favorite color:</label>
	<input type="color" id="favcolor" name="favcolor" value="#ff0000">

</div>


<div class="customizer" id="CombineCustomizer">
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

	<!-- Add an pattern offset input text box here? -->

</div>


<!------------------------------------------------->


<script>


// Set up defaults
// Load already saved data from wemos d1 mini

let activeCustomizer = 0;
let currentPin = 1;




function onInput(e) {
  // Unfortunately we need this global handler
  // Here's some docs to handle e: https://developer.mozilla.org/en-US/docs/Web/API/GlobalEventHandlers/oninput
  // But we're just going to publish the currently active customizer
  if (activeCustomizer != 0) {
    activeCustomizer.publish();
  }

} 

class AnimationCustomizer {
  constructor() {
    this.div = document.getElementById("AnimationCustomizer");
  }
  
  setup() {
	this.hide();
	this.inputs = this.div.getElementsByTagName('input');
	// Set up onChanged callback for each input element
	// to send out the full query string for the form (for now)
	for (let input of this.inputs) {
	  // We want to use oninput() as it's live, whereas onchanged()
	  // is only after the object loses focus.
	  input.oninput = onInput;
	}	
  }
  
  hide() {
    this.div.style.display = "none";
  }
    
  show() {
    this.div.style.display = "block";
  }
  
  publish() {
  // Send 
	// Return a string with the inputs in query string form
	console.log(currentPin);
	for (let input of this.inputs) {
	  console.log(input.name);
	  console.log(input.value);
	  
      //fetch(`/set?brightness=${this.value}`);
	}
  }
}

class CombineCustomizer extends AnimationCustomizer {
  constructor() {
    super();
    this.div = document.getElementById("CombineCustomizer");
  }
}



let animationCustomizer = new AnimationCustomizer();//.setup();
animationCustomizer.setup();
let combineCustomizer = new CombineCustomizer();//.setup();
combineCustomizer.setup();


	

// Selector on change
const selectElement = document.querySelector('.pattern');
selectElement.addEventListener('change', (event) => {
  //hideAllCustomizers();
  animationCustomizer.hide();
  combineCustomizer.hide();
  
  switch(event.target.value) {
    case "Animation":
	  activeCustomizer = animationCustomizer;
      break;
    case "combine":
	  activeCustomizer = combineCustomizer;
      break;
  }
  activeCustomizer.show();
  //fetch(`/set?brightness=${event.target.value}`);
});


// Pin selection on change
var x = document.getElementsByName("radioPin");
// loop through list
for (var i=0, len=x.length; i<len; i++) {
  x[i].onclick = function() { // assign onclick handler function to each
    // put clicked radio button's value in total field
    currentPin = this.value;
  };
}


</script>
</body>
</html>


)=====";
