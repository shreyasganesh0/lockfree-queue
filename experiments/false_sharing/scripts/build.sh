#!/bin/bash

gcc -D_GNU_SOURCE -lpthread -O3 src/false_sharing.c -o bin/false_sharing -Iinclude
