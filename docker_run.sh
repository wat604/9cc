#!/bin/bash
docker run --rm -v $PWD:$PWD -w $PWD compilerbook_x86_64 "$@"