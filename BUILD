load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@avr_tools//tools/avr:hex.bzl", "hex")

config_setting(
    name = "avr",
    values = {
        "cpu": "avr",
    },
)

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

cc_binary(
    name = "arduino_cli_mcu",
    srcs = glob([
        "main.c",
        "usbdrv/**/*.c",
        "usbdrv/**/*.h",
        "usbdrv/**/*.inc",
        "usbdrv/**/*.S",
    ]),
    copts = select({
        ":avr": [
            "-mmcu=$(MCU)",
            "-Os",
            "-std=c11",
        ],
        "//conditions:default": [],
    }),
    includes = ["usbdrv"],
    linkopts = select({
        ":avr": [
            "-mmcu=$(MCU)",
            "-std=c11",
        ],
        "//conditions:default": [],
    }),
)

hex(
    name = "arduino_cli_mcu_hex",
    src = ":arduino_cli_mcu",
)

#   listing(
#       name = "arduino_cli_mcu_listing",
#       src = ":arduino_cli_mcu",
#   )
