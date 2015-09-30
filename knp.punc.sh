#!/bin/bash
cd /home/wiseqa/src/KNP-WEB/knp++
./knp++ -i 2 -o xml -r rule/korean.punc -d dic/korean.punc --strict_pa_checking < $1 >  $1".out.xml"
