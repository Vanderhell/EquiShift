> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# Use-case analysis: nearest simplex vertex

## Verdikt

**USEFUL SPECIALIZED PRIMITIVE**

ImplicitnĂ˝ simplex odstrĂˇni uloĹľenĂş maticu a celĂ˝ vĂ˝poÄŤet skĂłre pri klasifikĂˇcii vstupu voÄŤi `N` rovnocennĂ˝m simplexovĂ˝m stavom. VĂ˝sledok je vĹˇeobecne uĹľitoÄŤnĂ˝ pre integer state/sensor klasifikĂˇciu, ale nie je lepĹˇou reprezentĂˇciou pre obyÄŤajnĂ˝ enum, bitmasku alebo one-hot dĂˇta.

## 1. AlgebraickĂˇ redukcia

OznaÄŤme `S = sum(x[j])`. Pre vertex `i` platĂ­:

```text
x Â· V_i = x[i] n + sum(j != i, -x[j])
          = n*x[i] - (S - x[i])
          = (n+1)*x[i] - S
```

Pre pevnĂ© `x` a `n` sĂş `(n+1)` kladnĂˇ konĹˇtanta a `-S` spoloÄŤnĂ© pre vĹˇetky vertexy. Preto:

```text
argmax_i (x Â· V_i) = argmax_i x[i]
```

To je presnĂˇ ekvivalencia, nie heuristika. StaÄŤĂ­ jeden lineĂˇrny prechod a porovnania; netreba materiĂˇlovaĹĄ vertex, nĂˇsobiĹĄ ani sÄŤĂ­taĹĄ dot product.

## 2. VzdialenosĹĄ

```text
||x - V_i||Â˛ = ||x||Â˛ + ||V_i||Â˛ - 2 xÂ·V_i
             = ||x||Â˛ + n(n+1) - 2((n+1)x[i] - S)
             = [||x||Â˛ + n(n+1) + 2S] - 2(n+1)x[i]
```

HranatĂˇ ÄŤasĹĄ je rovnakĂˇ pre vĹˇetky `i`. Preto:

```text
argmin_i ||x - V_i||Â˛ = argmax_i x[i]
```

VĹˇeobecnĂ˝ nearest-neighbor vĂ˝poÄŤet `O(NÂ˛)` arithmetic sa redukuje na `O(N)` comparisons a `O(1)` model storage.

## 3. Hyperrovina a projekcia

SimplexovĂˇ hyperrovina je `H: sum(y[j])=0`. OrtogonĂˇlna projekcia ÄľubovoÄľnĂ©ho `x` do `H` je:

```text
y[j] = x[j] - mean(x)
mean(x) = sum(x[j]) / N
```

Projekcia vĹˇak nemenĂ­ poradie komponentov: `y[i]-y[k] = x[i]-x[k]`. KeÄŹĹľe nearest vertex zĂˇvisĂ­ iba od poradia komponentov, rozhodnutie sa nezmenĂ­ a projekciu netreba vykonĂˇvaĹĄ. Toto platĂ­ aj pri celoÄŤĂ­selnom vstupe; netreba deliĹĄ ani zaokrĂşhÄľovaĹĄ.

## 4. Confidence margin

Nech `b` je najvĂ¤ÄŤĹˇia a `s` druhĂˇ najvĂ¤ÄŤĹˇia komponenta. Potom lacnĂ˝ margin je:

```text
m = b - s
```

Rozdiel vzdialenostĂ­ najlepĹˇieho a druhĂ©ho vertexu je presne:

```text
d_secondÂ˛ - d_bestÂ˛ = 2(N)m
```

Po projekcii do `H` je euklidovskĂˇ vzdialenosĹĄ od rozhodovacej hranice `x_b=x_s`:

```text
distance_to_boundary = m / sqrt(2)
```

Margin je teda presnĂˇ, monotĂłnna geometrickĂˇ confidence metrika. SamotnĂ© `sqrt()` netreba poÄŤĂ­taĹĄ, ak staÄŤĂ­ porovnĂˇvanie alebo threshold na `m`.

## 5. ImplementovanĂ© varianty

V [use_case_benchmark.c](C:\Users\vande\Desktop\Mobius slucka\Symplex\use_case_benchmark.c) sĂş:

