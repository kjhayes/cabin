#! /usr/bin/bash

jp2a --width=80 --height=25 $1 | tr -d '\r' | tr -d '\n'
