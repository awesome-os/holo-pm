#include "git_forge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define EXPECT_EQ_INT(actual, expected)                                             \
    do {                                                                            \
        int _a = (actual);                                                          \
        int _e = (expected);                                                        \
        if (_a != _e) {                                                             \
            fprintf(stderr, "FAIL %s:%d: expected %d, got %d\n",                    \
                    __FILE__, __LINE__, _e, _a);                                    \
            ++g_tests_failed;                                                       \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define EXPECT_EQ_STR(actual, expected)                                             \
    do {                                                                            \
        const char* _a = (actual);                                                  \
        const char* _e = (expected);                                                \
        if (strcmp(_a, _e) != 0) {                                                  \
            fprintf(stderr, "FAIL %s:%d: expected \"%s\", got \"%s\"\n",            \
                    __FILE__, __LINE__, _e, _a);                                    \
            ++g_tests_failed;                                                       \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define RUN_TEST(fn)                                                                \
    do {                                                                            \
        int _before = g_tests_failed;                                               \
        ++g_tests_run;                                                              \
        fn();                                                                       \
        if (g_tests_failed == _before) {                                            \
            printf("PASS %s\n", #fn);                                               \
        }                                                                           \
    } while (0)

static void test_abi_magic(void) {
    EXPECT_EQ_INT((int)(b8g_git_forge_abi_magic() == B8G_GIT_FORGE_ABI_MAGIC_V3), 1);
    EXPECT_EQ_INT((int)(B8G_GIT_FORGE_ABI_MAGIC_V3 == 0x33564742u), 1);
}

static void test_parse_minimal_uri(void) {
    b8g_git_forge_uri_t u;
    int rc = b8g_repository_parse_uri(
        "repository://github.com/awesome-os/holo-pm@v1.2.3",
        &u
    );
    EXPECT_EQ_INT(rc, B8G_GIT_FORGE_OK);
    EXPECT_EQ_STR(u.host, "github.com");
    EXPECT_EQ_STR(u.org, "awesome-os");
    EXPECT_EQ_STR(u.repo, "holo-pm");
    EXPECT_EQ_STR(u.ref, "v1.2.3");
    EXPECT_EQ_STR(u.subpath, "");
    EXPECT_EQ_STR(u.hook, "");
    EXPECT_EQ_STR(u.target, "");
}

static void test_parse_legacy_git_forge_alias(void) {
    b8g_git_forge_uri_t u;
    int rc = b8g_git_forge_parse_uri(
        "git-forge://github.com/awesome-os/holo-pm@v1.2.3#packages/cli?hook=build&target=linux-musl",
        &u
    );
    EXPECT_EQ_INT(rc, B8G_GIT_FORGE_OK);
    EXPECT_EQ_STR(u.host, "github.com");
    EXPECT_EQ_STR(u.org, "awesome-os");
    EXPECT_EQ_STR(u.repo, "holo-pm");
    EXPECT_EQ_STR(u.ref, "v1.2.3");
    EXPECT_EQ_STR(u.subpath, "packages/cli");
    EXPECT_EQ_STR(u.hook, "build");
    EXPECT_EQ_STR(u.target, "linux-musl");
}

static void test_parse_full_uri(void) {
    b8g_git_forge_uri_t u;
    int rc = b8g_repository_parse_uri(
        "repository://forge.awesome-os.com/awesome-os/b8g-runtime@refs/heads/main"
        "#packages/core/abi?hook=postinstall&target=x86_64-windows-gnu",
        &u
    );
    EXPECT_EQ_INT(rc, B8G_GIT_FORGE_OK);
    EXPECT_EQ_STR(u.host, "forge.awesome-os.com");
    EXPECT_EQ_STR(u.org, "awesome-os");
    EXPECT_EQ_STR(u.repo, "b8g-runtime");
    EXPECT_EQ_STR(u.ref, "refs/heads/main");
    EXPECT_EQ_STR(u.subpath, "packages/core/abi");
    EXPECT_EQ_STR(u.hook, "postinstall");
    EXPECT_EQ_STR(u.target, "x86_64-windows-gnu");
}

static void test_parse_query_without_subpath(void) {
    b8g_git_forge_uri_t u;
    int rc = b8g_repository_parse_uri(
        "repository://github.com/awesome-os/holo-pm@main?target=wasm32-wasi&hook=build",
        &u
    );
    EXPECT_EQ_INT(rc, B8G_GIT_FORGE_OK);
    EXPECT_EQ_STR(u.subpath, "");
    EXPECT_EQ_STR(u.hook, "build");
    EXPECT_EQ_STR(u.target, "wasm32-wasi");
}

static void test_reject_invalid_scheme_and_missing_ref(void) {
    b8g_git_forge_uri_t u;
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("https://github.com/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_SCHEME
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://github.com/awesome-os/holo-pm", &u),
        B8G_GIT_FORGE_ERR_MISSING_REF
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://github.com/awesome-os/holo-pm@", &u),
        B8G_GIT_FORGE_ERR_MISSING_REF
    );
}

static void test_reject_path_traversal(void) {
    b8g_git_forge_uri_t u;
    EXPECT_EQ_INT(
        b8g_repository_parse_uri(
            "repository://github.com/awesome-os/holo-pm@main#../etc/passwd", &u
        ),
        B8G_GIT_FORGE_ERR_TRAVERSAL
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri(
            "repository://github.com/awesome-os/holo-pm@main#a/../../secret", &u
        ),
        B8G_GIT_FORGE_ERR_TRAVERSAL
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri(
            "repository://github.com/../holo-pm@main", &u
        ),
        B8G_GIT_FORGE_ERR_TRAVERSAL
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri(
            "repository://github.com/awesome-os/holo-pm@main?hook=../evil", &u
        ),
        B8G_GIT_FORGE_ERR_TRAVERSAL
    );
}

static void test_reject_localhost_b8g016(void) {
    b8g_git_forge_uri_t u;
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://127.0.0.1/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://127.0.0.1:8080/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://localhost/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://LOCALHOST:3000/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
    EXPECT_EQ_INT(
        b8g_repository_parse_uri("repository://[::1]:8443/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
    EXPECT_EQ_INT(
        b8g_git_forge_parse_uri("git-forge://127.0.0.1/awesome-os/holo-pm@main", &u),
        B8G_GIT_FORGE_ERR_LOCALHOST
    );
}

static void test_v11_blob_path_formatting(void) {
    const char* sha512 =
        "ab0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
        "ef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd";
    char buf[256];

    EXPECT_EQ_INT(
        b8g_git_forge_v11_blob_path("/cas/store", sha512, 0, buf, sizeof(buf)),
        B8G_GIT_FORGE_OK
    );
    EXPECT_EQ_STR(
        buf,
        "/cas/store/v11/files/ab/"
        "ab0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
        "ef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
    );

    EXPECT_EQ_INT(
        b8g_git_forge_v11_blob_path("/cas/store/", sha512, 1, buf, sizeof(buf)),
        B8G_GIT_FORGE_OK
    );
    EXPECT_EQ_STR(
        buf,
        "/cas/store/v11/files/ab/"
        "ab0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
        "ef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd-exec"
    );

    /* Reject uppercase hex */
    const char* upper_sha512 =
        "AB0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd"
        "ef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcd";
    EXPECT_EQ_INT(
        b8g_git_forge_v11_blob_path("/cas/store", upper_sha512, 0, buf, sizeof(buf)),
        B8G_GIT_FORGE_ERR_INVALID_SHA512
    );

    /* Reject short digest */
    EXPECT_EQ_INT(
        b8g_git_forge_v11_blob_path("/cas/store", "deadbeef", 0, buf, sizeof(buf)),
        B8G_GIT_FORGE_ERR_INVALID_SHA512
    );

    /* Reject too-small output buffer */
    char tiny[32];
    EXPECT_EQ_INT(
        b8g_git_forge_v11_blob_path("/cas/store", sha512, 0, tiny, sizeof(tiny)),
        B8G_GIT_FORGE_ERR_OVERFLOW
    );
}

int main(void) {
    RUN_TEST(test_abi_magic);
    RUN_TEST(test_parse_minimal_uri);
    RUN_TEST(test_parse_legacy_git_forge_alias);
    RUN_TEST(test_parse_full_uri);
    RUN_TEST(test_parse_query_without_subpath);
    RUN_TEST(test_reject_invalid_scheme_and_missing_ref);
    RUN_TEST(test_reject_path_traversal);
    RUN_TEST(test_reject_localhost_b8g016);
    RUN_TEST(test_v11_blob_path_formatting);

    printf("SUMMARY: %d run, %d failed\n", g_tests_run, g_tests_failed);
    return (g_tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
