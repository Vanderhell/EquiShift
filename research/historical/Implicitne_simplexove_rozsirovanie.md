> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# ImplicitnĂ© simplexovĂ© rozĹˇirovanie pre embedded systĂ©my

## 1. CieÄľ

PreskĂşmaj a implementaÄŤne over deterministickĂ© pravidlo, ktorĂ© pri kaĹľdom zvĂ˝ĹˇenĂ­ rĂˇdu pridĂˇ **presne jeden novĂ˝ element**, priÄŤom sa geometrickĂˇ pozĂ­cia vĹˇetkĂ˝ch existujĂşcich elementov zmenĂ­ tak, aby zostali maximĂˇlne symetricky rozloĹľenĂ©.

CieÄľovĂˇ ĹˇtruktĂşra je **pravidelnĂ˝ simplex**:

| RĂˇd `n` | EfektĂ­vna dimenzia | PoÄŤet bodov |
|---:|---:|---:|
| 0 | 0D | 1 |
| 1 | 1D | 2 |
| 2 | 2D | 3 |
| 3 | 3D | 4 |
| 4 | 4D | 5 |
| `n` | `n`D | `n+1` |

PoÄŤet elementov teda rastie:

\[
N(n)=n+1=\Theta(n)
\]

Nie exponenciĂˇlne.

---

# 2. HlavnĂˇ myĹˇlienka

Neimplementuj simplex ako uloĹľenĂş maticu sĂşradnĂ­c.

HÄľadaj **implicitnĂş reprezentĂˇciu**, kde je bod definovanĂ˝ iba:

```c
n       // rĂˇd/simplex dimension
vertex  // index bodu 0..n
```

a jeho sĂşradnice sa zĂ­skajĂş deterministicky aĹľ vtedy, keÄŹ sĂş potrebnĂ©.

HlavnĂˇ otĂˇzka vĂ˝skumu:

> DokĂˇĹľeme reprezentovaĹĄ a pouĹľĂ­vaĹĄ rastĂşci pravidelnĂ˝ simplex bez potreby presĂşvaĹĄ alebo ukladaĹĄ vĹˇetky jeho body?

OdpoveÄŹ musĂ­ byĹĄ matematicky dokĂˇzanĂˇ a benchmarkovanĂˇ.

---

# 3. PrimĂˇrna reprezentĂˇcia, ktorĂş treba overiĹĄ

PouĹľi simplex s `n+1` bodmi v priestore s `n+1` sĂşradnicami.

Pre vertex:

\[
i \in \{0,\ldots,n\}
\]

a sĂşradnicu:

\[
j \in \{0,\ldots,n\}
\]

definuj:

\[
V(i,j,n)=
\begin{cases}
n & i=j\\
-1 & i\neq j
\end{cases}
\]

Teda naprĂ­klad pre `n=3`:

```text
V0 = [ 3, -1, -1, -1 ]
V1 = [-1,  3, -1, -1 ]
V2 = [-1, -1,  3, -1 ]
V3 = [-1, -1, -1,  3 ]
```

Tieto Ĺˇtyri body reprezentujĂş pravidelnĂ˝ tetraĂ©der.

Pri `n=4`:

```text
V0 = [ 4, -1, -1, -1, -1 ]
V1 = [-1,  4, -1, -1, -1 ]
V2 = [-1, -1,  4, -1, -1 ]
V3 = [-1, -1, -1,  4, -1 ]
V4 = [-1, -1, -1, -1,  4 ]
```

ExistujĂşce body teda skutoÄŤne **zmenia svoju pozĂ­ciu**, ale nemusĂ­me ich fyzicky aktualizovaĹĄ.

StaÄŤĂ­ zmeniĹĄ `n`.

Toto je kÄľĂşÄŤovĂˇ vlastnosĹĄ celĂ©ho experimentu.

---

# 4. PreÄŤo ide stĂˇle o n-rozmernĂ˝ simplex

Aj keÄŹ pouĹľĂ­vame `n+1` sĂşradnĂ­c, vĹˇetky body leĹľia v hyperrovine:

\[
x_0+x_1+\ldots+x_n=0
\]

pretoĹľe:

