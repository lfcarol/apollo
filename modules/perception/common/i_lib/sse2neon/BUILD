load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sse2neon",
    hdrs = ["sse2neon.h"],
    copts = select(
        {
            "@platforms//cpu:x86_64": [],
            "@platforms//cpu:aarch64": [
                "-march=armv8-a+fp+simd+crc",
            ],
	    },
        no_match_error = "Please Build with an ARM or Linux x86_64 platform",
    ),
)


