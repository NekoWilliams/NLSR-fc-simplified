# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
"""
Copyright (c) 2014-2024,  The University of Memphis,
                          Regents of the University of California,
                          Arizona Board of Regents.

This file is part of NLSR (Named-data Link State Routing).
See AUTHORS.md for complete list of NLSR authors and contributors.

NLSR is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
"""

import os
import subprocess
from waflib import Context, Logs, Utils

VERSION = '24.08'
APPNAME = 'nlsr'
GIT_TAG_PREFIX = 'NLSR-'

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags',
              'coverage', 'sanitizers', 'boost',
              'doxygen', 'sphinx'],
             tooldir=['.waf-tools'])

    optgrp = opt.add_option_group('NLSR Options')

    optgrp.add_option('--with-chronosync', dest='with_chronosync', action='store_true', default=False,
                      help='Build with ChronoSync support')
    optgrp.add_option('--without-chronosync', dest='with_chronosync', action='store_false', default=False,
                      help='Build without ChronoSync support')

    optgrp.add_option('--with-psync', dest='with_psync', action='store_true', default=True,
                      help='Build with PSync support')
    optgrp.add_option('--without-psync', dest='with_psync', action='store_false', default=True,
                      help='Build without PSync support')

    optgrp.add_option('--with-svs', dest='with_svs', action='store_true', default=False,
                      help='Build with State Vector Sync support')
    optgrp.add_option('--without-svs', dest='with_svs', action='store_false', default=False,
                      help='Build without State Vector Sync support')

    optgrp.add_option('--with-tests', action='store_true', default=False,
                      help='Build unit tests')

def configure(conf):
    conf.load('compiler_cxx gnu_dirs')
    conf.load('default-compiler-flags')
    conf.load('coverage')
    conf.load('boost')

    # NDN-CXXのバージョン確認
    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                  uselib_store='NDN_CXX', mandatory=True)
    
    # NDN-CXXのバージョン情報を取得
    ndn_cxx_version = conf.check_cfg(package='libndn-cxx', args=['--modversion'],
                                   uselib_store='NDN_CXX_VERSION', mandatory=True).strip()
    version_parts = ndn_cxx_version.split('.')
    conf.define('NDNCXX_VERSION_MAJOR', version_parts[0])
    conf.define('NDNCXX_VERSION_MINOR', version_parts[1])
    conf.define('NDNCXX_VERSION_PATCH', version_parts[2] if len(version_parts) > 2 else '0')

    # Boostの確認
    conf.check_boost(lib='system filesystem')

    # PSyncの確認
    conf.check_cfg(package='PSync', args=['--cflags', '--libs'],
                  uselib_store='PSYNC', mandatory=True)

    # システム設定
    conf.define('SYSCONFDIR', conf.env.SYSCONFDIR)
    conf.define('DEFAULT_CONFIG_FILE', f'{conf.env.SYSCONFDIR}/ndn/nlsr.conf')

    # ビルド設定
    if conf.options.with_tests:
        conf.define('HAVE_TESTS', 1)
    if conf.options.with_chronosync:
        conf.define('HAVE_CHRONOSYNC', 1)
    if conf.options.with_psync:
        conf.define('HAVE_PSYNC', 1)
    if conf.options.with_svs:
        conf.define('HAVE_SVS', 1)

    # 設定ファイルの生成
    conf.write_config_header('src/config.hpp', remove=False)

def build(bld):
    bld.program(
        target='bin/nlsr',
        source=bld.path.ant_glob('src/**/*.cpp'),
        use='NDN_CXX BOOST PSYNC',
        includes='src',
        install_path='${BINDIR}')

    bld.install_files('${SYSCONFDIR}/ndn', 'nlsr.conf.sample')
    bld.install_files('${SYSCONFDIR}/ndn', 'nlsr-auto-prefix.conf.sample')

    if bld.env.WITH_TESTS:
        bld.recurse('tests')

    if bld.env.WITH_OTHER_TESTS:
        bld.recurse('tests-integrated')

    if bld.env.WITH_DOCS:
        bld.recurse('docs')

    bld.install_files('${SYSCONFDIR}/systemd/system', 'systemd/nlsr.service')

