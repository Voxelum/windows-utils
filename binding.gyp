{
  "variables": {
    "WIN_VER": "v10",
    "USE_ADDITIONAL_WINMD": "true"
  },
  "includes": ["common.gypi"],
  "targets": [
    {
      "target_name": "windows-utils",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ ],
      "conditions": [
        ["OS=='win'", {
          "libraries": ["-lruntimeobject.lib"],
          "sources": [
            "addon.cc"
          ]
        }],
        ["WIN_VER==\"v10\"", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalUsingDirectories": [
                "%ProgramFiles(x86)%/Microsoft Visual Studio 14.0/VC/lib/store/references",
                "%ProgramFiles%/Microsoft Visual Studio 14.0/VC/lib/store/references",
                "%ProgramFiles%/Microsoft Visual Studio/2017/BuildTools/VC/Tools/MSVC/14.16.27023/lib/x86/store/references"
              ]
            }
          }
        }],
        ["USE_ADDITIONAL_WINMD==\"true\"", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalUsingDirectories": [
                "%ProgramFiles%/Windows Kits/10/UnionMetadata/10.0.17763.0",
                "%ProgramFiles%/Windows Kits/10/Include/10.0.17763.0/um",
                "%ProgramFiles%/Windows Kits/10/Include/10.0.17763.0/winrt",
                "%ProgramFiles(x86)%/Windows Kits/10/UnionMetadata/10.0.17763.0",
                "%ProgramFiles(x86)%/Windows Kits/10/UnionMetadata/10.0.17763.0/winrt",
                "%ProgramFiles(x86)%/Windows Kits/10/Include/10.0.17763.0/um"
              ]
            }
          }
        }]
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions": ["/ZW", "/std:c++17"],
          "DisableSpecificWarnings": [4609]
        },
        "VCLinkerTool": {
          "GenerateDebugInformation": "false"
        }
      }
    }
  ]
}