\[
n+n(-1)=0
\]

EfektĂ­vna dimenzia ĹˇtruktĂşry je teda iba:

\[
n
\]

Nie `n+1`.

NezamieĹaĹĄ **ambient dimension** (poÄŤet pouĹľitĂ˝ch sĂşradnĂ­c) s efektĂ­vnou dimenziou geometrie.

---

# 5. Symetria

Pre kaĹľdĂ˝ vertex musĂ­ platiĹĄ rovnakĂˇ norma:

\[
\|V_i\|^2
=
n^2+n
=
n(n+1)
\]

teda:

\[
\|V_i\|=\sqrt{n(n+1)}
\]

VĹˇetky body preto leĹľia na jednej hypersfĂ©re.

Pre dva rĂ´zne body:

\[
V_i\cdot V_j=-(n+1)
\]

a preto:

\[
\cos(\theta)
=
\frac{V_i\cdot V_j}
{\|V_i\|\|V_j\|}
=
-\frac{1}{n}
\]

KaĹľdĂˇ dvojica bodov teda zviera rovnakĂ˝ uhol.

Takisto musĂ­ byĹĄ rovnakĂˇ vzdialenosĹĄ medzi ÄľubovoÄľnĂ˝mi dvoma rĂ´znymi bodmi:

\[
\|V_i-V_j\|
=
\sqrt{2}(n+1)
\]

pre nenormalizovanĂş celoÄŤĂ­selnĂş reprezentĂˇciu.

Tieto vlastnosti musia byĹĄ sĂşÄŤasĹĄou automatickĂ˝ch testov.

---

# 6. Embedded vĂ˝hoda

NajzaujĂ­mavejĹˇia vlastnosĹĄ je:

```c
coordinate(n, vertex, axis)
```

mĂ´Ĺľe byĹĄ prakticky:

```c
return vertex == axis ? n : -1;
```

Teda:

- bez heapu,
- bez tabuÄľky,
- bez matice,
- bez trigonometrie,
- bez `sqrt()` pri beĹľnom pouĹľĂ­vanĂ­,
- bez floating point,
- bez prepoÄŤĂ­tavania starĂ˝ch vertexov,
- bez inicializaÄŤnej fĂˇzy.

VĂ˝poÄŤet jednej sĂşradnice:

\[
O(1)
\]

PamĂ¤ĹĄ potrebnĂˇ na reprezentĂˇciu konkrĂ©tneho vertexu:

\[
O(1)
\]

naprĂ­klad iba:

```c
struct simplex_vertex
{
    uint16_t n;
    uint16_t index;
};
```

---

# 7. DĂ´leĹľitĂ© rozlĂ­Ĺˇenie komplexĂ­t

NesnaĹľ sa umelo tvrdiĹĄ, Ĺľe vĹˇetko je `O(1)`.

Presne rozliĹˇuj:

### PoÄŤet vertexov

\[
O(n)
\]

### ReprezentĂˇcia jednĂ©ho vertexu

PotenciĂˇlne:

\[
O(1)
\]

### VĂ˝poÄŤet jednej sĂşradnice

PotenciĂˇlne:

\[
O(1)
\]

### MaterializĂˇcia celĂ©ho vertexu

Vertex mĂˇ `n+1` sĂşradnĂ­c:

\[
O(n)
\]

### EnumerĂˇcia vĹˇetkĂ˝ch vertexov

Je ich `n+1`:

\[
O(n)
\]

ak nepotrebujeme materializovaĹĄ ich sĂşradnice.

### MaterializĂˇcia vĹˇetkĂ˝ch sĂşradnĂ­c vĹˇetkĂ˝ch vertexov

\[
(n+1)^2
\]

teda:

\[
O(n^2)
\]

Toto sa nedĂˇ oznaÄŤiĹĄ za `O(n)` iba preto, Ĺľe poÄŤet vertexov rastie lineĂˇrne.

---

# 8. Expanzia

PreskĂşmaj najmĂ¤ operĂˇciu:

```text
n â†’ n+1
```

Pri implicitnej reprezentĂˇcii by nemalo byĹĄ potrebnĂ© iterovaĹĄ cez starĂ© vertexy.

