load("@rules_cc//cc:defs.bzl", "cc_library")
load("//tools/platform:build_defs.bzl", "if_aarch64")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sse2neon",
    hdrs = ["sse2neon.h"],
    copts = if_aarch64([
        "-march=armv8-a+fp+simd+crc",
    ]),
)


