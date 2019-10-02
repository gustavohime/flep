# FLEP - Fast Lite Expression Parser
 Copyright (C) 2019 Gustavo Hime

FLEP compiles a parenthesized mathematical expression stored in a string and 
then uses the compiled representation to evaluate it several times.
Expressions can contain:
 - up to seven variables, designated by the letters "abcxyzw"
 - standard four operations `+`, `-`, `/`, `*`, plus `^` for power
 - balanced parentheses
 - functions `sin`, `cos`, `tan`, `log`, `exp` and `sqrt`.
 - constants `e` and `pi`.

FLEP uses a recursive parser to generate a RPN representation, then
runs it on a stack-based engine. FLEP's code is:
 - Just plain old ANSI C.
 - Uses plain old standard C libraries only.
 - Is less than 600 LOC, less than 400 LOC if comments and blanks are removed.
 - Compiles into an object with less than 12K with GCC.
 - Is threadsafe.

There are several other C/C++ libraries you might want to check out.
The 
[C++ Mathematical Expression Parser Benchmark](https://github.com/ArashPartow/math-parser-benchmark-project) provides a quite thorough comparison, and
depending on your priorities and constraints there might be more suitable
alternatives. At the time of this writing, FLEP passes the benchmark suite
with flying colors for correctness and speed, given its minimalistic 
approach: other packages provide faster evaluation at the cost of greatly
increased complexity.

FLEP provides you with a GPLv3 absolute minimal implementation in C. Integrating it into your project should take no time, it should cause no conflicts and be as bug free as can be. I wrote it for fun and exercise: have fun using it without a care and do report any bugs you find, so I can bite my tongue and fix them.


## How to use

Here is the ABSOLUTE MINIMAL example of how to use it.

```C
  const char* exp = "a + b^(c/pi)" // expression to compile
  const struct FLEP* f = flep_parse(exp); // compile, returns opaque structure
  double abc[3] = {1.0, 2.0, 3.0}; // a = 1, b = 2, c = 3
  double x = flep_eval(f, abc); // computes 'exp' for the values in 'abc'
  flep_free(f); // release memory allocated by flep_parse
```

The preceding example does not account for parsing errors. 
Malformed expressions may fail to parse, or there may be as-of-yet unknown
bugs. The following example is the PROPER USAGE, including error checking.

```C
  const char* exp = "a + b^(c/pi)"
  int error, int position;
  const struct FLEP* f = flep_parse(exp, &error, &position);
  if (!f) { // failed to parse
    printf("Failed to parse at position %d (%s)\n",
      position, flep_translate(error));
    exit(1); // deal with failure somehow
  }
  double abc[3] = {1.0, 2.0, 3.0};
  double x = flep_eval(f, abc);
  flep_free(f);
```

## Compiling and running the example

The compilation is rather trivial, you need `gcc` and `make`. Just run `make`.
You can then run `example` without any arguments for a simplistic benchmark.
Alternatively, you can run `example [input.file]` to parse a list of expressions
of your choice. A sample input file is provided - containing the same expressions hardcoded in [example.c](https://github.com/gustavohime/flep/blob/master/example.c).