NaprĂ­klad:

```text
n = 3

V0 = [ 3 -1 -1 -1 ]
V1 = [-1  3 -1 -1 ]
V2 = [-1 -1  3 -1 ]
V3 = [-1 -1 -1  3 ]
```

po expanzii:

```text
n = 4

V0 = [ 4 -1 -1 -1 -1 ]
V1 = [-1  4 -1 -1 -1 ]
V2 = [-1 -1  4 -1 -1 ]
V3 = [-1 -1 -1  4 -1 ]
V4 = [-1 -1 -1 -1  4 ]
```

Geometricky:

- pribudol novĂ˝ vertex,
- pribudla novĂˇ dimenzia,
- vĹˇetky starĂ© vertexy zmenili polohu,
- stĂˇle zostala ĂşplnĂˇ symetria.

Ale reprezentĂˇcia mohla vykonaĹĄ iba:

```c
n++;
```

Toto treba explicitne dokĂˇzaĹĄ.

---

# 9. NormalizovanĂˇ verzia

PreskĂşmaj aj variant, kde majĂş vĹˇetky vertexy polomer:

\[
R=1
\]

Potom:

\[
\hat V_i =
\frac{V_i}
{\sqrt{n(n+1)}}
\]

NormalizĂˇcia vĹˇak nesmie byĹĄ automaticky sĂşÄŤasĹĄou embedded API.

Ak aplikĂˇcia nepotrebuje fyzickĂ© geometrickĂ© sĂşradnice, celoÄŤĂ­selnĂˇ reprezentĂˇcia je pravdepodobne vĂ˝razne vĂ˝hodnejĹˇia.

VyhodnoĹĄ:

- integer representation,
- fixed-point representation,
- floating-point normalized representation.

---

# 10. AlternatĂ­vna minimĂˇlna nD reprezentĂˇcia

Ako druhĂş vetvu vĂ˝skumu implementuj alebo matematicky analyzuj skutoÄŤnĂş reprezentĂˇciu v `R^n`.

RekurzĂ­vne moĹľno z `(n-1)`-simplexu vytvoriĹĄ `n`-simplex.

Pre starĂ© vertexy:

\[
v_i^{(n)}
=
\left(
a_n v_i^{(n-1)},
-\frac1n
\right)
\]

kde:

\[
a_n=
\sqrt{1-\frac1{n^2}}
=
\frac{\sqrt{n^2-1}}{n}
\]

a novĂ˝ vertex:

\[
v_n^{(n)}
=
(0,\ldots,0,1)
\]

TĂˇto reprezentĂˇcia pouĹľĂ­va presne `n` sĂşradnĂ­c, ale pravdepodobne je pre malĂ© MCU drahĹˇia kvĂ´li:

- nĂˇsobeniu,
- deleniu,
- odmocninĂˇm,
- floating point/fixed point,
- zloĹľitejĹˇej materializĂˇcii.

Porovnaj ju s implicitnou celoÄŤĂ­selnou `R^(n+1)` reprezentĂˇciou.

---

# 11. API experiment

Nenavrhuj veÄľkĂş kniĹľnicu.

Najprv vytvor minimĂˇlne experimentĂˇlne API.

NaprĂ­klad:

```c
typedef uint16_t sx_dim_t;
typedef uint16_t sx_vertex_t;
typedef int32_t  sx_coord_t;
```

MoĹľnĂ© primitĂ­va:

```c
sx_coord_t sx_coord(
    sx_dim_t n,
    sx_vertex_t vertex,
    sx_dim_t axis);
```

```c
bool sx_valid_vertex(
    sx_dim_t n,
    sx_vertex_t vertex);
```

```c
int64_t sx_dot(
    sx_dim_t n,
    sx_vertex_t a,
    sx_vertex_t b);
```

```c
uint64_t sx_norm2(
    sx_dim_t n);
```

```c
uint64_t sx_distance2(
    sx_dim_t n,
    sx_vertex_t a,
    sx_vertex_t b);
```

NevytvĂˇraj API iba preto, aby existovalo.

