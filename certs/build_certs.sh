#!/bin/bash

openssl genrsa -des3 -out server.key 1024

#Generate Certificate signing request
openssl req -new -key server.key -out server.csr

#Sign certificate with private key
openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt

#Remove password requirement
cp server.key server.key.secure
openssl rsa -in server.key.secure -out server.key

#Generate dhparam file
openssl dhparam -out dh2048.pem 2048
