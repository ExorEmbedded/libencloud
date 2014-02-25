#!/bin/bash

env > /tmp/auth.out

[ "${username}" != "user" ] && exit 1
[ "${password}" != "pass" ] && exit 1

exit 0