KaĹľdĂˇ funkcia musĂ­ maĹĄ konkrĂ©tny embedded use-case.

---

# 12. SkĂşs odvodiĹĄ operĂˇcie bez sĂşradnĂ­c

Toto je veÄľmi dĂ´leĹľitĂˇ ÄŤasĹĄ vĂ˝skumu.

PokiaÄľ sa dĂˇ vĂ˝sledok vypoÄŤĂ­taĹĄ priamo z:

```text
n
vertexA
vertexB
```

nesmie sa materializovaĹĄ vektor.

NaprĂ­klad norma:

\[
\|V_i\|^2=n(n+1)
\]

nevyĹľaduje prechod cez sĂşradnice.

Dot product:

\[
V_i\cdot V_j=
\begin{cases}
n(n+1) & i=j\\
-(n+1) & i\neq j
\end{cases}
\]

Takisto nemusĂ­ byĹĄ `O(n)`.

HÄľadaj podobnĂ© uzavretĂ© vzorce pre:

- normu,
- vzdialenosĹĄ,
- dot product,
- cosine similarity,
- nearest vertex,
- klasifikĂˇciu,
- projekciu,
- vĂ˝ber vertexu,
- interpolĂˇciu/barycentrickĂ© vĂˇhy,
- transformĂˇciu medzi rĂˇdmi.

CieÄľom je robiĹĄ **simplexovĂ© operĂˇcie bez existencie simplexovej matice**.

---

# 13. NajdĂ´leĹľitejĹˇia embedded otĂˇzka

SamotnĂ˝ pravidelnĂ˝ simplex nie je novĂ˝ matematickĂ˝ objav.

NevydĂˇvaj ho za novĂ˝ algoritmus.

VĂ˝skum mĂˇ odpovedaĹĄ na inĂş otĂˇzku:

> Existuje prakticky pouĹľiteÄľnĂˇ embedded primitĂ­va zaloĹľenĂˇ na implicitnom simplexe, ktorĂˇ odstrĂˇni reĂˇlnu bolesĹĄ embedded programĂˇtora?

Bez takej aplikĂˇcie projekt nemĂˇ vĂ˝znam.

---

# 14. KandidĂˇtne pouĹľitia

PreskĂşmaj minimĂˇlne tieto oblasti.

### 14.1 DeterministickĂ© rovnomernĂ© rozdelenie N stavov

Ak potrebujeme `N` stavov bez preferovanĂ©ho smeru/stavu:

\[
n=N-1
\]

Simplex poskytuje maximĂˇlne symetrickĂ© rozloĹľenie.

---

### 14.2 N-way rozhodovanie

PreskĂşmaj, ÄŤi simplex mĂ´Ĺľe reprezentovaĹĄ:

- N kandidĂˇtov,
- N senzorov,
- N vstupov,
- N scheduler stavov,

bez biasu (systematickĂ©ho zvĂ˝hodnenia) niektorĂ©ho z nich.

---

### 14.3 KlasifikĂˇcia

PreskĂşmaj reprezentĂˇciu `N` tried simplexovĂ˝mi vertexmi namiesto klasickĂ©ho one-hot:

```text
[1,0,0,0]
[0,1,0,0]
...
```

SimplexovĂˇ reprezentĂˇcia mĂˇ nulovĂ˝ centroid a presne definovanĂş vzĂˇjomnĂş korelĂˇciu.

Zisti, ÄŤi z toho existuje praktickĂ˝ prĂ­nos na MCU.

---

### 14.4 DeterministickĂ© probe directions

PreskĂşmaj pouĹľitie ako malĂ©ho poÄŤtu maximĂˇlne oddelenĂ˝ch smerov pre:

- kalibrĂˇciu,
- optimalizaÄŤnĂ© kroky,
- sampling (vzorkovanie),
- senzorovĂ© sondovanie,
- testovanie priestoru parametrov.

---

### 14.5 Fault / state separation

PreskĂşmaj, ÄŤi rovnakĂˇ vzdialenosĹĄ medzi stavmi mĂ´Ĺľe pomĂ´cĹĄ pri:

- robustnom rozliĹˇovanĂ­ stavov,
- detekcii chĂ˝b,
- jednoduchĂ˝ch rozhodovacĂ­ch schĂ©mach.

