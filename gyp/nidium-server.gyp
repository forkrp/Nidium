# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidium-server',
        'type': 'executable',
        'product_dir': '<(nidium_exec_path)',
        'dependencies': [
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*'
        ],
        'include_dirs': [
            '<(nidium_src_path)',
        ],
        'sources': [
            '<(nidium_src_path)/Server/app/main.cpp',
            '<(nidium_src_path)/Server/Server.cpp',
            '<(nidium_src_path)/Server/REPL.cpp',
            '<(nidium_src_path)/Server/Context.cpp',
        ],
        'conditions': [
            ['OS!="win"', {
                'include_dirs': [
                    '<(third_party_path)/linenoise/',
                ],
                'defines':[
                    'LINENOISE_INTERRUPTIBLE',
                ],
                'sources': [
                    '<(third_party_path)/setproctitle/setproctitle.c',
                    '<(third_party_path)/linenoise/linenoise.c',
                ]
            }],
            ['OS=="win"', {
                'ldflags': [
                    '/NODEFAULTLIB:libcmt.lib',
                    '/VERBOSE:LIB',
                ],
                'link_settings': {
                    'libraries': [
                        'Shell32.lib',
                        'Shlwapi.lib',
                        'Kernel32.lib',

                        ]
                },
                'include_dirs': [
                    '<(third_party_path)/ninja/src',
                ],
                'sources': [
                    '<(third_party_path)/setproctitle/setproctitle.c',
                    '<(third_party_path)/ninja/src/getopt.c',
                ],
            }],
            ['OS=="linux"', {
                'ldflags': [
                    '-rdynamic',
                ],
            }],
            ['OS=="mac"', {
                "xcode_settings": {
                    "OTHER_LDFLAGS": [
                        '-rdynamic',
                    ]
                },
            }],
            ['nofork==1', {
                'defines':['NIDIUM_NO_FORK']
            }],
        ],
    }]
}
