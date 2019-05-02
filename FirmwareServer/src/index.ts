const fs = require('fs');
const http = require('http');
const https = require('https');
const express = require('express');

const app = express();

// Certificate
const privateKey = fs.readFileSync('/etc/letsencrypt/live/baaka.io/privkey.pem', 'utf8');
const certificate = fs.readFileSync('/etc/letsencrypt/live/baaka.io/cert.pem', 'utf8');
const ca = fs.readFileSync('/etc/letsencrypt/live/baaka.io/chain.pem', 'utf8');

const credentials = {
	key: privateKey,
	cert: certificate,
	ca: ca
};

app.use((req, res) => {
	res.send('Hello there !');
});

const httpsServer = https.createServer(credentials, app);

httpsServer.listen(8080, () => {
	console.log('HTTPS Server running on port 8080');
});