def docs(bld):
    from waflib import Options
    Options.commands = ['doxygen', 'sphinx'] + Options.commands

def doxygen(bld):
    version(bld)

    if not bld.env.DOXYGEN:
        bld.fatal('Cannot build documentation ("doxygen" not found in PATH)')

    bld(features='subst',
        name='doxygen.conf',
        source=['docs/doxygen.conf.in',
                'docs/named_data_theme/named_data_footer-with-analytics.html.in'],
        target=['docs/doxygen.conf',
                'docs/named_data_theme/named_data_footer-with-analytics.html'],
        VERSION=VERSION,
        HAVE_DOT='YES' if bld.env.DOT else 'NO',
        HTML_FOOTER='../build/docs/named_data_theme/named_data_footer-with-analytics.html' \
                        if os.getenv('GOOGLE_ANALYTICS', None) \
                        else '../docs/named_data_theme/named_data_footer.html',
        GOOGLE_ANALYTICS=os.getenv('GOOGLE_ANALYTICS', ''))

    bld(features='doxygen',
        doxyfile='docs/doxygen.conf',
        use='doxygen.conf')

def sphinx(bld):
    version(bld)

    if not bld.env.SPHINX_BUILD:
        bld.fatal('Cannot build documentation ("sphinx-build" not found in PATH)')

    bld(features='sphinx',
        config='docs/conf.py',
        outdir='docs',
        source=bld.path.ant_glob('docs/**/*.rst'),
        version=VERSION_BASE,
        release=VERSION)

def version(ctx):
    # don't execute more than once
    if getattr(Context.g_module, 'VERSION_BASE', None):
        return

    Context.g_module.VERSION_BASE = Context.g_module.VERSION
    Context.g_module.VERSION_SPLIT = VERSION_BASE.split('.')

    # first, try to get a version string from git
    version_from_git = ''
    try:
        cmd = ['git', 'describe', '--abbrev=8', '--always', '--match', f'{GIT_TAG_PREFIX}*']
        version_from_git = subprocess.run(cmd, capture_output=True, check=True, text=True).stdout.strip()
        if version_from_git:
            if GIT_TAG_PREFIX and version_from_git.startswith(GIT_TAG_PREFIX):
                Context.g_module.VERSION = version_from_git[len(GIT_TAG_PREFIX):]
            elif not GIT_TAG_PREFIX and ('.' in version_from_git or '-' in version_from_git):
                Context.g_module.VERSION = version_from_git
            else:
                # no tags matched (or we are in a shallow clone)
                Context.g_module.VERSION = f'{VERSION_BASE}+git.{version_from_git}'
    except (OSError, subprocess.SubprocessError):
        pass

    # fallback to the VERSION.info file, if it exists and is not empty
    version_from_file = ''
    version_file = ctx.path.find_node('VERSION.info')
    if version_file is not None:
        try:
            version_from_file = version_file.read().strip()
        except OSError as e:
            Logs.warn(f'{e.filename} exists but is not readable ({e.strerror})')
    if version_from_file and not version_from_git:
        Context.g_module.VERSION = version_from_file
        return

    # update VERSION.info if necessary
    if version_from_file == Context.g_module.VERSION:
        # already up-to-date
        return
    if version_file is None:
        version_file = ctx.path.make_node('VERSION.info')
    try:
        version_file.write(Context.g_module.VERSION)
    except OSError as e:
        Logs.warn(f'{e.filename} is not writable ({e.strerror})')

def dist(ctx):
    ctx.algo = 'tar.xz'
    version(ctx)

def distcheck(ctx):
    ctx.algo = 'tar.xz'
    version(ctx)
