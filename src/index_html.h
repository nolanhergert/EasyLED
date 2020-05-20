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
</body>
</html>



)=====";