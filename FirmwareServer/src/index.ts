const fs = require('fs');
const http = require('http');
const express = require('express');

const app = express();

app.use((req, res) => {
	res.send('Hello there !');
});

const httpServer = http.createServer(app);

httpServer.listen(8000, () => {
	console.log('HTTP Server running on port 8000');
});
