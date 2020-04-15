load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "libusb",
    srcs = glob([
        "libusb/**/*.cpp",
        "libusb/**/*.c",
        "libusb/**/*.h",
    ]),
    copts = [
        "-fdiagnostics-color",
        "-Wall",
    ],
    strip_include_prefix = "include",
    deps = ["@system_libs//:dl"],
)

cc_binary(
    name = "arduino_cli",
    srcs = glob([
        "arduino_cli.cpp",
        "usb_device.h",
    ]),
    copts = [
        "-fdiagnostics-color",
        "-ldl",
        "-Wall",
    ],
    deps = ["libusb"],
)