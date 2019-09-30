/*
 * FLEP - Fast Lite Expression Parser
 * Copyright (C) 2019 Gustavo Hime
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 */

/* Test program. See README.txt for information on compiling and running.
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "flep.h"

#define N_BUILT_IN 27
const char* built_in[] = {
  "sin(2.2 * a) + cos(pi / b)",
  "1 - sin(2.2 * a) + cos(pi / b)",
  "sqrt(3 + sin(2.2 * a) + cos(pi / b) / 3.3)",
  "(a^2 / sin(2 * pi / b)) -a / 2.2",
  "1-(a/b*0.5)",
  "e^log(7*a)",
  "10^log(3+b)",
  "(cos(2.41)/b)",
  "-(sin(pi+a)+1)",
  "a-(e^(log(7+b)))",
  "(1.123*sin(a)+2.1234)/3.1237",
  "(1.123*cos(a)-3.1235)/3.1238",
  "(1.123*tan(a)+2.1236)/3.1239",
  "(b+a/b) * (a-b/a)",
  "a/((a+b)*(a-b))/b",
  "1.1-((a*b)+(a/b))-3.3",
  "a+b",
  "(a+b)*3.3",
  "(2*a+2*a)",
  "2*(2*a)",
  "(2*a)*2",
  "-(b^1.1)",
  "a+b*(a+b)",
  "(1.1+b)*(-3.3)",
  "a+b-e*pi/5^6",
  "a^b/e*pi-5+6",
  "2.2*(a+b)",
};
void time_wrapper(int *sec, int *usec) {
  struct timeval t;
  int ok = gettimeofday(&t, 0);
  if (ok) {
    printf("gettimeofday returned non-null (%d), aborting.\n", ok);
    exit(1);
  }
  *sec = t.tv_sec;
  *usec = t.tv_usec;
}

double keep;
volatile double *pkeep = &keep;

#define N_FOR_BENCH 1000000
#define N_DISCARD 10

#define NATIVE(i, s) \
double native_eval##i(double* ab) { \
  return s; \
} \
double native_bench##i(volatile double* ab) { \
  int s1, u1, s2, u2; \
  time_wrapper(&s1, &u1); \
  for (int j = 0; j < N_FOR_BENCH; j++) { \
    *pkeep += s; \
  } \
  time_wrapper(&s2, &u2); \
  return u2 - u1 + 1e6 * (s2 - s1); \
}

NATIVE(00,sin(2.2 * ab[0]) + cos(M_PI / ab[1]))
NATIVE(01,1 - sin(2.2 * ab[0]) + cos(M_PI / ab[1]))
NATIVE(02,sqrt(3 + sin(2.2 * ab[0]) + cos(M_PI / ab[1]) / 3.3))
NATIVE(03,(pow(ab[0],2) / sin(2 * M_PI / ab[1])) -ab[0] / 2.2)
NATIVE(04,1-(ab[0]/ab[1]*0.5))
NATIVE(05,exp(log(7*ab[0])))
NATIVE(06,pow(10,log(3+ab[1])))
NATIVE(07,(cos(2.41)/ab[1]))
NATIVE(08,-(sin(M_PI+ab[0])+1))
NATIVE(09,ab[0]-(exp(log(7+ab[1]))))
NATIVE(10,(1.123*sin(ab[0])+2.1234)/3.1237)
NATIVE(11,(1.123*cos(ab[0])-3.1235)/3.1238)
NATIVE(12,(1.123*tan(ab[0])+2.1236)/3.1239)
NATIVE(13,(ab[1]+ab[0]/ab[1]) * (ab[0]-ab[1]/ab[0]))
NATIVE(14,ab[0]/((ab[0]+ab[1])*(ab[0]-ab[1]))/ab[1])
NATIVE(15,1.1-((ab[0]*ab[1])+(ab[0]/ab[1]))-3.3)
NATIVE(16,ab[0]+ab[1])
NATIVE(17,(ab[0]+ab[1])*3.3)
NATIVE(18,(2*ab[0]+2*ab[0]))
NATIVE(19,2*(2*ab[0]))
NATIVE(20,(2*ab[0])*2)
NATIVE(21,-pow(ab[1],1.1))
NATIVE(22,ab[0]+ab[1]*(ab[0]+ab[1]))
NATIVE(23,(1.1+ab[1])*(-3.3))
NATIVE(24,ab[0]+ab[1]-exp(1)*M_PI/pow(5,6))
NATIVE(25,pow(ab[0],ab[1])/exp(1)*M_PI-5+6)
NATIVE(26,2.2*(ab[0]+ab[1]))

double (*native_eval[N_BUILT_IN])(double *)= {
  &native_eval00, &native_eval01, &native_eval02, &native_eval03, 
  &native_eval04, &native_eval05, &native_eval06, &native_eval07, 
  &native_eval08, &native_eval09, &native_eval10, &native_eval11, 
  &native_eval12, &native_eval13, &native_eval14, &native_eval15, 
  &native_eval16, &native_eval17, &native_eval18, &native_eval19,
  &native_eval20, &native_eval21, &native_eval22, &native_eval23, 
  &native_eval24, &native_eval25, &native_eval26
};

double (*native_bench[N_BUILT_IN])(volatile double*)= {
  &native_bench00, &native_bench01, &native_bench02, &native_bench03, 
  &native_bench04, &native_bench05, &native_bench06, &native_bench07, 
  &native_bench08, &native_bench09, &native_bench10, &native_bench11, 
  &native_bench12, &native_bench13, &native_bench14, &native_bench15, 
  &native_bench16, &native_bench17, &native_bench18, &native_bench19,
  &native_bench20, &native_bench21, &native_bench22, &native_bench23, 
  &native_bench24, &native_bench25, &native_bench26
};

double compare(const struct FLEP* flep, double (*nat)(double*)) {
  double a, b, x, y, relerr = 0;
  int n = 0;
  for (a = 0.1; a <= 3.0; a += 0.2)
  for (b = 0.2; b <= 3.0; b += 0.2) {
    double ab[2] = {a, b};
    n++;
    x = flep_eval(flep, ab);
    y = nat(ab);
    relerr += fabs(y ? (x-y)/y : 0);
  }
  return relerr / n;
}

double benchmark(const struct FLEP* flep, double (*nat)(volatile double*)) {
  int s1, u1, s2, u2;
  double ab[2] = {1.1, 2.2}, time_flep, time_nat;
  volatile double *pab = ab;
  for (int i = 0; i < N_DISCARD; i++) {
    *pkeep += flep_eval(flep, (double*)pab);
    {double x = ab[0]; ab[0] = ab[1]; ab[1] = x;}
  }
  time_wrapper(&s1, &u1);
  for (int i = 0; i < N_FOR_BENCH; i++) {
    *pkeep += flep_eval(flep, ab);
    {double x = ab[0]; ab[0] = ab[1]; ab[1] = x;}
  }
  time_wrapper(&s2, &u2);
  time_flep = (double)(u2 - u1) + 1e6 * (double)(s2 - s1);
  time_nat = nat(pab);
  return time_flep / time_nat;
}

int main(int argc, const char* argv[]) {
  FILE* infile = 0;
  if (argc > 1) {
    infile = fopen(argv[1], "rb");
    if (!infile) {
      printf("Failed to open input file \"%s\"\n", argv[1]);
      return 1;
    }
  }
  printf("FLEP - Fast Light Expression Parser\n\n");
  if (infile) {
    printf("Reading expressions from \"%s\" (compile only).\n", argv[1]);
  } else {
    printf(
"Using built-in test expressions (compile and run).\n"
"Expressions will be evaluated %d times in benchmark\n\n",
      N_FOR_BENCH);
  }

  int i = 0, bad = 0, total = 0;
  const char* exp = 0;
#define BUFLEN 512
  char buf[BUFLEN];
  if (!infile) {
    printf(
  "Column A: relative error of FLEP to native implementation in %%\n"
  "Column B: relative time of FLEP to native implementation (ratio)\n"
  "Column C: test expression\n\n"
  " %3s%3s | %3s%2s | %10s\n"
  " %6s | %5s |\n", 
  "A", "", "B", "", "C",
  "", "");
  }
  for (;;i++) {
    if (infile) {
      while (!feof(infile)) {
	exp = fgets(buf, BUFLEN, infile);
	if (exp) {
	  while (isspace(*exp)) exp++;
	  if (*exp == '#') continue;
	  int n = strlen(exp);
	  if (!n) continue;
	  if (exp[n-1] == '\n') buf[n-1] = 0;
	  total++;
	  break;
	}
      }
      if (!exp) {
	break;
      }
    } else {
      if (i == N_BUILT_IN) return 0;
      exp = built_in[i];
    }
    int error, position;
    const struct FLEP* flep;
    flep = flep_parse(exp, &error, &position);
    if (!flep) {
      printf("FLEP failed to parse (%s)\n%s\n%*s\n", 
	flep_translate(error), exp, position, "^");
      bad++;
      continue;
    }
    if (!infile) {
      double percent_off = compare(flep, native_eval[i]);
      printf(" %5.2f%% |", percent_off);
      double ratio= benchmark(flep, native_bench[i]);
      printf(" %5.2f |", ratio);
      printf(" %-s\n", exp);
    } else {
      printf("\"%s\"\n", exp);
      // Uncomment the line below to see the RPN representation
      //flep_dump(flep);
    }
    flep_free(flep);
  }
  if (infile) {
    printf("Successfully parsed %d of %d expressions from \"%s\"\n",
      total -bad, total, argv[1]);
  }
}