* **Baseline A** â€“ explicitnĂˇ matica `V`, vĹˇeobecnĂ˝ dot product.
* **Baseline B** â€“ `sx_coord()` v kaĹľdej bunke, stĂˇle vĹˇeobecnĂ˝ dot product.
* **Candidate C** â€“ `argmax(x[i])`, bez vertexov a nĂˇsobenĂ­.

Vstupy vznikajĂş runtime cez volatile LCG. KlasifikĂˇtory sĂş `noinline`, vĂ˝sledok sa akumuluje do `volatile benchmark_sink` a kaĹľdĂˇ veÄľkosĹĄ mĂˇ 100 nĂˇhodnĂ˝ch ekvivalenÄŤnĂ˝ch kontrol.

## 6. Benchmark

Meranie GCC `-O2` na desktopovej referencii, `REPS=20000`, jednotka ns/op (rozlĂ­Ĺˇenie `clock()` je pribliĹľne 50 ns):

| N | A explicit | B implicitnĂ˝ vĹˇeobecnĂ˝ | C redukovanĂ˝ | A model RAM | C model RAM |
|---:|---:|---:|---:|---:|---:|
| 3 | 0 | 0 | 50 | 36 B | 4 B |
| 4 | 0 | 50 | 0 | 64 B | 4 B |
| 5 | 0 | 50 | 0 | 100 B | 4 B |
| 8 | 100 | 50 | 50 | 256 B | 4 B |
| 16 | 200 | 250 | 50 | 1 024 B | 4 B |
| 32 | 700 | 850 | 150 | 4 096 B | 4 B |
| 64 | 2 850 | 3 700 | 250 | 16 384 B | 4 B |
| 128 | 9 350 | 13 350 | 500 | 65 536 B | 4 B |
| 256 | 35 200 | 52 000 | 1 000 | 262 144 B | 4 B |

ÄŚĂ­sla sĂş referenÄŤnĂ©, nie MCU cycle count; rozhodujĂşci je rast: A/B pribliĹľne kvadraticky, C lineĂˇrne. Pri malĂ˝ch `N` ÄŤasovaÄŤ zaokrĂşhÄľuje vĂ˝sledky.

## 7. RAM, flash a assembly

ExplicitnĂ˝ baseline mĂˇ pracovnĂş maticu `4*NÂ˛` bajtov. Candidate C potrebuje iba `(n,index)` = 4 B model state; vstupnĂ˝ vektor je spoloÄŤnĂ˝ aplikaÄŤnĂ˝ vstup a nie je zapoÄŤĂ­tanĂ˝ ako modelovĂˇ RAM.

Na hoste:

```text
gcc -O2: text 16564, data 228, bss 263616
gcc -Os: text 15480, data 263364, bss 368
```

HostskĂ˝ benchmark obsahuje vĹˇetky tri varianty a 256-limitnĂş maticu, preto jeho celkovĂ˝ `.bss` nie je veÄľkosĹĄ Candidate C. Na embedded buildoch treba varianty linkovaĹĄ oddelene; modelovĂˇ Ăşspora ostĂˇva `O(NÂ˛)` versus `O(1)`.

Disassembly kritickĂ˝ch sluÄŤiek ukazuje:

* A: vnorenĂˇ sluÄŤka, load vstupu, load matice, `imul`, `add`, dve sluÄŤkovĂ© vetvy.
* B: vnorenĂˇ sluÄŤka, load vstupu, porovnanie osi s vertexom, implicitne generovanĂˇ sĂşradnica, `imul`, `add`.
* C: jedna sluÄŤka, load vstupu, compare a conditional move, bez `imul` a bez vnorenĂ©ho prechodu.

V prostredĂ­ nebol dostupnĂ˝ `arm-none-eabi-gcc`, ESP32 toolchain ani reĂˇlny target, takĹľe Cortex-M0/M0+ cycle count nebol tvrdenĂ˝. Na Cortex-M0 by C navyĹˇe nevyĹľadoval nĂˇsobenie ani delenie, ÄŤo je prakticky vĂ˝znamnejĹˇie neĹľ samotnĂ© desktopovĂ© ns.

## 8. ÄŽalĹˇie use-cases

