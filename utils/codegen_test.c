/* codegen_test.c - test that codegen rejects overlong section names */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static int
run_codegen(const char *section, int *exit_status_out)
{
  pid_t pid = fork();
  if(pid < 0) return -1;
  if(pid == 0) {
    char *args[] = {"./codegen", (char *)section, NULL};
    execv("./codegen", args);
    _exit(127);
  }
  int status;
  waitpid(pid, &status, 0);
  if(WIFSIGNALED(status)) return -(WTERMSIG(status));
  *exit_status_out = WEXITSTATUS(status);
  return 0;
}

int
main(void)
{
  int failures = 0;
  int exit_status = 0;

  /* 100-char name: exceeds section[50] limit — must be rejected, not crash */
  const char *long_name =
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  int result = run_codegen(long_name, &exit_status);
  if(result < 0) {
    fprintf(stderr, "FAIL: codegen crashed with signal %d on overlong name\n", -result);
    failures++;
  } else if(exit_status == 0) {
    fprintf(stderr, "FAIL: codegen should reject overlong name with non-zero exit\n");
    failures++;
  }

  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
