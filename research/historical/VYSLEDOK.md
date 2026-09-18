> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# VĂ˝sledok experimentu

## Verdikt

**KEEP AS INTERNAL PRIMITIVE** â€” implicitnĂ˝ simplex je veÄľmi dobrĂ˝ malĂ˝ stavebnĂ˝ prvok pre algoritmy, ktorĂ© potrebujĂş deterministickĂ©, navzĂˇjom rovnako vzdialenĂ© stavy alebo smery. Nie je sĂˇm osebe novĂ˝m algoritmom a neodĂ´vodĹuje veÄľkĂş verejnĂş kniĹľnicu.

## DĂ´kaz

Pre `V(i,j)=n`, keÄŹ `i=j`, a `-1` inak, mĂˇ kaĹľdĂ˝ riadok jednu hodnotu `n` a `n` hodnĂ´t `-1`. Preto je sĂşÄŤet kaĹľdĂ©ho riadku nulovĂ˝, norma na druhĂş je `nÂ˛+n=n(n+1)` a pre dve rĂ´zne riadky je skalĂˇrny sĂşÄŤin `-(n+1)`. Rozdiel dvoch rĂ´znych riadkov mĂˇ dve nenulovĂ© zloĹľky `+(n+1)` a `-(n+1)`, teda vzdialenosĹĄ na druhĂş je `2(n+1)Â˛`. Gramova matica mĂˇ jedno nulovĂ© vlastnĂ© ÄŤĂ­slo a zvyĹˇnĂ˝ch `n` kladnĂ˝ch, takĹľe rank je `n`.

Pri expanzii sa nemenĂ­ identita vertexu; zmenĂ­ sa iba spoloÄŤnĂ˝ parameter `n`. NovĂ˝ vertex je index `n+1`. Preto je expanzia presne jeden skalĂˇrny zĂˇpis a neprechĂˇdza starĂ© vertexy.

## ImplementĂˇcia a zloĹľitosĹĄ

`simplex.h` obsahuje bez-heapovĂ© API. SĂşradnica, validĂˇcia, dot product, norma a vzdialenosĹĄ sĂş `O(1)`. ReprezentĂˇcia vertexu je `(n,index)`, teda `O(1)`. MaterializĂˇcia celĂ©ho vertexu je `O(n)` a vĹˇetkĂ˝ch sĂşradnĂ­c `O(nÂ˛)`. `sx_expand` je `O(1)`.

## Overflow

Pre signed typ pri `n(n+1)` sĂş bezpeÄŤnĂ© maximĂˇ: `int8_t 10`, `int16_t 180`, `int32_t 46340`, `int64_t 3037000499`. Pre `2(n+1)Â˛`: `int8_t 6`, `int16_t 126`, `int32_t 32766`, `int64_t 2147483646`. ReferenÄŤnĂ© API pouĹľĂ­va `uint64_t` pre odvodenĂ© hodnoty a pri `uint16_t n` je potrebnĂˇ kontrola, ak sa vĂ˝sledok zuĹľuje do menĹˇieho typu.

## Overenie

`simplex_test.c` exhaustĂ­vne kontroluje `n=0..255`, centroid, normu, dot product, vzdialenosĹĄ, validĂˇciu a deterministickĂş expanziu aĹľ po `uint16_t` limit. Benchmark meria opakovanĂş implicitnĂş operĂˇciu bez presĂşvania dĂˇt.

## Embedded posĂşdenie

Na AVR/M0 je integer varianta lacnĂˇ a nevyĹľaduje FPU, tabuÄľku ani inicializĂˇciu. NormalizovanĂˇ a rekurzĂ­vna `R^n` varianta potrebuje nĂˇsobenie, delenie a ÄŤasto odmocniny, takĹľe patrĂ­ len do vrstvy, ktorĂˇ skutoÄŤne potrebuje euklidovskĂ© sĂşradnice. NajlepĹˇĂ­ konkrĂ©tny use-case je internĂ© kĂłdovanie `N` rovnocennĂ˝ch stavov/senzorov a deterministickĂ© probe directions; prĂ­nos treba potvrdiĹĄ aplikĂˇciou, nie iba geometriou.

