#!/bin/bash
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 1000 -nodes
cat key.pem > ssl.pem; cat cert.pem >> ssl.pem
