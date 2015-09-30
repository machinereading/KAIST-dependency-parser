#!/bin/sh
/root/HOME/bin/knp++ -i 2 -o xml -r rule/korean.functional -d dic/korean.functional --strict_pa_checking < $1 >  $1".out.xml"