NevytvĂˇraj vĹˇak umelĂ© use-cases. Ak nie je merateÄľnĂ˝ prĂ­nos, oznaÄŤ kandidĂˇta ako nepouĹľiteÄľnĂ˝.

---

# 15. Embedded obmedzenia

ImplementĂˇcia musĂ­ byĹĄ navrhnutĂˇ minimĂˇlne pre:

- AVR,
- Cortex-M0/M0+,
- Cortex-M3/M4,
- ESP32,
- STM32.

Priorita:

1. Ĺľiadny heap,
2. Ĺľiadny skrytĂ˝ globĂˇlny stav,
3. deterministickĂ˝ runtime,
4. celoÄŤĂ­selnĂˇ aritmetika, pokiaÄľ je moĹľnĂˇ,
5. Ĺľiadna potreba FPU,
6. malĂ© API,
7. Ĺľiadne veÄľkĂ© lookup tabuÄľky,
8. jasnĂ© overflow limity.

---

# 16. Overflow analĂ˝za

Povinne odvodiĹĄ maximĂˇlne bezpeÄŤnĂ© `n` pre:

```text
int8_t
int16_t
int32_t
int64_t
```

SamotnĂˇ sĂşradnica potrebuje uloĹľiĹĄ:

```text
n
-1
```

ale odvodenĂ© vĂ˝poÄŤty obsahujĂş naprĂ­klad:

\[
n(n+1)
\]

a:

\[
(n+1)^2
\]

Pre kaĹľdĂş operĂˇciu urÄŤ:

- maximĂˇlny podporovanĂ˝ `n`,
- dĂˇtovĂ˝ typ medzivĂ˝sledku,
- ÄŤi je potrebnĂˇ saturĂˇcia,
- ÄŤi sa dĂˇ overflow predvĂ­daĹĄ eĹˇte pred operĂˇciou.

Ĺ˝iadne undefined behavior.

---

# 17. Testovanie

MinimĂˇlne:

## Invariant 1 â€” poÄŤet vertexov

```text
vertex_count(n) == n + 1
```

## Invariant 2 â€” centroid

Pre kaĹľdĂ˝ `n`:

\[
\sum_i V_i = 0
\]

## Invariant 3 â€” norma

Pre kaĹľdĂ˝ vertex:

\[
\|V_i\|^2=n(n+1)
\]

## Invariant 4 â€” dot product

Pre:

\[
i\neq j
\]

musĂ­ platiĹĄ:

\[
V_i\cdot V_j=-(n+1)
\]

## Invariant 5 â€” vzdialenosĹĄ

VĹˇetky rozdielne dvojice musia maĹĄ rovnakĂş vzdialenosĹĄ.

## Invariant 6 â€” rank

Pre `n+1` vertexov musĂ­ byĹĄ geometrickĂ˝ rank:

\[
n
\]

## Invariant 7 â€” deterministickosĹĄ

RovnakĂ©:

```text
n
vertex
axis
```

musĂ­ vĹľdy produkovaĹĄ presne rovnakĂ˝ vĂ˝sledok.

---

# 18. ExhaustĂ­vne testy

Testuj minimĂˇlne:

```text
n = 0 .. 255
```

Ak dĂˇtovĂ© typy dovolia, rozĹˇĂ­r:

```text
n = 0 .. 65535
```

GeometrickĂ© invarianty nemusĂ­Ĺˇ vĹľdy kontrolovaĹĄ explicitnou `O(nÂ˛)` materializĂˇciou.

PouĹľi aj odvodenĂ© closed-form (uzavretĂ©) rovnice.

---

# 19. Benchmark

Porovnaj:

### A
Explicitne uloĹľenĂˇ simplexovĂˇ matica.

### B
RekurzĂ­vne generovanĂ˝ simplex v `R^n`.

### C
ImplicitnĂ˝ integer simplex v `R^(n+1)`.

Meraj:

- RAM,
- flash/code size,
- inicializĂˇciu,
- vĂ˝poÄŤet jednej sĂşradnice,
- vĂ˝poÄŤet vertexu,
- dot product,
- distance,
- iterĂˇciu vertexov,
- zmenu `n â†’ n+1`.

