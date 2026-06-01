#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

START_TEST(test_codegen_buffer_overflow_boundary)
{
    /* Invariant: codegen must not crash or corrupt memory when processing
       adversarial argv[1] inputs that exceed the fixed 'section' buffer capacity */
    
    const char *payloads[] = {
        "valid_input",                                    /* valid short input */
        "a",                                              /* boundary: minimal */
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",  /* boundary: ~48 bytes (typical overflow trigger) */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" /* exploit: 100+ bytes */
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        pid_t pid = fork();
        ck_assert_int_ne(pid, -1);
        
        if (pid == 0) {
            /* Child process: execute codegen with adversarial input */
            char *argv[] = {"./utils/codegen", (char *)payloads[i], NULL};
            execv("./utils/codegen", argv);
            exit(127); /* exec failed */
        } else {
            /* Parent: wait for child and verify it did not crash with signal */
            int status;
            waitpid(pid, &status, 0);
            
            /* Security property: process must not terminate via signal (SIGSEGV, SIGABRT, etc.)
               Normal exit or exit via error code is acceptable; signal death is not */
            ck_assert_msg(!WIFSIGNALED(status),
                "codegen crashed with signal %d on payload %d: %s",
                WTERMSIG(status), i, payloads[i]);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_codegen_buffer_overflow_boundary);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}