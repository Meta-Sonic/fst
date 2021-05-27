#!/bin/bash
cd include
for filename in fst/*.h; do
  echo $filename
  name=$(echo "$filename" | cut -f 1 -d '.')
  echo $name
  echo "// -*- C++ -*-
#pragma once 
#include <$name.h>" > $name
done