NajdĂ´leĹľitejĹˇĂ­ benchmark:

```text
expand simplex
```

Pri implicitnej reprezentĂˇcii oÄŤakĂˇvame operĂˇciu blĂ­zku:

\[
O(1)
\]

pretoĹľe sa fyzicky nepresĂşva Ĺľiadny vertex.

MusĂ­ sa to vĹˇak overiĹĄ podÄľa presne definovanĂ©ho API.

---

# 20. KritĂ©riĂˇ Ăşspechu

Projekt je zaujĂ­mavĂ˝ iba vtedy, ak sa potvrdĂ­ aspoĹ jedna vĂ˝znamnĂˇ vĂ˝hoda.

NaprĂ­klad:

- `O(1)` reprezentĂˇcia vertexu,
- `O(1)` zĂ­skanie ÄľubovoÄľnej sĂşradnice,
- `O(1)` expanzia rĂˇdu,
- `O(1)` dot product medzi implicitnĂ˝mi vertexmi,
- nulovĂˇ dynamickĂˇ alokĂˇcia,
- vĂ˝razne menĹˇia RAM neĹľ explicitnĂˇ reprezentĂˇcia,
- praktickĂ˝ embedded algoritmus, ktorĂ˝ vÄŹaka tejto reprezentĂˇcii zjednoduĹˇĂ­ existujĂşci problĂ©m.

---

# 21. KritĂ©riĂˇ zamietnutia

Projekt oznaÄŤ ako nevhodnĂ˝, ak vĂ˝sledkom bude iba:

> â€žDokĂˇĹľeme na MCU generovaĹĄ pravidelnĂ˝ simplex.â€ś

To samo osebe nestaÄŤĂ­.

Rovnako projekt zamietni, ak:

- neexistuje praktickĂ˝ use-case,
- explicitnĂˇ alternatĂ­va je jednoduchĹˇia a rovnako lacnĂˇ,
- Ăşspora pamĂ¤te nemĂˇ praktickĂ˝ vĂ˝znam,
- simplex slĂşĹľi iba ako matematickĂˇ kuriozita,
- API by bolo vĂ¤ÄŤĹˇie neĹľ problĂ©m, ktorĂ˝ rieĹˇi.

---

# 22. PoĹľadovanĂ˝ vĂ˝sledok vĂ˝skumu

Na konci neposkytni iba zdrojovĂ˝ kĂłd.

Vypracuj rozhodnutie:

```text
KEEP
KEEP AS INTERNAL PRIMITIVE
RESEARCH FURTHER
REJECT
```

a podloĹľ ho meraniami.

VĂ˝stup musĂ­ obsahovaĹĄ:

1. matematickĂ˝ dĂ´kaz,
2. odvodenie implicitnej reprezentĂˇcie,
3. complexity analĂ˝zu,
4. overflow analĂ˝zu,
5. referenÄŤnĂş C implementĂˇciu,
6. exhaustive testy,
7. benchmarky,
8. embedded use-case analĂ˝zu,
9. porovnanie s beĹľnĂ˝mi alternatĂ­vami,
10. koneÄŤnĂ˝ verdikt.

---

# 23. ZĂˇsadnĂˇ myĹˇlienka, ktorĂş nesmie implementĂˇcia stratiĹĄ

Nejde o to, Ĺľe mĂˇme `n+1` bodov.

Ide o toto:

> **ZvĂ˝ĹˇenĂ­m jedinĂ©ho parametra `n` dokĂˇĹľeme implicitne zmeniĹĄ polohu vĹˇetkĂ˝ch existujĂşcich elementov, pridaĹĄ novĂ˝ element a zachovaĹĄ dokonalĂş vzĂˇjomnĂş symetriu â€” bez potreby uloĹľiĹĄ alebo fyzicky prepoÄŤĂ­taĹĄ celĂş ĹˇtruktĂşru.**

PrĂˇve tĂşto vlastnosĹĄ treba skĂşmaĹĄ ako potenciĂˇlnu embedded primitĂ­vu.
