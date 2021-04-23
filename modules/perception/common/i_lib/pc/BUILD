load("@rules_cc//cc:defs.bzl", "cc_library")
load("//tools:cpplint.bzl", "cpplint")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "i_ground",
    srcs = ["i_ground.cc"],
    hdrs = ["i_ground.h"],
    deps = [
        ":i_struct_s",
        ":i_util",
        "//modules/perception/common/i_lib/algorithm:i_sort",
        "//modules/perception/common/i_lib/core",
        "//modules/perception/common/i_lib/da:i_ransac",
        "//modules/perception/common/i_lib/geometry:i_plane",
    ],
)

cc_library(
    name = "i_struct_s",
    hdrs = ["i_struct_s.h"],
    deps = select(
        {
            "@platforms//cpu:x86_64": [],
	        "@platforms//cpu:aarch64": [
                "//modules/perception/common/i_lib/sse2neon:sse2neon",
	        ],
	    },
        no_match_error = "Please Build with an ARM or Linux x86_64 platform",
    ),
)

cc_library(
    name = "i_util",
    hdrs = ["i_util.h"],
)

cpplint()
