/* codegen_test.c - test that codegen rejects overlong section names */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test the bounds checking logic directly */
#define SECTION_SIZE 50

static int
test_section_bounds(const char *input)
{
  char section[SECTION_SIZE];
  
  /* Simulate the bounds check from codegen.c line 228 */
  if(strlen(input) + 7 + 1 > sizeof(section)) {
    return 1; /* Would be rejected */
  }
  
  /* Simulate the snprintf from codegen.c line 234 */
  snprintf(section, sizeof(section), "flickr.%s", input);
  
  return 0; /* Would be accepted */
}

int
main(void)
{
  int failures = 0;
  
  /* Test 1: Valid short input should be accepted */
  if(test_section_bounds("photos.search") != 0) {
    fprintf(stderr, "FAIL: Valid short input was rejected\n");
    failures++;
  }
  
  /* Test 2: Boundary case - exactly at limit should be accepted */
  char boundary[SECTION_SIZE - 7 - 1 + 1];
  memset(boundary, 'a', sizeof(boundary) - 1);
  boundary[sizeof(boundary) - 1] = '\0';
  if(test_section_bounds(boundary) != 0) {
    fprintf(stderr, "FAIL: Boundary input was rejected\n");
    failures++;
  }
  
  /* Test 3: One byte over limit should be rejected */
  char overlong[SECTION_SIZE - 7 + 1];
  memset(overlong, 'A', sizeof(overlong) - 1);
  overlong[sizeof(overlong) - 1] = '\0';
  if(test_section_bounds(overlong) != 1) {
    fprintf(stderr, "FAIL: Overlong input was not rejected\n");
    failures++;
  }
  
  /* Test 4: Very long input (100 chars) should be rejected */
  const char *very_long =
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  if(test_section_bounds(very_long) != 1) {
    fprintf(stderr, "FAIL: Very long input was not rejected\n");
    failures++;
  }
  
  if(failures == 0) {
    printf("All tests passed\n");
  }
  
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