### N-way stavovĂˇ reprezentĂˇcia

Pre obyÄŤajnĂ˝ diskrĂ©tny stav je `enum` menĹˇĂ­ a lepĹˇĂ­. Bitmask je lepĹˇĂ­ pre mnoĹľinu stavov a one-hot je jednoduchĹˇĂ­, ak je potrebnĂˇ priamo kompatibilnĂˇ vektorovĂˇ reprezentĂˇcia. Simplex prinĂˇĹˇa vĂ˝hodu iba vtedy, keÄŹ vstup uĹľ mĂˇ porovnateÄľnĂ© skĂłre/koordinĂˇty a treba geometricky symetrickĂş nearest-state klasifikĂˇciu.

### Fault/state separation

Simplex dĂˇva vĹˇetkĂ˝m dvojiciam rovnakĂş vzdialenosĹĄ a z marginu presnĂş hranicovĂş confidence. To pomĂˇha pri rovnakej chybovej tolerancii vĹˇetkĂ˝ch stavov, ale rovnakĂş vlastnosĹĄ moĹľno pre rozhodovanie dostaĹĄ aj z one-hot kĂłdov. PrĂ­nos je najmĂ¤ v nulovom centri a closed-form skĂłre, nie v lepĹˇej separĂˇcii samej osebe.

### Probe directions

`N` simplexovĂ˝ch vertexov je deterministickĂˇ mnoĹľina `N` rovnocennĂ˝ch smerov s rovnakĂ˝mi pĂˇrovĂ˝mi uhlami. Je vhodnĂˇ pre malĂ© pevnĂ© mnoĹľstvo symetrickĂ˝ch probe smerov. Nie je univerzĂˇlne lepĹˇia neĹľ `Â±axis`: axis dĂˇva jednoduchĹˇie fyzickĂ© perturbĂˇcie a hypercube/random mĂ´Ĺľe maĹĄ viac smerov alebo lepĹˇie pokrytie podÄľa Ăşlohy. Candidate C odstraĹuje generovanie smerov iba vtedy, ak downstream operĂˇcia vie pracovaĹĄ s indexom/koordinĂˇtou implicitne.

### SenzorovĂˇ klasifikĂˇcia

Pre `N` senzorovĂ˝ch tried moĹľno pouĹľĂ­vaĹĄ najvyĹˇĹˇiu normalizovanĂş sensor score. KlasifikĂˇcia je potom presne Candidate C a confidence je top-two rozdiel. Ak senzory majĂş odliĹˇnĂ˝ Ĺˇum alebo odliĹˇnĂ© nĂˇklady chyby, symetrickĂ˝ simplex je nevhodnĂ˝ bez dodatoÄŤnĂ˝ch vĂˇh; obyÄŤajnĂ˝ kalibrovanĂ˝ threshold je lepĹˇĂ­.

## 9. Limity

Simplex nepomĂˇha, ak sa vyĹľadujĂş fyzickĂ© sĂşradnice vĹˇetkĂ˝ch vertexov, ÄľubovoÄľnĂ© lineĂˇrne transformĂˇcie, rĂ´zne polomery, nerovnakĂ© vĂˇhy alebo sĂşradnice mimo simplexoÂ­vej symetrie. MaterializĂˇcia stĂˇle stojĂ­ `O(NÂ˛)`. Pri voÄľnom `N` treba oĹˇetriĹĄ overflow `N*x[i]`, ak sa pouĹľĂ­va vĹˇeobecnĂ˝ vzorec; Candidate C sa tomuto nĂˇsobeniu vyhĂ˝ba.

## ZĂˇver

Experiment naĹˇiel reĂˇlnu, presne dokĂˇzanĂş redukciu pre ĹˇpecifickĂş triedu embedded problĂ©mov: nearest-state/classification podÄľa simplexovĂ˝ch vertexov. NajhodnotnejĹˇia primitĂ­va nie je generĂˇtor geometrie, ale `argmax` plus top-two margin. OdporĂşÄŤanie je implementovaĹĄ ju ako malĂş internĂş integer primitĂ­vu; nevytvĂˇraĹĄ zatiaÄľ veÄľkĂ© verejnĂ© simplexovĂ© API.

