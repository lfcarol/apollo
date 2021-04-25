load("@rules_cc//cc:defs.bzl", "cc_library")
load("//tools/platform:build_defs.bzl", "if_aarch64", "if_x86_64")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "localization_msf",
    srcs = if_x86_64([
        "lib/x86_64/liblocalization_msf.so",
    ]) + if_aarch64([
        "lib/aarch64/liblocalization_msf.so",
    ]),
    hdrs = glob(["include/*.h"]),
    include_prefix = "localization_msf",
    strip_include_prefix = "include",
    linkopts = [
        "-lm",
    ],
)
