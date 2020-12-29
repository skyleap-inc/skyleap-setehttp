/*
Commands:
   1 - write file to folder
   2 - get file
   3 - delete file
*/

var express = require('express');
var bodyParser = require('body-parser');
var app = express();
app.disable('x-powered-by');
app.disable('etag');

// parse application/x-www-form-urlencoded
app.use(bodyParser.urlencoded({ extended: false }))

// parse application/json
app.use(bodyParser.json());

// WRITE FILE
app.post('/1', function (req, res) {
    return res.send('Hello from server. POST vars: ' + JSON.stringify(req.body, null, 2));
});

// GET FILE
app.post('/2', function (req, res) {
    return res.send('Hello from server. POST vars: ' + JSON.stringify(req.body, null, 2));
});

// DELETE FILE
app.post('/3', function (req, res) {
	return res.send('Hello from server. POST vars: ' + JSON.stringify(req.body, null, 2));
});

var server = app.listen(8080, function () {
   var host = server.address().address;
   var port = server.address().port;
   console.log("Listening %s:%s", host, port);
});