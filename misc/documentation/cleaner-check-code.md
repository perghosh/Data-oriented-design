
C++
*words that might show complexity*
```bash
# This script checks the code for the cleaner documentation.
cleaner count "*.cpp;*.h" -R --mode search --page -1 --page-size 20 --sort 2 --rpattern  "\bnew\b" --rpattern "\btemplate\b" --rpattern "\btypename\b" --rpattern "\bthread\b" --rpattern "\batomic\b" --rpattern "\breinterpret_cast\b" --rpattern "\bvolatile\b" --rpattern "\bgoto\b" --rpattern "\bfriend\b"
```

*performance*
```bash
cleaner count "*.cpp;*.h" -R --mode search --page -1 --page-size 20 --sort 2 --rpattern "\bmalloc\b" --rpattern "\bfree\b" --rpattern "\bmemcpy\b" --rpattern "\bpow\b" --rpattern "\bsqrt\b"
```


```bash

```