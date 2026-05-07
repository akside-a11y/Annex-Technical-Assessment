#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * pthFactor
 *
 * Strategy
 * ---------
 * n can be as large as 10^15, so we cannot trial-divide all the way to n.
 * However, every factor pair (d, n/d) satisfies d <= sqrt(n), so we only
 * need to iterate up to sqrt(n) ~ 3.16 * 10^7 — perfectly feasible.
 *
 * We collect the "small" factors (d) into one array and the corresponding
 * "large" factors (n/d) into another (only when d != n/d to avoid duplicates
 * for perfect squares).  After the loop:
 *
 *   small[] is already sorted ascending   [1, 2, 3, ...]
 *   large[] holds the mirrors in reverse  [..., n/3, n/2, n/1]
 *
 * Reversing large[] and appending it to small[] gives the full sorted factor
 * list without ever needing to sort.
 *
 * Time  : O(sqrt(n))
 * Space : O(sqrt(n))  — at most ~63 245 factors for n <= 10^15 in practice
 */

long pthFactor(long n, long p) {
    /* Maximum number of small factors is sqrt(n) ~ 3.16e7, but the actual
       count of divisors of any number <= 10^15 is at most a few thousand.
       Allocate generously; we will only use what we need.                  */
    long small_cap = 100000;
    long *small = (long *)malloc(small_cap * sizeof(long));
    long *large = (long *)malloc(small_cap * sizeof(long));
    if (!small || !large) {
        free(small);
        free(large);
        return 0;
    }

    long s_count = 0, l_count = 0;
    long sq = (long)sqrt((double)n);

    /* Adjust for floating-point rounding */
    while ((sq + 1) * (sq + 1) <= n) sq++;
    while (sq * sq > n) sq--;

    for (long d = 1; d <= sq; d++) {
        if (n % d == 0) {
            small[s_count++] = d;
            if (d != n / d) {
                large[l_count++] = n / d;
            }
        }
    }

    /*
     * Full sorted factor list (ascending):
     *   small[0], small[1], ..., small[s_count-1],
     *   large[l_count-1], large[l_count-2], ..., large[0]
     *
     * Total factors = s_count + l_count.
     * We answer using index arithmetic without building the merged array.
     */
    long total = s_count + l_count;
    long result = 0;

    if (p <= total) {
        if (p <= s_count) {
            result = small[p - 1];                    /* 1-based index */
        } else {
            result = large[l_count - (p - s_count)];  /* mirror part   */
        }
    }
    /* else: fewer than p factors → return 0 (already 0) */

    free(small);
    free(large);
    return result;
}

/* ── Driver (reads from stdin) ─────────────────────────────────────────── */
int main(void) {
    long n, p;
    printf("Enter n and p: ");
    if (scanf("%ld %ld", &n, &p) != 2) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }
    printf("%ld\n", pthFactor(n, p));
    return 0;
}
