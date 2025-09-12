import os
import subprocess

cc_compiler = 'gcc'
cxx_compiler = 'g++'

commonflags = [
    '-g',
    '-Os',
    '-Wall',
    '-Wextra',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
    '-Wno-unused-value',
    '-Wpointer-arith',
    '-ffunction-sections',
    '-fdata-sections',
    '-I/usr/local/include',
]

def get_compiler_name(cc):
    try:
        out = subprocess.check_output(cc.split() + ['--version'], universal_newlines=True)
        if "clang" in out.lower():
            return "clang"
        elif "gcc" in out or "gnu" in out.lower():
            return "gcc"
    except Exception:
        return None
    return None

cc_name = get_compiler_name(cc_compiler)

# --- Apply compiler-specific flags ---
cflags = [
    '-std=gnu11',
    '-Werror',
] + commonflags

if cc_name == "gcc":
    cflags.insert(1, '-Wno-discarded-qualifiers')
elif cc_name == "clang":
    cflags.insert(1, '-Wno-ignored-qualifiers')
    cflags.insert(2, '-Wno-unknown-warning-option')

cxxflags = [
    '-std=c++17',
] + commonflags

defines = [
    'MS_PLATFORM_X86',
    '_GNU_SOURCE',
]

define_flags = ['-D{}'.format(define) for define in defines]

link_flags = [
    '-lrt',
    '-lm',
    '-pthread',
]

x86_env = Environment(
    ENV={'PATH': os.environ['PATH']},

    CC=cc_compiler,
    CXX=cxx_compiler,
    CCFLAGS=cflags + define_flags,
    CXXFLAGS=cxxflags + define_flags,
    CPPPATH=[],

    LINKFLAGS=link_flags,

    LIBS=[],
)

Return('x86_env')
