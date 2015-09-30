# KAIST dependency parser

## Description

soon

## How to use

For the raw text, it is required to run [SMA](https://github.com/machinereading/SMA-Korean) morphological analyzer

For the result of SMA:

```
./TransFormToJuman.o -i "SMA 분석결과 파일" -o "출력결과 파일" -punc
```

For the result of depenency parser:
```
./knp.baseline.sh "output.file"
```

`Output file: output.file.out.xml`

## Licenses

* `CC BY-NC-SA` [Attribution-NonCommercial-ShareAlike](https://creativecommons.org/licenses/by-nc-sa/2.0/)
* If you want to commercialize this resource, [please contact to us](http://mrlab.kaist.ac.kr/contact)



