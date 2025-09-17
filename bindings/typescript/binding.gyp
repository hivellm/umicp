{
  "targets": [
    {
      "target_name": "umicp_core",
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "sources": [
        "src/umicp_core.cc",
        "src/envelope_wrap.cc",
        "src/frame_wrap.cc",
        "src/matrix_wrap.cc",
        "../../../umicp/cpp/src/envelope.cpp",
        "../../../umicp/cpp/src/frame.cpp",
        "../../../umicp/cpp/src/matrix_ops.cpp",
        "../../../umicp/cpp/src/buffer.cpp",
        "../../../umicp/cpp/src/config.cpp",
        "../../../umicp/cpp/src/protocol.cpp",
        "../../../umicp/cpp/src/serialization.cpp",
        "../../../umicp/cpp/src/security.cpp",
        "../../../umicp/cpp/src/compression.cpp",
        "../../../umicp/cpp/src/transport.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../../../umicp/cpp/include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "conditions": [
        ["OS=='linux'", {
          "libraries": [
            "-ljson-c",
            "-lz",
            "-lssl",
            "-lcrypto",
            "-luuid"
          ]
        }],
        ["OS=='win'", {
          "libraries": [
            "json-c.lib",
            "zlib.lib",
            "libssl.lib",
            "libcrypto.lib",
            "rpcrt4.lib"
          ]
        }]
      ]
    }
  ]
}
