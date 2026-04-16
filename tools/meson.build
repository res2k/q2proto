project('q2proto', 'c', 'cpp',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 1.4.0',
  default_options: [
    'c_std=c11',
    'cpp_std=c++23,vc++latest',
    'buildtype=debugoptimized',
    'default_library=static',
  ],
)


q2proto_inc = include_directories('../inc')

cc = meson.get_compiler('c')

c_args = []
link_args = []
if cc.get_id() == 'msvc'
  # Enable conformant preprocessor
  c_args += '/Zc:preprocessor'
  # Declare we want UTF-8
  manifest_utf8 = files('windows-utf8.manifest')[0].full_path()
  link_args += ['/MANIFEST:EMBED', f'/MANIFESTINPUT:@manifest_utf8@']
endif

fmt = dependency('fmt')
zlib = dependency('zlib')

q2proto_src = [
    '../src/single_source_q2proto.c',
  '../src/dummy_q2protoerr_client_write.c',
  '../src/dummy_q2protoerr_server_read.c',
  '../src/dummy_q2protoerr_server_write.c',
  '../src/dummy_q2protoio_write.c',
  '../src/dummy_q2protoio_deflate.c',
]

q2proto = static_library('q2proto', q2proto_src,
  include_directories:   [q2proto_inc, '../src'],
  gnu_symbol_visibility: 'hidden',
  c_args:                c_args,
  cpp_args:              c_args,
  )

demodump_src = [
  'demodump.cpp',
  'q2protodbg.cpp',
  'q2protoerr.cpp',
  'q2protoio.cpp',
  ]
executable(f'demodump', demodump_src,
  include_directories:   [q2proto_inc],
  dependencies:          [fmt, zlib],
  link_with:             [q2proto],
  gnu_symbol_visibility: 'hidden',
  win_subsystem:         'console,6.0',
  c_args:                c_args,
  cpp_args:              c_args,
  link_args:             link_args,
)
