#!/usr/bin/python3

import os
import glob
import re
from os import path
from typing import Optional, List, Iterable, Tuple


def list_directories(root_path: str):
    return [x for x in os.listdir(root_path) if path.isdir(path.join(root_path, x))]


def process_name(s: str) -> str:
    s = re.sub('[^a-zA-Z0-9]+', '_', s)
    s = re.sub('([a-z])([A-Z])', '\\1_\\2', s)
    s = re.sub('([A-Z]+)([A-Z])', '\\1_\\2', s)
    return s.upper()


def process_guard(s: str) -> str:
    r = []
    for x in s.split('_'):
        if len(r) != 0 and r[-1] == x:
            continue

        r.append(x)

    return '_'.join(r)


def find_first(x, cond):
    for i in range(len(x)):
        if cond(x[i]):
            return i

    return None


def find_last(x, cond):
    for i in range(len(x) - 1, -1, -1):
        if cond(x[i]):
            return i

    return None


def process_file(file_path: str, guard: str):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    line_ifndef = find_first(lines, lambda x: x.startswith('#ifndef'))
    line_define = find_first(lines, lambda x: x.startswith('#define'))
    line_endif = find_last(lines, lambda x: x.startswith('#endif'))

    if line_ifndef is None or line_define is None or line_endif is None:
        raise RuntimeError(file_path + ': failed to identify header guards')

    if line_ifndef > 3 or line_define > 3 or line_endif < len(lines) - 4:
        raise RuntimeError(file_path + ': header guards are identified inside of file')

    if line_define < line_ifndef:
        raise RuntimeError(file_path + ': #ifndef after #define')

    lines_n = ['#ifndef %s\n' % guard, '#define %s\n' % guard] + lines[line_define+1:line_endif] + \
              ['#endif // %s\n' % guard]

    with open(file_path, 'w') as f:
        f.writelines(lines_n)


def find_includes(root_dir: str) -> Iterable[str]:
    extensions = ['h', 'hpp']
    return (f for ext in extensions for f in glob.iglob('**/*.' + ext, root_dir=root_dir, recursive=True))


def process_src_dir(root_dir: str, module: str, qualifier: str):
    for f in find_includes(root_dir):
        f_name = process_name(path.splitext(f)[0])
        f_guard = '%s_%s' % (module, f_name)
        f_guard = 'UB_%s_%s_H' % (qualifier, process_guard(f_guard))
        process_file(path.join(root_dir, f), f_guard)


def process_include_dir(root_dir: str, module: str):
    ub_prefix = 'ub' + path.sep

    for f in find_includes(root_dir):
        f_ub_file = f.startswith(ub_prefix)
        f_name = f[len(ub_prefix):] if f_ub_file else f
        f_name = process_name(path.splitext(f_name)[0])

        if f_ub_file:
            f_guard = '%s_%s' % (module, f_name)
        else:
            f_guard = 'EXTRA_%s' % f_name

        f_guard = 'UB_%s_H' % process_guard(f_guard)
        process_file(path.join(root_dir, f), f_guard)


def scan_dir(s_dir: str, module_prefix: List[str]):
    cmake_file = path.join(s_dir, 'CMakeLists.txt')
    if path.exists(cmake_file) and path.isfile(cmake_file):
        module_name = process_name('_'.join(module_prefix)).upper()
        process_include_dir(path.join(s_dir, 'include'), module_name)
        process_src_dir(path.join(s_dir, 'src'), module_name, 'SRC')
        process_src_dir(path.join(s_dir, 'test'), module_name, 'TEST')
        return

    for f in os.listdir(s_dir):
        f_dir = path.join(s_dir, f)

        if path.isdir(f_dir):
            scan_dir(f_dir, module_prefix + [f])


def run():
    ub_root = path.realpath(path.join(path.dirname(__file__), '..'))

    scan_dir(path.join(ub_root, 'src'), [])
    process_src_dir(path.join(ub_root, 'tools', 'test', 'device_lib'), 'DEVICE', 'TEST')


if __name__ == '__main__':
    run()
