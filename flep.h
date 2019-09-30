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

/*
 * Compile a parenthesized mathematical expression stored in a string and 
 * then used the compiled representation to evaluate it several times.
 * Expressions can contain:
 * - up to seven variables, designated by the letters "abcxyzw"
 * - standard operations +, -, /, * plus ^ for power
 * - balanced parentheses
 * - functions "sin", "cos", "tan", "log", "exp" and "sqrt"
 * - constants "e" and "pi"
*/
#ifndef FLEP_H
#define FLEP_H
#ifdef __cplusplus
extern "C" {
#endif

struct FLEP; /* Opaque to user - holds compiled expression */

const struct FLEP* flep_parse(const char* s, int* error, 
  int* position);
/* Parse string "exp".
 * If successful, return pointer to dynamically allocated struct FLEP*.
 * If not successful, return NULL.
 * Either or both "position" and "error" may be NULL. 
 * If non-null and success, the pointed values are unchanged.
 * If non-null and fail, then:
 *   "*position" contains the offset of "exp" where error occurred.
 *   "*error" constainns following: */
#define FLEP_OK             0 /* (== 0) if success */
#define FLEP_BADSYNTAX     20 /* bad expression syntax */
#define FLEP_BADTOKEN      21 /* bad token */
#define FLEP_EXPECTED_OPEN 22 /* expected "(" (e.g. after "sin") */
#define FLEP_UNBALANCED    23 /* unbalanced parentheses */

double flep_eval(const struct FLEP* f, double* val);
/* Evaluate expression pointed to by "f" using arguments pointed to by "val"
 * - "f" was compiled with "flep_parse"
 * - "val" is an array of up to 7 values, the positions of which correspond
 *   to the variable names "abcxyzw", i.e. val[0] is "a", val[1] is "b", etc
 */

/* Deallocate memory of "f" previously returned by "flep_parse" */
void flep_free(const struct FLEP* f);

/* Return string value for code "c" previously returned in "error" 
 * after a failed call to "flep_parse".
 */
const char* flep_translate(int c);

/* For debugging and curiosity satisfaction: dump opcodes to stdout */
void flep_dump(const struct FLEP* f);
#ifdef __cplusplus
}
#endif

#endif

